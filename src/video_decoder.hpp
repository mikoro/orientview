#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

#include <spdlog/spdlog.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

struct VideoFrame {
    uint8_t* data = nullptr;
    int width = 0;
    int height = 0;
    int linesize = 0;
    double timestamp = 0.0;
    double duration = 0.0;

    ~VideoFrame() {
        if (data) {
            av_free(data);
            data = nullptr;
        }
    }
};

struct AudioFrame {
    uint8_t* data = nullptr;
    int size = 0;

    ~AudioFrame() {
        if (data) {
            av_free(data);
            data = nullptr;
        }
    }
};

class VideoDecoder {
  private:
    std::string _filePath;
    AVFormatContext* _formatContext = nullptr;
    AVCodecContext* _videoCodecContext = nullptr;
    AVCodecContext* _audioCodecContext = nullptr;
    SwsContext* _swsContext = nullptr;
    SwrContext* _swrContext = nullptr;
    int _videoStreamIndex = -1;
    int _audioStreamIndex = -1;
    std::vector<std::string> _videoDecoders;
    std::vector<std::string> _audioDecoders;

    std::thread _decodingThread;
    std::atomic<bool> _running{false};
    std::atomic<bool> _seekRequested{false};
    double _seekPosition = 0.0;

    std::queue<std::shared_ptr<VideoFrame>> _videoFrameQueue;
    std::queue<std::shared_ptr<AudioFrame>> _audioFrameQueue;
    std::mutex _videoQueueMutex;
    std::mutex _audioQueueMutex;
    std::condition_variable _videoQueueCV;
    std::condition_variable _audioQueueCV;

    const int MAX_QUEUE_SIZE = 30;

    double _duration = 0.0;
    double _currentPosition = 0.0;

    void DecodingThread() {
        AVPacket* packet = av_packet_alloc();
        AVFrame* frame = av_frame_alloc();

        while (_running) {
            if (_seekRequested) {
                SeekToPosition(_seekPosition);
                _seekRequested = false;
            }

            {
                std::unique_lock<std::mutex> videoLock(_videoQueueMutex, std::defer_lock);
                std::unique_lock<std::mutex> audioLock(_audioQueueMutex, std::defer_lock);

                std::lock(videoLock, audioLock);

                if (_videoFrameQueue.size() >= MAX_QUEUE_SIZE || (_audioStreamIndex >= 0 && _audioFrameQueue.size() >= MAX_QUEUE_SIZE)) {
                    videoLock.unlock();
                    audioLock.unlock();
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    continue;
                }
            }

            int ret = av_read_frame(_formatContext, packet);
            if (ret < 0) {
                if (ret == AVERROR_EOF) {
                    SeekToPosition(0);
                    continue;
                } else {
                    break;
                }
            }

            if (!DecodePacket(packet)) {
                av_packet_unref(packet);
                break;
            }

            av_packet_unref(packet);
        }

        if (_videoCodecContext) {
            avcodec_send_packet(_videoCodecContext, nullptr);
            while (avcodec_receive_frame(_videoCodecContext, frame) == 0) {
                auto videoFrame = ProcessVideoFrame(frame);
                if (videoFrame) {
                    std::lock_guard<std::mutex> lock(_videoQueueMutex);
                    _videoFrameQueue.push(videoFrame);
                    _videoQueueCV.notify_one();
                }
                av_frame_unref(frame);
            }
        }

        if (_audioCodecContext) {
            avcodec_send_packet(_audioCodecContext, nullptr);
            while (avcodec_receive_frame(_audioCodecContext, frame) == 0) {
                auto audioFrame = ProcessAudioFrame(frame);
                if (audioFrame) {
                    std::lock_guard<std::mutex> lock(_audioQueueMutex);
                    _audioFrameQueue.push(audioFrame);
                    _audioQueueCV.notify_one();
                }
                av_frame_unref(frame);
            }
        }

        av_frame_free(&frame);
        av_packet_free(&packet);

        spdlog::debug("Decoding thread stopped");
    }

    bool DecodePacket(AVPacket* packet) {
        if (packet->stream_index == _videoStreamIndex) {
            int ret = avcodec_send_packet(_videoCodecContext, packet);
            if (ret < 0) {
                spdlog::error("Error sending video packet to decoder");
                return false;
            }

            while (ret >= 0) {
                AVFrame* frame = av_frame_alloc();
                ret = avcodec_receive_frame(_videoCodecContext, frame);

                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    av_frame_free(&frame);
                    break;
                } else if (ret < 0) {
                    spdlog::error("Error receiving video frame from decoder");
                    av_frame_free(&frame);
                    return false;
                }

                auto videoFrame = ProcessVideoFrame(frame);
                if (videoFrame) {
                    _currentPosition = videoFrame->timestamp;

                    {
                        std::lock_guard<std::mutex> lock(_videoQueueMutex);
                        _videoFrameQueue.push(videoFrame);
                        _videoQueueCV.notify_one();
                    }
                }

                av_frame_free(&frame);
            }
        } else if (packet->stream_index == _audioStreamIndex) {
            int ret = avcodec_send_packet(_audioCodecContext, packet);
            if (ret < 0) {
                spdlog::error("Error sending audio packet to decoder");
                return false;
            }

            while (ret >= 0) {
                AVFrame* frame = av_frame_alloc();
                ret = avcodec_receive_frame(_audioCodecContext, frame);

                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    av_frame_free(&frame);
                    break;
                } else if (ret < 0) {
                    spdlog::error("Error receiving audio frame from decoder");
                    av_frame_free(&frame);
                    return false;
                }

                auto audioFrame = ProcessAudioFrame(frame);
                if (audioFrame) {
                    {
                        std::lock_guard<std::mutex> lock(_audioQueueMutex);
                        _audioFrameQueue.push(audioFrame);
                        _audioQueueCV.notify_one();
                    }
                }

                av_frame_free(&frame);
            }
        }

        return true;
    }

    std::shared_ptr<VideoFrame> ProcessVideoFrame(AVFrame* frame) {
        if (!_swsContext) {
            _swsContext = sws_getContext(_videoCodecContext->width, _videoCodecContext->height, _videoCodecContext->pix_fmt, _videoCodecContext->width, _videoCodecContext->height, AV_PIX_FMT_RGB24, SWS_BILINEAR, nullptr, nullptr, nullptr);

            if (!_swsContext) {
                spdlog::error("Could not initialize swscale context");
                return nullptr;
            }
        }

        auto videoFrame = std::make_shared<VideoFrame>();
        videoFrame->width = _videoCodecContext->width;
        videoFrame->height = _videoCodecContext->height;
        videoFrame->linesize = _videoCodecContext->width * 3; // RGB24 = 3 bytes per pixel

        int bufferSize = av_image_get_buffer_size(AV_PIX_FMT_RGB24, videoFrame->width, videoFrame->height, 1);
        videoFrame->data = (uint8_t*)av_malloc(bufferSize);

        if (!videoFrame->data) {
            spdlog::error("Could not allocate video frame buffer");
            return nullptr;
        }

        uint8_t* dest[4] = {videoFrame->data, nullptr, nullptr, nullptr};
        int destLinesize[4] = {videoFrame->linesize, 0, 0, 0};

        sws_scale(_swsContext, frame->data, frame->linesize, 0, frame->height, dest, destLinesize);

        AVRational timeBase = _formatContext->streams[_videoStreamIndex]->time_base;

        if (frame->pts != AV_NOPTS_VALUE) {
            videoFrame->timestamp = (double)frame->pts * av_q2d(timeBase);
        } else {
            videoFrame->timestamp = 0;
        }

        // Calculate frame duration
        if (_videoCodecContext->framerate.num > 0 && _videoCodecContext->framerate.den > 0) {
            videoFrame->duration = (double)_videoCodecContext->framerate.den / _videoCodecContext->framerate.num;
        } else {
            // Default to 1/25 second if we can't determine frame duration
            videoFrame->duration = 0.04;
        }

        return videoFrame;
    }

    std::shared_ptr<AudioFrame> ProcessAudioFrame(AVFrame* frame) {
        if (!_audioCodecContext) {
            return nullptr;
        }

        if (!_swrContext) {
            AVChannelLayout outLayout;
            av_channel_layout_default(&outLayout, 2);

            _swrContext = swr_alloc();
            if (!_swrContext) {
                spdlog::error("Could not allocate swresample context");
                return nullptr;
            }

            int ret = swr_alloc_set_opts2(&_swrContext,
                                          &outLayout,                      // out_ch_layout
                                          AV_SAMPLE_FMT_FLT,               // out_sample_fmt
                                          48000,                           // out_sample_rate
                                          &_audioCodecContext->ch_layout,  // in_ch_layout
                                          _audioCodecContext->sample_fmt,  // in_sample_fmt
                                          _audioCodecContext->sample_rate, // in_sample_rate
                                          0, NULL);

            if (ret < 0) {
                spdlog::error("Could not set swresample options");
                swr_free(&_swrContext);
                return nullptr;
            }

            if (swr_init(_swrContext) < 0) {
                spdlog::error("Could not initialize swresample context");
                return nullptr;
            }
        }

        auto audioFrame = std::make_shared<AudioFrame>();

        int outSamples = swr_get_out_samples(_swrContext, frame->nb_samples);
        int outSize = av_samples_get_buffer_size(nullptr, 2, outSamples, AV_SAMPLE_FMT_FLT, 1);

        audioFrame->data = (uint8_t*)av_malloc(outSize);
        if (!audioFrame->data) {
            spdlog::error("Could not allocate audio frame buffer");
            return nullptr;
        }

        uint8_t* outData[1] = {audioFrame->data};
        int convertedSamples = swr_convert(_swrContext, outData, outSamples, (const uint8_t**)frame->data, frame->nb_samples);

        if (convertedSamples < 0) {
            spdlog::error("Error converting audio samples");
            av_free(audioFrame->data);
            return nullptr;
        }

        audioFrame->size = av_samples_get_buffer_size(nullptr, 2, convertedSamples, AV_SAMPLE_FMT_FLT, 1);

        return audioFrame;
    }

    void SeekToPosition(double position) {
        if (!_formatContext || _videoStreamIndex < 0) {
            return;
        }

        {
            std::lock_guard<std::mutex> lock(_videoQueueMutex);
            std::queue<std::shared_ptr<VideoFrame>> empty;
            std::swap(_videoFrameQueue, empty);
        }

        {
            std::lock_guard<std::mutex> lock(_audioQueueMutex);
            std::queue<std::shared_ptr<AudioFrame>> empty;
            std::swap(_audioFrameQueue, empty);
        }

        int64_t timestamp = position * AV_TIME_BASE;
        int streamIndex = _videoStreamIndex;
        AVRational timeBase = AV_TIME_BASE_Q;

        if (streamIndex >= 0) {
            timestamp = av_rescale_q(timestamp, timeBase, _formatContext->streams[streamIndex]->time_base);
        }

        int ret = av_seek_frame(_formatContext, streamIndex, timestamp, AVSEEK_FLAG_BACKWARD);
        if (ret < 0) {
            spdlog::error("Error seeking to position {}", position);
            return;
        }

        if (_videoCodecContext) {
            avcodec_flush_buffers(_videoCodecContext);
        }

        if (_audioCodecContext) {
            avcodec_flush_buffers(_audioCodecContext);
        }

        _currentPosition = position;
    }

  public:
    VideoDecoder() = default;
    ~VideoDecoder() { Close(); }

    bool IsRunning() const { return _running; }
    double GetDuration() const { return _duration; }
    double GetCurrentPosition() const { return _currentPosition; }
    int GetVideoWidth() const { return _videoCodecContext ? _videoCodecContext->width : 0; }
    int GetVideoHeight() const { return _videoCodecContext ? _videoCodecContext->height : 0; }

    const std::vector<std::string>& GetVideoDecoders() const { return _videoDecoders; }
    const std::vector<std::string>& GetAudioDecoders() const { return _audioDecoders; }

    bool Init(const std::string& filePath) {
        Close();

        _filePath = filePath;
        _running = true;

        _formatContext = avformat_alloc_context();
        if (avformat_open_input(&_formatContext, filePath.c_str(), nullptr, nullptr) != 0) {
            spdlog::error("Could not open input file: {}", filePath);
            Close();
            return false;
        }

        if (avformat_find_stream_info(_formatContext, nullptr) < 0) {
            spdlog::error("Could not find stream information");
            Close();
            return false;
        }

        for (unsigned int i = 0; i < _formatContext->nb_streams; i++) {
            if (_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && _videoStreamIndex < 0) {
                _videoStreamIndex = (int)i;
            } else if (_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && _audioStreamIndex < 0) {
                _audioStreamIndex = (int)i;
            }
        }

        if (_videoStreamIndex < 0) {
            spdlog::error("Could not find video stream");
            Close();
            return false;
        }

        const AVCodec* videoCodec = nullptr;

        if (!Session::Instance().videoDecoder.empty()) {
            videoCodec = avcodec_find_decoder_by_name(Session::Instance().videoDecoder.c_str());

            if (!videoCodec) {
                spdlog::warn("Requested video codec '{}' not found, falling back to default", Session::Instance().videoDecoder);
            }
        }

        if (!videoCodec) {
            videoCodec = avcodec_find_decoder(_formatContext->streams[_videoStreamIndex]->codecpar->codec_id);

            if (!videoCodec) {
                spdlog::error("Unsupported video codec");
                Close();
                return false;
            }
        }

        _videoDecoders.clear();
        void* iter = nullptr;
        const AVCodec* codec = nullptr;

        while ((codec = av_codec_iterate(&iter))) {
            if (!av_codec_is_decoder(codec)) continue;
            if (codec->id != _formatContext->streams[_videoStreamIndex]->codecpar->codec_id) continue;
            _videoDecoders.push_back(codec->name);
        }

        Session::Instance().videoDecoder = videoCodec->name;
        spdlog::info("Selected video decoder: {} ({})", videoCodec->name, videoCodec->long_name ? videoCodec->long_name : "unknown");

        _videoCodecContext = avcodec_alloc_context3(videoCodec);

        if (!_videoCodecContext) {
            spdlog::error("Could not allocate video codec context");
            Close();
            return false;
        }

        if (avcodec_parameters_to_context(_videoCodecContext, _formatContext->streams[_videoStreamIndex]->codecpar) < 0) {
            spdlog::error("Could not copy video codec parameters");
            Close();
            return false;
        }

        _videoCodecContext->pkt_timebase = _formatContext->streams[_videoStreamIndex]->time_base;

        if (avcodec_open2(_videoCodecContext, videoCodec, nullptr) < 0) {
            spdlog::error("Could not open video codec");
            Close();
            return false;
        }

        if (_audioStreamIndex >= 0) {
            const AVCodec* audioCodec = nullptr;

            if (!Session::Instance().audioDecoder.empty()) {
                audioCodec = avcodec_find_decoder_by_name(Session::Instance().audioDecoder.c_str());

                if (!audioCodec) {
                    spdlog::warn("Requested audio codec '{}' not found, falling back to default", Session::Instance().audioDecoder);
                }
            }

            if (!audioCodec) {
                audioCodec = avcodec_find_decoder(_formatContext->streams[_audioStreamIndex]->codecpar->codec_id);
            }

            _audioDecoders.clear();
            void* iter = nullptr;
            const AVCodec* codec = nullptr;

            while ((codec = av_codec_iterate(&iter))) {
                if (!av_codec_is_decoder(codec)) continue;
                if (codec->id != _formatContext->streams[_audioStreamIndex]->codecpar->codec_id) continue;
                _audioDecoders.push_back(codec->name);
            }

            if (!audioCodec) {
                spdlog::warn("Unsupported audio codec");
                _audioStreamIndex = -1;
            } else {
                Session::Instance().audioDecoder = audioCodec->name;
                spdlog::info("Selected audio decoder: {} ({})", audioCodec->name, audioCodec->long_name ? audioCodec->long_name : "unknown");
                _audioCodecContext = avcodec_alloc_context3(audioCodec);

                if (!_audioCodecContext) {
                    spdlog::warn("Could not allocate audio codec context");
                    _audioStreamIndex = -1;
                } else {
                    if (avcodec_parameters_to_context(_audioCodecContext, _formatContext->streams[_audioStreamIndex]->codecpar) < 0) {
                        spdlog::warn("Could not copy audio codec parameters");
                        avcodec_free_context(&_audioCodecContext);
                        _audioStreamIndex = -1;
                    } else {
                        _audioCodecContext->pkt_timebase = _formatContext->streams[_audioStreamIndex]->time_base;

                        if (avcodec_open2(_audioCodecContext, audioCodec, nullptr) < 0) {
                            spdlog::warn("Could not open audio codec");
                            avcodec_free_context(&_audioCodecContext);
                            _audioStreamIndex = -1;
                        }
                    }
                }
            }
        }

        if (_formatContext->duration != AV_NOPTS_VALUE) {
            _duration = (double)_formatContext->duration / (double)AV_TIME_BASE;
        } else {
            _duration = 0.0;
        }

        spdlog::info("Opened video file: {}, Duration: {:.2f} seconds, Resolution: {}x{}{}", filePath, _duration, _videoCodecContext->width, _videoCodecContext->height,
                     _audioStreamIndex >= 0 ? fmt::format(", Audio: {} channels, {} Hz", _audioCodecContext->ch_layout.nb_channels, _audioCodecContext->sample_rate) : "");

        _decodingThread = std::thread(&VideoDecoder::DecodingThread, this);
        return true;
    }

    void Close() {
        if (_running) {
            _running = false;

            _videoQueueCV.notify_all();
            _audioQueueCV.notify_all();

            if (_decodingThread.joinable()) {
                _decodingThread.join();
            }
        }

        if (_swsContext) {
            sws_freeContext(_swsContext);
            _swsContext = nullptr;
        }

        if (_swrContext) {
            swr_free(&_swrContext);
            _swrContext = nullptr;
        }

        if (_videoCodecContext) {
            avcodec_free_context(&_videoCodecContext);
            _videoCodecContext = nullptr;
        }

        if (_audioCodecContext) {
            avcodec_free_context(&_audioCodecContext);
            _audioCodecContext = nullptr;
        }

        if (_formatContext) {
            avformat_close_input(&_formatContext);
            _formatContext = nullptr;
        }

        _videoStreamIndex = -1;
        _audioStreamIndex = -1;
        _duration = 0.0;
        _currentPosition = 0.0;

        {
            std::lock_guard<std::mutex> lock(_videoQueueMutex);
            std::queue<std::shared_ptr<VideoFrame>> empty;
            std::swap(_videoFrameQueue, empty);
        }

        {
            std::lock_guard<std::mutex> lock(_audioQueueMutex);
            std::queue<std::shared_ptr<AudioFrame>> empty;
            std::swap(_audioFrameQueue, empty);
        }
    }

    void Seek(double position) {
        if (!_running) {
            return;
        }

        _seekPosition = position;
        _seekRequested = true;
    }

    std::shared_ptr<VideoFrame> GetNextVideoFrame() {
        std::unique_lock<std::mutex> lock(_videoQueueMutex);

        if (_videoFrameQueue.empty()) {
            return nullptr;
        }

        auto frame = _videoFrameQueue.front();
        _videoFrameQueue.pop();
        return frame;
    }

    std::shared_ptr<AudioFrame> GetNextAudioFrame() {
        std::unique_lock<std::mutex> lock(_audioQueueMutex);

        if (_audioFrameQueue.empty()) {
            return nullptr;
        }

        auto frame = _audioFrameQueue.front();
        _audioFrameQueue.pop();
        return frame;
    }

    static void ListAllCodecs() {
        const AVCodec* codec = nullptr;
        void* iter = nullptr;

        spdlog::info("Hardware-accelerated video decoders:");
        bool foundHwDecoders = false;
        while ((codec = av_codec_iterate(&iter))) {
            if (!av_codec_is_decoder(codec)) continue;

            // Check if this is a hardware decoder
            if (codec->capabilities & AV_CODEC_CAP_HARDWARE) {
                foundHwDecoders = true;
                spdlog::info("  HW Video Decoder: {} ({})", codec->name, codec->long_name ? codec->long_name : "");
            }
        }

        if (!foundHwDecoders) {
            spdlog::info("  No hardware video decoders found");
        }

        spdlog::info("Software video decoders:");
        iter = nullptr;
        bool foundSwDecoders = false;
        while ((codec = av_codec_iterate(&iter))) {
            if (!av_codec_is_decoder(codec)) continue;
            if (codec->type != AVMEDIA_TYPE_VIDEO) continue;
            if (codec->capabilities & AV_CODEC_CAP_HARDWARE) continue;

            foundSwDecoders = true;
            spdlog::info("  SW Video Decoder: {} ({})", codec->name, codec->long_name ? codec->long_name : "");
        }

        if (!foundSwDecoders) {
            spdlog::info("  No software video decoders found");
        }

        spdlog::info("Hardware-accelerated video encoders:");
        iter = nullptr;
        bool foundHwEncoders = false;
        while ((codec = av_codec_iterate(&iter))) {
            if (!av_codec_is_encoder(codec)) continue;
            if (codec->type != AVMEDIA_TYPE_VIDEO) continue;

            // Check if this is a hardware encoder
            if (codec->capabilities & AV_CODEC_CAP_HARDWARE) {
                foundHwEncoders = true;
                spdlog::info("  HW Video Encoder: {} ({})", codec->name, codec->long_name ? codec->long_name : "");
            }
        }

        if (!foundHwEncoders) {
            spdlog::info("  No hardware video encoders found");
        }

        spdlog::info("Software video encoders:");
        iter = nullptr;
        bool foundSwEncoders = false;
        while ((codec = av_codec_iterate(&iter))) {
            if (!av_codec_is_encoder(codec)) continue;
            if (codec->type != AVMEDIA_TYPE_VIDEO) continue;
            if (codec->capabilities & AV_CODEC_CAP_HARDWARE) continue;

            foundSwEncoders = true;
            spdlog::info("  SW Video Encoder: {} ({})", codec->name, codec->long_name ? codec->long_name : "");
        }

        if (!foundSwEncoders) {
            spdlog::info("  No software video encoders found");
        }
    }
};
