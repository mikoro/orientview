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

class BufferPool {
  private:
    std::vector<uint8_t*> _availableBuffers;
    std::mutex _mutex;
    size_t _bufferSize = 0;

  public:
    BufferPool() = default;
    ~BufferPool() { Clear(); }

    void Clear() {
        std::lock_guard<std::mutex> lock(_mutex);
        for (auto buffer : _availableBuffers) {
            if (buffer) {
                av_free(buffer);
            }
        }
        _availableBuffers.clear();
        _bufferSize = 0;
    }

    void SetBufferSize(size_t size) {
        if (_bufferSize != size) {
            Clear();
            _bufferSize = size;
        }
    }

    uint8_t* GetBuffer() {
        std::lock_guard<std::mutex> lock(_mutex);
        if (!_availableBuffers.empty()) {
            uint8_t* buffer = _availableBuffers.back();
            _availableBuffers.pop_back();
            return buffer;
        }

        return static_cast<uint8_t*>(av_malloc(_bufferSize));
    }

    void ReturnBuffer(uint8_t* buffer) {
        if (!buffer) return;

        std::lock_guard<std::mutex> lock(_mutex);
        _availableBuffers.push_back(buffer);
    }
};

struct AudioFrame {
    uint8_t* data = nullptr;
    int size = 0;
    double duration = 0.0;
    BufferPool* bufferPool = nullptr;

    ~AudioFrame() {
        if (data) {
            if (bufferPool) {
                bufferPool->ReturnBuffer(data);
            } else {
                av_free(data);
            }
            data = nullptr;
        }
    }
};

struct VideoFrame {
    uint8_t* data = nullptr;
    int width = 0;
    int height = 0;
    int linesize = 0;
    double timestamp = 0.0;
    double duration = 0.0;
    BufferPool* bufferPool = nullptr;
    std::vector<std::shared_ptr<AudioFrame>> audioFrames; // For Immediate mode

    ~VideoFrame() {
        if (data) {
            if (bufferPool) {
                bufferPool->ReturnBuffer(data);
            } else {
                av_free(data);
            }
            data = nullptr;
        }
        audioFrames.clear();
    }
};

enum class PlaybackMode {
    RealTime,
    Immediate
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
    std::vector<std::string> _hardwareVideoDecoders;
    std::vector<std::string> _hardwareAudioDecoders;

    std::function<void(std::shared_ptr<AudioFrame>)> _audioFrameCallback;

    std::thread _decodingThread;
    std::atomic<bool> _running{false};
    std::atomic<bool> _isPlaying{false};
    std::atomic<bool> _seekRequested{false};
    double _seekPosition = 0.0;
    PlaybackMode _playbackMode = PlaybackMode::RealTime;
    std::atomic<bool> _initialAudioBuffered{false};
    std::atomic<bool> _initialAudioBurstSent{false};

    std::queue<std::shared_ptr<VideoFrame>> _videoFrameQueue;
    std::queue<std::shared_ptr<AudioFrame>> _audioFrameQueue;
    std::mutex _queuesMutex;
    std::condition_variable _videoFrameConsumedCV;
    std::condition_variable _videoFrameAvailableCV;
    BufferPool _videoBufferPool;
    BufferPool _audioBufferPool;
    const size_t _maxVideoFrameQueueSize = 10;
    const double _audioBurstDuration = 0.2;
    std::shared_ptr<VideoFrame> _initialVideoFrame = nullptr;
    std::atomic<bool> _initialVideoFrameCaptured{false};
    std::chrono::high_resolution_clock::time_point _startTimePoint;
    double _nextVideoFramePresentationTime = 0.0;
    double _nextAudioFramePresentationTime = 0.0;

    double _duration = 0.0;
    double _currentPosition = 0.0;
    double _frameRate = 0.0;
    double _frameDuration = 0.0;
    int _colorDepth = 0;
    std::string _pixelFormat;

    void DecodingThread() {
        spdlog::info("Decoding thread started");
        AVPacket* packet = av_packet_alloc();

        while (_running) {
            if (_seekRequested) {
                SeekToPosition(_seekPosition);
                _seekRequested = false;
            }

            if (_playbackMode == PlaybackMode::RealTime && _audioStreamIndex >= 0 && _audioFrameCallback) {
                if (!_initialAudioBuffered) {
                    double totalAudioDuration = 0.0;

                    while (totalAudioDuration < _audioBurstDuration) {
                        int ret = av_read_frame(_formatContext, packet);
                        if (ret < 0) break;

                        if (!DecodePacket(packet)) {
                            av_packet_unref(packet);
                            break;
                        }

                        {
                            std::lock_guard<std::mutex> lock(_queuesMutex);
                            totalAudioDuration = 0.0;
                            std::queue<std::shared_ptr<AudioFrame>> tempQueue = _audioFrameQueue;
                            while (!tempQueue.empty()) {
                                totalAudioDuration += tempQueue.front()->duration;
                                tempQueue.pop();
                            }
                        }

                        av_packet_unref(packet);
                    }

                    _initialAudioBuffered = true;
                    spdlog::info("Burst audio gathered");
                }
            }

            if (!_isPlaying) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            if (_playbackMode == PlaybackMode::RealTime && _audioStreamIndex >= 0 && _audioFrameCallback) {
                if (!_initialAudioBurstSent) {
                    std::queue<std::shared_ptr<AudioFrame>> audioFramesToBurst;

                    {
                        std::lock_guard<std::mutex> lock(_queuesMutex);
                        audioFramesToBurst = _audioFrameQueue;
                        std::queue<std::shared_ptr<AudioFrame>> emptyAudioQueue;
                        std::swap(_audioFrameQueue, emptyAudioQueue);
                    }

                    while (!audioFramesToBurst.empty()) {
                        auto audioFrame = audioFramesToBurst.front();
                        audioFramesToBurst.pop();
                        _audioFrameCallback(audioFrame);
                    }

                    spdlog::info("Burst audio sent");
                    _initialAudioBurstSent = true;
                    _startTimePoint = std::chrono::high_resolution_clock::now();
                    _nextVideoFramePresentationTime = 0.0;
                    _nextAudioFramePresentationTime = 0.0;
                } else {
                    std::shared_ptr<AudioFrame> audioFrame = nullptr;
                    bool shouldSendAudio = false;

                    {
                        std::lock_guard<std::mutex> lock(_queuesMutex);
                        if (!_audioFrameQueue.empty()) {

                            audioFrame = _audioFrameQueue.front();
                            auto now = std::chrono::high_resolution_clock::now();
                            double elapsedTime = std::chrono::duration<double>(now - _startTimePoint).count();

                            if (elapsedTime >= _nextAudioFramePresentationTime) {
                                _audioFrameQueue.pop();
                                shouldSendAudio = true;
                            }
                        }
                    }

                    if (audioFrame && shouldSendAudio) {
                        _audioFrameCallback(audioFrame);
                        _nextAudioFramePresentationTime += audioFrame->duration;
                    }
                }
            }

            bool videoQueueFull = false;

            {
                std::unique_lock<std::mutex> lock(_queuesMutex);
                videoQueueFull = _videoFrameQueue.size() >= _maxVideoFrameQueueSize;
            }

            if (videoQueueFull) {
                std::unique_lock<std::mutex> lock(_queuesMutex);
                //_videoFrameConsumedCV.wait_for(lock, std::chrono::milliseconds(10));
                continue;
            }

            int ret = av_read_frame(_formatContext, packet);

            if (ret < 0) {
                if (ret == AVERROR_EOF) {
                    _isPlaying = false;
                    continue;
                }

                break;
            }

            if (!DecodePacket(packet)) {
                av_packet_unref(packet);
                break;
            }

            if (_playbackMode == PlaybackMode::Immediate) {
                _videoFrameAvailableCV.notify_one();
            }

            av_packet_unref(packet);
        }

        av_packet_free(&packet);
        spdlog::info("Decoding thread stopped");
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
                }

                if (ret < 0) {
                    spdlog::error("Error receiving video frame from decoder");
                    av_frame_free(&frame);
                    return false;
                }

                auto videoFrame = ProcessVideoFrame(frame);
                if (videoFrame) {
                    _currentPosition = videoFrame->timestamp;

                    {
                        std::lock_guard<std::mutex> lock(_queuesMutex);
                        _videoFrameQueue.push(videoFrame);

                        if (!_initialVideoFrameCaptured) {
                            _initialVideoFrame = std::make_shared<VideoFrame>(*videoFrame);
                            _initialVideoFrame->data = _videoBufferPool.GetBuffer();
                            memcpy(_initialVideoFrame->data, videoFrame->data, videoFrame->linesize * videoFrame->height);
                            _initialVideoFrame->bufferPool = &_videoBufferPool;
                            _initialVideoFrameCaptured = true;
                            spdlog::info("Initial video frame created");
                        }
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
                }

                if (ret < 0) {
                    spdlog::error("Error receiving audio frame from decoder");
                    av_frame_free(&frame);
                    return false;
                }

                auto audioFrame = ProcessAudioFrame(frame);
                if (audioFrame) {
                    {
                        std::lock_guard<std::mutex> lock(_queuesMutex);
                        _audioFrameQueue.push(audioFrame);
                    }
                }

                av_frame_free(&frame);
            }
        }

        return true;
    }

    std::shared_ptr<VideoFrame> ProcessVideoFrame(AVFrame* frame) {
        if (!_swsContext) {
            _swsContext =
                sws_getContext(_videoCodecContext->width, _videoCodecContext->height, _videoCodecContext->pix_fmt, _videoCodecContext->width, _videoCodecContext->height, AV_PIX_FMT_RGB24, SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);

            if (!_swsContext) {
                spdlog::error("Could not initialize swscale context");
                return nullptr;
            }
        }

        auto videoFrame = std::make_shared<VideoFrame>();
        videoFrame->width = _videoCodecContext->width;
        videoFrame->height = _videoCodecContext->height;
        videoFrame->linesize = _videoCodecContext->width * 3; // RGB24 = 3 bytes per pixel
        videoFrame->bufferPool = &_videoBufferPool;

        int bufferSize = av_image_get_buffer_size(AV_PIX_FMT_RGB24, videoFrame->width, videoFrame->height, 1);
        _videoBufferPool.SetBufferSize(bufferSize);
        videoFrame->data = _videoBufferPool.GetBuffer();

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
            videoFrame->timestamp = 0.0;
        }

        videoFrame->duration = _frameDuration;
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
                                          0, nullptr);

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
        audioFrame->bufferPool = &_audioBufferPool;

        int outSamples = swr_get_out_samples(_swrContext, frame->nb_samples);
        int outSize = av_samples_get_buffer_size(nullptr, 2, outSamples, AV_SAMPLE_FMT_FLT, 1);

        _audioBufferPool.SetBufferSize(outSize);
        audioFrame->data = _audioBufferPool.GetBuffer();

        if (!audioFrame->data) {
            spdlog::error("Could not allocate audio frame buffer");
            return nullptr;
        }

        uint8_t* outData[1] = {audioFrame->data};
        int convertedSamples = swr_convert(_swrContext, outData, outSamples, frame->data, frame->nb_samples);

        if (convertedSamples < 0) {
            spdlog::error("Error converting audio samples");
            _audioBufferPool.ReturnBuffer(audioFrame->data);
            audioFrame->data = nullptr;
            return nullptr;
        }

        audioFrame->size = av_samples_get_buffer_size(nullptr, 2, convertedSamples, AV_SAMPLE_FMT_FLT, 1);

        if (_audioCodecContext->sample_rate > 0) {
            audioFrame->duration = (double)convertedSamples / _audioCodecContext->sample_rate;
        } else {
            audioFrame->duration = 0.02; // Fallback to 20ms if sample rate is unknown
        }

        return audioFrame;
    }

    void SeekToPosition(double position) {
        if (!_formatContext || _videoStreamIndex < 0) {
            return;
        }

        {
            std::lock_guard<std::mutex> lock(_queuesMutex);
            std::queue<std::shared_ptr<VideoFrame>> emptyVideoQueue;
            std::swap(_videoFrameQueue, emptyVideoQueue);
            std::queue<std::shared_ptr<AudioFrame>> emptyAudioQueue;
            std::swap(_audioFrameQueue, emptyAudioQueue);
        }

        _initialAudioBuffered = false;
        _initialAudioBurstSent = false;
        _initialVideoFrameCaptured = false;

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
    bool IsPlaying() const { return _isPlaying; }
    PlaybackMode GetPlaybackMode() const { return _playbackMode; }
    double GetDuration() const { return _duration; }
    double GetCurrentPosition() const { return _currentPosition; }
    int GetVideoWidth() const { return _videoCodecContext ? _videoCodecContext->width : 0; }
    int GetVideoHeight() const { return _videoCodecContext ? _videoCodecContext->height : 0; }
    double GetFrameRate() const { return _frameRate; }
    double GetFrameDuration() const { return _frameDuration; }
    int GetColorDepth() const { return _colorDepth; }
    const std::string& GetPixelFormat() const { return _pixelFormat; }
    const std::vector<std::string>& GetVideoDecoders() const { return _videoDecoders; }
    const std::vector<std::string>& GetAudioDecoders() const { return _audioDecoders; }
    bool IsHardwareVideoDecoder(const std::string& codecName) const { return std::find(_hardwareVideoDecoders.begin(), _hardwareVideoDecoders.end(), codecName) != _hardwareVideoDecoders.end(); }
    bool IsHardwareAudioDecoder(const std::string& codecName) const { return std::find(_hardwareAudioDecoders.begin(), _hardwareAudioDecoders.end(), codecName) != _hardwareAudioDecoders.end(); }

    void SetPlaybackMode(PlaybackMode mode) {
        _playbackMode = mode;
        spdlog::info("Playback mode set to: {}", mode == PlaybackMode::RealTime ? "RealTime" : "Immediate");
    }
    
    void SetAudioFrameCallback(std::function<void(std::shared_ptr<AudioFrame>)> callback) {
        _audioFrameCallback = callback;
    }
    
    void Play() {
        if (!_running) {
            return;
        }

        if (!_isPlaying) {
            _isPlaying = true;
        }
    }

    void Pause() {
        if (_isPlaying) {
            _isPlaying = false;

            {
                std::lock_guard<std::mutex> lock(_queuesMutex);
                std::queue<std::shared_ptr<VideoFrame>> emptyVideoQueue;
                std::swap(_videoFrameQueue, emptyVideoQueue);

                std::queue<std::shared_ptr<AudioFrame>> emptyAudioQueue;
                std::swap(_audioFrameQueue, emptyAudioQueue);
            }

            _initialAudioBuffered = false;
            _initialAudioBurstSent = false;
            _initialVideoFrameCaptured = false;
        }
    }

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
        _hardwareVideoDecoders.clear();
        void* iter = nullptr;
        const AVCodec* codec = nullptr;

        while ((codec = av_codec_iterate(&iter))) {
            if (!av_codec_is_decoder(codec)) continue;
            if (codec->id != _formatContext->streams[_videoStreamIndex]->codecpar->codec_id) continue;

            _videoDecoders.push_back(codec->name);

            if (codec->capabilities & AV_CODEC_CAP_HARDWARE) {
                _hardwareVideoDecoders.push_back(codec->name);
            }
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
            _hardwareAudioDecoders.clear();
            void* iter = nullptr;
            const AVCodec* codec = nullptr;

            while ((codec = av_codec_iterate(&iter))) {
                if (!av_codec_is_decoder(codec)) continue;
                if (codec->id != _formatContext->streams[_audioStreamIndex]->codecpar->codec_id) continue;

                _audioDecoders.push_back(codec->name);

                if (codec->capabilities & AV_CODEC_CAP_HARDWARE) {
                    _hardwareAudioDecoders.push_back(codec->name);
                }
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

        if (_videoCodecContext->framerate.num > 0 && _videoCodecContext->framerate.den > 0) {
            _frameRate = (double)_videoCodecContext->framerate.num / _videoCodecContext->framerate.den;
        } else if (_formatContext->streams[_videoStreamIndex]->avg_frame_rate.num > 0 && _formatContext->streams[_videoStreamIndex]->avg_frame_rate.den > 0) {
            _frameRate = (double)_formatContext->streams[_videoStreamIndex]->avg_frame_rate.num / _formatContext->streams[_videoStreamIndex]->avg_frame_rate.den;
        } else {
            spdlog::warn("Could not determine video framerate, falling back to 30");
            _frameRate = 30.0;
        }

        _frameDuration = 1.0 / _frameRate;
        _colorDepth = 8;
        _pixelFormat = "unknown";

        if (_videoCodecContext->pix_fmt != AV_PIX_FMT_NONE) {
            const AVPixFmtDescriptor* desc = av_pix_fmt_desc_get(_videoCodecContext->pix_fmt);

            if (desc) {
                _colorDepth = desc->comp[0].depth;
                _pixelFormat = desc->name ? desc->name : "unknown";
            }
        }

        spdlog::info("Opened video file: {}, Duration: {:.2f} seconds, Resolution: {}x{}, Framerate: {:.2f} fps, Color depth: {} bits, Pixel format: {}{}", filePath, _duration, _videoCodecContext->width, _videoCodecContext->height,
                     _frameRate, _colorDepth, _pixelFormat, _audioStreamIndex >= 0 ? fmt::format(", Audio: {} channels, {} Hz", _audioCodecContext->ch_layout.nb_channels, _audioCodecContext->sample_rate) : "");

        _decodingThread = std::thread(&VideoDecoder::DecodingThread, this);
        return true;
    }

    void Close() {
        if (_running) {
            _isPlaying = false;
            _running = false;

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

        {
            std::lock_guard<std::mutex> lock(_queuesMutex);
            std::queue<std::shared_ptr<VideoFrame>> emptyVideoQueue;
            std::swap(_videoFrameQueue, emptyVideoQueue);

            std::queue<std::shared_ptr<AudioFrame>> emptyAudioQueue;
            std::swap(_audioFrameQueue, emptyAudioQueue);
        }

        _videoBufferPool.Clear();
        _audioBufferPool.Clear();

        _initialAudioBuffered = false;
        _initialAudioBurstSent = false;
        _initialVideoFrameCaptured = false;

        _videoStreamIndex = -1;
        _audioStreamIndex = -1;
        _duration = 0.0;
        _currentPosition = 0.0;
        _frameRate = 0.0;
        _colorDepth = 0;
        _pixelFormat = "unknown";
    }

    void Seek(double position) {
        if (!_running) {
            return;
        }

        _seekPosition = position;
        _seekRequested = true;
    }

    std::shared_ptr<VideoFrame> GetNextVideoFrame() {
        if (_playbackMode == PlaybackMode::RealTime) {
            std::unique_lock<std::mutex> lock(_queuesMutex);

            if (_initialVideoFrame) {
                spdlog::info("Initial video frame used");
                auto frame = _initialVideoFrame;
                _initialVideoFrame = nullptr;
                return frame;
            }

            if (!_isPlaying) {
                return nullptr;
            }

            if (!_initialAudioBurstSent) {
                return nullptr;
            }

            if (_videoFrameQueue.empty()) {
                return nullptr;
            }

            auto frame = _videoFrameQueue.front();
            auto now = std::chrono::high_resolution_clock::now();
            double elapsedTime = std::chrono::duration<double>(now - _startTimePoint).count();

            if (elapsedTime < _nextVideoFramePresentationTime) {
                return nullptr;
            }

            _nextVideoFramePresentationTime += frame->duration;

            _videoFrameQueue.pop();
            _videoFrameConsumedCV.notify_one();

            return frame;
        } else { // Immediate mode
            std::unique_lock<std::mutex> lock(_queuesMutex);

            while (_videoFrameQueue.empty() && _isPlaying) {
                _videoFrameAvailableCV.wait(lock);
            }

            if (!_isPlaying || _videoFrameQueue.empty()) {
                return nullptr;
            }

            auto frame = _videoFrameQueue.front();
            _videoFrameQueue.pop();

            if (_audioStreamIndex >= 0) {
                while (!_audioFrameQueue.empty()) {
                    frame->audioFrames.push_back(_audioFrameQueue.front());
                    _audioFrameQueue.pop();
                }
            }

            _videoFrameConsumedCV.notify_one();

            return frame;
        }
    }
};
