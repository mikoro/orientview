#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
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
    double pts = 0.0;

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
    double pts = 0.0;

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

    std::thread _decodingThread;
    std::atomic<bool> _running{false};
    std::atomic<bool> _paused{true};
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

    std::function<void(std::shared_ptr<VideoFrame>)> _videoFrameCallback;
    std::function<void(std::shared_ptr<AudioFrame>)> _audioFrameCallback;

    void DecodingThread() {
        AVPacket* packet = av_packet_alloc();
        AVFrame* frame = av_frame_alloc();

        while (_running) {
            if (_seekRequested) {
                SeekToPosition(_seekPosition);
                _seekRequested = false;
            }

            if (_paused) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            // Check if queues are full
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

            // Read packet
            int ret = av_read_frame(_formatContext, packet);
            if (ret < 0) {
                if (ret == AVERROR_EOF) {
                    // End of file, seek back to beginning
                    SeekToPosition(0);
                    continue;
                } else {
                    // Error reading frame
                    break;
                }
            }

            // Process packet
            if (!DecodePacket(packet)) {
                av_packet_unref(packet);
                break;
            }

            av_packet_unref(packet);
        }

        // Flush decoders
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
            // Video packet
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
                    // Update current position
                    _currentPosition = videoFrame->pts;

                    // Add to queue
                    {
                        std::lock_guard<std::mutex> lock(_videoQueueMutex);
                        _videoFrameQueue.push(videoFrame);
                        _videoQueueCV.notify_one();
                    }

                    // Call callback if set
                    if (_videoFrameCallback) {
                        _videoFrameCallback(videoFrame);
                    }
                }

                av_frame_free(&frame);
            }
        } else if (packet->stream_index == _audioStreamIndex) {
            // Audio packet
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
                    // Add to queue
                    {
                        std::lock_guard<std::mutex> lock(_audioQueueMutex);
                        _audioFrameQueue.push(audioFrame);
                        _audioQueueCV.notify_one();
                    }

                    // Call callback if set
                    if (_audioFrameCallback) {
                        _audioFrameCallback(audioFrame);
                    }
                }

                av_frame_free(&frame);
            }
        }

        return true;
    }

    std::shared_ptr<VideoFrame> ProcessVideoFrame(AVFrame* frame) {
        // Initialize swscale context if needed
        if (!_swsContext) {
            _swsContext = sws_getContext(_videoCodecContext->width, _videoCodecContext->height, _videoCodecContext->pix_fmt, _videoCodecContext->width, _videoCodecContext->height, AV_PIX_FMT_RGB24, SWS_BILINEAR, nullptr, nullptr, nullptr);

            if (!_swsContext) {
                spdlog::error("Could not initialize swscale context");
                return nullptr;
            }
        }

        // Create output frame
        auto videoFrame = std::make_shared<VideoFrame>();
        videoFrame->width = _videoCodecContext->width;
        videoFrame->height = _videoCodecContext->height;
        videoFrame->linesize = _videoCodecContext->width * 3; // RGB24 = 3 bytes per pixel

        // Allocate buffer for RGB data
        int bufferSize = av_image_get_buffer_size(AV_PIX_FMT_RGB24, videoFrame->width, videoFrame->height, 1);
        videoFrame->data = (uint8_t*)av_malloc(bufferSize);

        if (!videoFrame->data) {
            spdlog::error("Could not allocate video frame buffer");
            return nullptr;
        }

        // Convert frame to RGB
        uint8_t* dest[4] = {videoFrame->data, nullptr, nullptr, nullptr};
        int destLinesize[4] = {videoFrame->linesize, 0, 0, 0};

        sws_scale(_swsContext, frame->data, frame->linesize, 0, frame->height, dest, destLinesize);

        // Set timestamp
        if (frame->pts != AV_NOPTS_VALUE) {
            AVRational timeBase = _formatContext->streams[_videoStreamIndex]->time_base;
            videoFrame->pts = frame->pts * av_q2d(timeBase);
        } else {
            videoFrame->pts = 0;
        }

        return videoFrame;
    }

    std::shared_ptr<AudioFrame> ProcessAudioFrame(AVFrame* frame) {
        if (!_audioCodecContext) {
            return nullptr;
        }

        // Initialize swresample context if needed
        if (!_swrContext) {
            // Create stereo output channel layout
            AVChannelLayout outLayout;
            av_channel_layout_default(&outLayout, 2);

            // Allocate the resampler context first
            _swrContext = swr_alloc();
            if (!_swrContext) {
                spdlog::error("Could not allocate swresample context");
                return nullptr;
            }

            // Initialize the resampler
            int ret = swr_alloc_set_opts2(&_swrContext,
                                          &outLayout,                      // out_ch_layout
                                          AV_SAMPLE_FMT_S16,               // out_sample_fmt
                                          _audioCodecContext->sample_rate, // out_sample_rate
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

        // Create output frame
        auto audioFrame = std::make_shared<AudioFrame>();

        // Calculate output size and allocate buffer
        int outSamples = swr_get_out_samples(_swrContext, frame->nb_samples);
        int outSize = av_samples_get_buffer_size(nullptr, 2, outSamples, AV_SAMPLE_FMT_S16, 1);

        audioFrame->data = (uint8_t*)av_malloc(outSize);
        if (!audioFrame->data) {
            spdlog::error("Could not allocate audio frame buffer");
            return nullptr;
        }

        // Convert audio
        uint8_t* outData[1] = {audioFrame->data};
        int convertedSamples = swr_convert(_swrContext, outData, outSamples, (const uint8_t**)frame->data, frame->nb_samples);

        if (convertedSamples < 0) {
            spdlog::error("Error converting audio samples");
            av_free(audioFrame->data);
            return nullptr;
        }

        // Set size and timestamp
        audioFrame->size = av_samples_get_buffer_size(nullptr, 2, convertedSamples, AV_SAMPLE_FMT_S16, 1);

        if (frame->pts != AV_NOPTS_VALUE) {
            AVRational timeBase = _formatContext->streams[_audioStreamIndex]->time_base;
            audioFrame->pts = frame->pts * av_q2d(timeBase);
        } else {
            audioFrame->pts = 0;
        }

        return audioFrame;
    }

    void SeekToPosition(double position) {
        if (!_formatContext || _videoStreamIndex < 0) {
            return;
        }

        // Clear queues
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

        // Convert position to stream timebase
        int64_t timestamp = position * AV_TIME_BASE;
        int streamIndex = _videoStreamIndex;
        AVRational timeBase = AV_TIME_BASE_Q;

        if (streamIndex >= 0) {
            timestamp = av_rescale_q(timestamp, timeBase, _formatContext->streams[streamIndex]->time_base);
        }

        // Seek
        int ret = av_seek_frame(_formatContext, streamIndex, timestamp, AVSEEK_FLAG_BACKWARD);
        if (ret < 0) {
            spdlog::error("Error seeking to position {}", position);
            return;
        }

        // Flush codec buffers
        if (_videoCodecContext) {
            avcodec_flush_buffers(_videoCodecContext);
        }

        if (_audioCodecContext) {
            avcodec_flush_buffers(_audioCodecContext);
        }

        _currentPosition = position;
        spdlog::debug("Seeked to position {}", position);
    }

  public:
    VideoDecoder() = default;

    ~VideoDecoder() { Close(); }

    bool Open(const std::string& filePath) {
        Close();

        _filePath = filePath;

        // Open input file
        _formatContext = avformat_alloc_context();
        if (avformat_open_input(&_formatContext, filePath.c_str(), nullptr, nullptr) != 0) {
            spdlog::error("Could not open input file: {}", filePath);
            Close();
            return false;
        }

        // Find stream info
        if (avformat_find_stream_info(_formatContext, nullptr) < 0) {
            spdlog::error("Could not find stream information");
            Close();
            return false;
        }

        // Find video and audio streams
        for (unsigned int i = 0; i < _formatContext->nb_streams; i++) {
            if (_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && _videoStreamIndex < 0) {
                _videoStreamIndex = i;
            } else if (_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && _audioStreamIndex < 0) {
                _audioStreamIndex = i;
            }
        }

        if (_videoStreamIndex < 0) {
            spdlog::error("Could not find video stream");
            Close();
            return false;
        }

        // Set up video codec
        const AVCodec* videoCodec = avcodec_find_decoder(_formatContext->streams[_videoStreamIndex]->codecpar->codec_id);
        if (!videoCodec) {
            spdlog::error("Unsupported video codec");
            Close();
            return false;
        }

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

        if (avcodec_open2(_videoCodecContext, videoCodec, nullptr) < 0) {
            spdlog::error("Could not open video codec");
            Close();
            return false;
        }

        // Set up audio codec if available
        if (_audioStreamIndex >= 0) {
            const AVCodec* audioCodec = avcodec_find_decoder(_formatContext->streams[_audioStreamIndex]->codecpar->codec_id);
            if (!audioCodec) {
                spdlog::warn("Unsupported audio codec");
                _audioStreamIndex = -1;
            } else {
                _audioCodecContext = avcodec_alloc_context3(audioCodec);
                if (!_audioCodecContext) {
                    spdlog::warn("Could not allocate audio codec context");
                    _audioStreamIndex = -1;
                } else {
                    if (avcodec_parameters_to_context(_audioCodecContext, _formatContext->streams[_audioStreamIndex]->codecpar) < 0) {
                        spdlog::warn("Could not copy audio codec parameters");
                        avcodec_free_context(&_audioCodecContext);
                        _audioStreamIndex = -1;
                    } else if (avcodec_open2(_audioCodecContext, audioCodec, nullptr) < 0) {
                        spdlog::warn("Could not open audio codec");
                        avcodec_free_context(&_audioCodecContext);
                        _audioStreamIndex = -1;
                    }
                }
            }
        }

        // Calculate duration
        if (_formatContext->duration != AV_NOPTS_VALUE) {
            _duration = _formatContext->duration / (double)AV_TIME_BASE;
        } else {
            _duration = 0.0;
        }

        spdlog::info("Opened video file: {}", filePath);
        spdlog::info("Duration: {:.2f} seconds", _duration);
        spdlog::info("Video dimensions: {}x{}", _videoCodecContext->width, _videoCodecContext->height);

        if (_audioStreamIndex >= 0) {
            spdlog::info("Audio: {} channels, {} Hz", _audioCodecContext->ch_layout.nb_channels, _audioCodecContext->sample_rate);
        }

        return true;
    }

    void Close() {
        Stop();

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

    void Start() {
        if (_running) {
            return;
        }

        if (!_formatContext || _videoStreamIndex < 0) {
            spdlog::error("Cannot start decoding: no video opened");
            return;
        }

        _running = true;
        _paused = false;
        _decodingThread = std::thread(&VideoDecoder::DecodingThread, this);
    }

    void Pause() { _paused = true; }

    void Resume() { _paused = false; }

    void Stop() {
        if (!_running) {
            return;
        }

        _running = false;
        _paused = true;

        _videoQueueCV.notify_all();
        _audioQueueCV.notify_all();

        if (_decodingThread.joinable()) {
            _decodingThread.join();
        }
    }

    void Seek(double position) {
        if (!_running) {
            return;
        }

        _seekPosition = position;
        _seekRequested = true;
    }

    double GetDuration() const { return _duration; }
    double GetCurrentPosition() const { return _currentPosition; }

    bool IsRunning() const { return _running; }
    bool IsPaused() const { return _paused; }

    void SetVideoFrameCallback(std::function<void(std::shared_ptr<VideoFrame>)> callback) { _videoFrameCallback = callback; }

    void SetAudioFrameCallback(std::function<void(std::shared_ptr<AudioFrame>)> callback) { _audioFrameCallback = callback; }

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

    int GetVideoWidth() const { return _videoCodecContext ? _videoCodecContext->width : 0; }
    int GetVideoHeight() const { return _videoCodecContext ? _videoCodecContext->height : 0; }
    int GetAudioChannels() const { return _audioCodecContext ? _audioCodecContext->ch_layout.nb_channels : 0; }
    int GetAudioSampleRate() const { return _audioCodecContext ? _audioCodecContext->sample_rate : 0; }

    static void LogAvailableCodecs() {
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
