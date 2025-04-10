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
    std::string _name = "unnamed";

  public:
    BufferPool() = default;
    explicit BufferPool(const std::string& name) : _name(name) {}
    ~BufferPool() { Clear(); }

    void Clear() {
        std::lock_guard lock(_mutex);

        for (auto buffer : _availableBuffers) {
            if (buffer) {
                av_free(buffer);
            }
        }

        _availableBuffers.clear();
        _bufferSize = 0;

        spdlog::debug("Buffer pool '{}' cleared", _name);
    }

    void SetBufferSize(size_t size) {
        if (_bufferSize != size) {
            Clear();
            _bufferSize = size;
            spdlog::debug("Buffer pool '{}' size set to {}", _name, _bufferSize);
        }
    }

    uint8_t* GetBuffer() {
        std::lock_guard lock(_mutex);

        if (!_availableBuffers.empty()) {
            uint8_t* buffer = _availableBuffers.back();
            _availableBuffers.pop_back();
            return buffer;
        }

        spdlog::debug("Buffer added to buffer pool '{}'", _name);
        return static_cast<uint8_t*>(av_malloc(_bufferSize));
    }

    void ReturnBuffer(uint8_t* buffer) {
        if (!buffer) {
            return;
        }

        std::lock_guard lock(_mutex);
        _availableBuffers.push_back(buffer);
    }
};

struct AudioFrame {
    uint8_t* data = nullptr;
    int size = 0;
    double duration = 0.0;

    ~AudioFrame() {
        if (data) {
            av_free(data);
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
    std::vector<std::shared_ptr<AudioFrame>> audioFrames; // For Immediate mode
    std::function<void(uint8_t*)> bufferDeleter;

    ~VideoFrame() {
        if (data) {
            if (bufferDeleter) {
                bufferDeleter(data);
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
    BufferPool _videoBufferPool{"video"};

    std::thread _decodeThread;
    std::atomic<bool> _decodeThreadRunning{false};

    PlaybackMode _playbackMode = PlaybackMode::RealTime;
    std::atomic<bool> _isPlaying{false};
    std::atomic<bool> _seekRequested{false};
    double _seekPosition = 0.0;

    std::queue<std::shared_ptr<VideoFrame>> _videoFrameQueue;
    std::queue<std::shared_ptr<AudioFrame>> _audioFrameQueue;
    std::mutex _videoQueueMutex;
    std::mutex _audioQueueMutex;
    std::condition_variable _videoFrameAvailableCV;
    std::condition_variable _decodeSleepCV;
    std::mutex _decodeSleepMutex;
    const size_t _maxVideoFrameQueueSize = 4;

    std::shared_ptr<VideoFrame> _initialVideoFrame = nullptr;
    std::atomic<bool> _initialVideoFrameCaptured{false};
    const double _initialAudioDuration = 0.2;
    std::atomic<bool> _initialAudioBuffered{false};
    std::atomic<bool> _initialAudioSent{false};
    std::function<void(std::shared_ptr<AudioFrame>)> _audioFrameCallback;

    std::chrono::high_resolution_clock::time_point _playStartTime;
    double _nextVideoFramePresentationTime = 0.0;
    double _nextAudioFramePresentationTime = 0.0;
    bool _slowDecodeWarningShown = false;

    double _duration = 0.0;
    double _currentPosition = 0.0;
    double _frameRate = 0.0;
    double _frameDuration = 0.0;

    static const char* ErrorToString(int errnum) {
        thread_local char buffer[AV_ERROR_MAX_STRING_SIZE];
        return av_make_error_string(buffer, AV_ERROR_MAX_STRING_SIZE, errnum);
    }

    void DecodeThread() {
        spdlog::debug("Decode thread started");
        AVPacket* packet = av_packet_alloc();

        while (_decodeThreadRunning) {
            if (_seekRequested) {
                SeekToPosition(_seekPosition);
                _seekRequested = false;
            }

            if (!_initialVideoFrameCaptured) {
                spdlog::debug("Gathering initial video frame");

                while (!_initialVideoFrameCaptured && _decodeThreadRunning) {
                    int result = av_read_frame(_formatContext, packet);

                    if (result < 0) {
                        break;
                    }

                    if (!DecodePacket(packet)) {
                        av_packet_unref(packet);
                        _decodeThreadRunning = false;
                        break;
                    }

                    av_packet_unref(packet);
                }

                if (_initialVideoFrameCaptured) {
                    spdlog::debug("Initial video frame gathered");
                }
            }

            if (_seekRequested) {
                continue;
            }

            if (_playbackMode == PlaybackMode::RealTime && _audioStreamIndex >= 0 && _audioFrameCallback && !_initialAudioBuffered && _decodeThreadRunning) {
                spdlog::debug("Gathering initial audio frames");
                double totalAudioDuration = 0.0;

                while (totalAudioDuration < _initialAudioDuration && _decodeThreadRunning) {
                    int result = av_read_frame(_formatContext, packet);

                    if (result < 0) {
                        break;
                    }

                    if (!DecodePacket(packet)) {
                        av_packet_unref(packet);
                        _decodeThreadRunning = false;
                        break;
                    }

                    {
                        std::lock_guard lock(_audioQueueMutex);
                        totalAudioDuration = 0.0;
                        std::queue<std::shared_ptr<AudioFrame>> tempQueue = _audioFrameQueue;

                        while (!tempQueue.empty()) {
                            totalAudioDuration += tempQueue.front()->duration;
                            tempQueue.pop();
                        }
                    }

                    av_packet_unref(packet);

                    if (_seekRequested) {
                        break;
                    }
                }

                if (totalAudioDuration >= _initialAudioDuration) {
                    _initialAudioBuffered = true;
                    spdlog::debug("Initial audio frames gathered");
                }
            }

            if (_seekRequested) {
                continue;
            }

            if (!_isPlaying) {
                std::unique_lock lock(_decodeSleepMutex);
                _decodeSleepCV.wait_for(lock, std::chrono::milliseconds(1000));
                continue;
            }

            if (_playbackMode == PlaybackMode::RealTime && _audioStreamIndex >= 0 && _audioFrameCallback) {
                if (!_initialAudioSent) {
                    std::queue<std::shared_ptr<AudioFrame>> audioFramesToBurst;

                    {
                        std::lock_guard lock(_audioQueueMutex);
                        audioFramesToBurst = _audioFrameQueue;
                        std::queue<std::shared_ptr<AudioFrame>> emptyAudioQueue;
                        std::swap(_audioFrameQueue, emptyAudioQueue);
                    }

                    while (!audioFramesToBurst.empty()) {
                        auto audioFrame = audioFramesToBurst.front();
                        audioFramesToBurst.pop();
                        _audioFrameCallback(audioFrame);
                    }

                    _initialAudioSent = true;
                    _playStartTime = std::chrono::high_resolution_clock::now();
                    _nextVideoFramePresentationTime = 0.0;
                    _nextAudioFramePresentationTime = 0.0;
                    spdlog::debug("Initial audio frames sent");
                } else {
                    std::shared_ptr<AudioFrame> audioFrame = nullptr;
                    bool shouldSendAudio = false;

                    {
                        std::lock_guard lock(_audioQueueMutex);

                        if (!_audioFrameQueue.empty()) {
                            audioFrame = _audioFrameQueue.front();
                            auto now = std::chrono::high_resolution_clock::now();
                            double elapsedTime = std::chrono::duration<double>(now - _playStartTime).count();

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

            {
                std::lock_guard lock(_videoQueueMutex);

                if (_videoFrameQueue.size() >= _maxVideoFrameQueueSize) {
                    continue;
                }
            }

            int result = av_read_frame(_formatContext, packet);

            if (result < 0) {
                if (result == AVERROR_EOF) {
                    _isPlaying = false;
                    continue;
                }

                spdlog::error("Could not read frame: {}", ErrorToString(result));
                break;
            }

            if (!DecodePacket(packet)) {
                av_packet_unref(packet);
                _decodeThreadRunning = false;
                break;
            }

            if (_playbackMode == PlaybackMode::Immediate) {
                _videoFrameAvailableCV.notify_one();
            }

            av_packet_unref(packet);
        }

        _isPlaying = false;
        av_packet_free(&packet);
        spdlog::debug("Decode thread stopped");
    }

    void SeekToPosition(double position) {
        {
            std::lock_guard lock(_videoQueueMutex);
            std::queue<std::shared_ptr<VideoFrame>> emptyQueue;
            std::swap(_videoFrameQueue, emptyQueue);
        }

        {
            std::lock_guard lock(_audioQueueMutex);
            std::queue<std::shared_ptr<AudioFrame>> emptyQueue;
            std::swap(_audioFrameQueue, emptyQueue);
        }

        _initialVideoFrameCaptured = false;
        _initialAudioBuffered = false;
        _initialAudioSent = false;

        auto timestamp = (int64_t)(position * AV_TIME_BASE);

        if (_videoStreamIndex >= 0) {
            timestamp = av_rescale_q(timestamp, AV_TIME_BASE_Q, _formatContext->streams[_videoStreamIndex]->time_base);
        }

        int result = av_seek_frame(_formatContext, _videoStreamIndex, timestamp, AVSEEK_FLAG_BACKWARD);

        if (result < 0) {
            spdlog::error("Could not seek to position {}: {}", position, std::string(ErrorToString(result)));
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

    bool DecodePacket(const AVPacket* packet) {
        if (!packet) {
            spdlog::error("Could not decode packet: packet is null");
            return false;
        }

        if (packet->stream_index == _videoStreamIndex) {
            int result = avcodec_send_packet(_videoCodecContext, packet);

            if (result < 0) {
                spdlog::error("Could not send video packet to decoder: {}", ErrorToString(result));
                return false;
            }

            while (true) {
                AVFrame* frame = av_frame_alloc();
                result = avcodec_receive_frame(_videoCodecContext, frame);

                if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
                    av_frame_free(&frame);
                    break;
                }

                if (result < 0) {
                    spdlog::error("Could not receive video frame from decoder: {}", ErrorToString(result));
                    av_frame_free(&frame);
                    return false;
                }

                if (auto videoFrame = ProcessVideoFrame(frame)) {
                    {
                        std::lock_guard lock(_videoQueueMutex);
                        _videoFrameQueue.push(videoFrame);
                    }

                    _currentPosition = videoFrame->timestamp;

                    if (!_initialVideoFrameCaptured) {
                        _initialVideoFrame = videoFrame;
                        _initialVideoFrameCaptured = true;
                    }
                }

                av_frame_free(&frame);
            }
        } else if (packet->stream_index == _audioStreamIndex) {
            int result = avcodec_send_packet(_audioCodecContext, packet);

            if (result < 0) {
                spdlog::error("Could not send audio packet to decoder: {}", ErrorToString(result));
                return false;
            }

            while (true) {
                AVFrame* frame = av_frame_alloc();
                result = avcodec_receive_frame(_audioCodecContext, frame);

                if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
                    av_frame_free(&frame);
                    break;
                }

                if (result < 0) {
                    spdlog::error("Could not receive audio frame from decoder: {}", ErrorToString(result));
                    av_frame_free(&frame);
                    return false;
                }

                if (auto audioFrame = ProcessAudioFrame(frame)) {
                    std::lock_guard lock(_audioQueueMutex);
                    _audioFrameQueue.push(audioFrame);
                }

                av_frame_free(&frame);
            }
        }

        return true;
    }

    std::shared_ptr<VideoFrame> ProcessVideoFrame(const AVFrame* frame) {
        if (!_swsContext) {
            _swsContext = sws_getContext(_videoCodecContext->width,
                                         _videoCodecContext->height,
                                         _videoCodecContext->pix_fmt,
                                         _videoCodecContext->width,
                                         _videoCodecContext->height,
                                         AV_PIX_FMT_RGB24,
                                         SWS_FAST_BILINEAR,
                                         nullptr,
                                         nullptr,
                                         nullptr);

            if (!_swsContext) {
                spdlog::error("Could not create sws context");
                return nullptr;
            }

            int bufferSize = av_image_get_buffer_size(AV_PIX_FMT_RGB24, _videoCodecContext->width, _videoCodecContext->height, 1);
            _videoBufferPool.SetBufferSize(bufferSize);
        }

        auto videoFrame = std::make_shared<VideoFrame>();
        videoFrame->width = _videoCodecContext->width;
        videoFrame->height = _videoCodecContext->height;
        videoFrame->linesize = _videoCodecContext->width * 3; // RGB24 = 3 bytes per pixel
        videoFrame->data = _videoBufferPool.GetBuffer();

        if (!videoFrame->data) {
            spdlog::error("Could not get video frame buffer from pool");
            return nullptr;
        }

        videoFrame->bufferDeleter = [this](uint8_t* buffer) { _videoBufferPool.ReturnBuffer(buffer); };

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

    std::shared_ptr<AudioFrame> ProcessAudioFrame(const AVFrame* frame) {
        if (!_audioCodecContext) {
            return nullptr;
        }

        if (!_swrContext) {
            _swrContext = swr_alloc();

            if (!_swrContext) {
                spdlog::error("Could not allocate swresample context");
                return nullptr;
            }

            AVChannelLayout outLayout;
            av_channel_layout_default(&outLayout, 2);

            int result = swr_alloc_set_opts2(&_swrContext,
                                             &outLayout,                      // out_ch_layout
                                             AV_SAMPLE_FMT_FLT,               // out_sample_fmt
                                             48000,                           // out_sample_rate
                                             &_audioCodecContext->ch_layout,  // in_ch_layout
                                             _audioCodecContext->sample_fmt,  // in_sample_fmt
                                             _audioCodecContext->sample_rate, // in_sample_rate
                                             0,
                                             nullptr);

            if (result < 0) {
                spdlog::error("Could not set swresample options: {}", ErrorToString(result));
                swr_free(&_swrContext);
                return nullptr;
            }

            result = swr_init(_swrContext);

            if (result < 0) {
                spdlog::error("Could not initialize swresample context: {}", ErrorToString(result));
                return nullptr;
            }
        }

        auto audioFrame = std::make_shared<AudioFrame>();
        int outSamples = swr_get_out_samples(_swrContext, frame->nb_samples);
        int outSize = av_samples_get_buffer_size(nullptr, 2, outSamples, AV_SAMPLE_FMT_FLT, 1);
        audioFrame->data = static_cast<uint8_t*>(av_malloc(outSize));

        if (!audioFrame->data) {
            spdlog::error("Could not allocate audio frame buffer");
            return nullptr;
        }

        uint8_t* outData[1] = {audioFrame->data};
        int convertedSamples = swr_convert(_swrContext, outData, outSamples, frame->data, frame->nb_samples);

        if (convertedSamples < 0) {
            spdlog::error("Could not convert audio samples: {}", ErrorToString(convertedSamples));
            av_free(audioFrame->data);
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

  public:
    VideoDecoder() = default;

    bool IsPlaying() const { return _isPlaying; }
    double GetDuration() const { return _duration; }
    double GetCurrentPosition() const { return _currentPosition; }
    int GetVideoWidth() const { return _videoCodecContext ? _videoCodecContext->width : 0; }
    int GetVideoHeight() const { return _videoCodecContext ? _videoCodecContext->height : 0; }
    const std::vector<std::string>& GetVideoDecoders() const { return _videoDecoders; }
    const std::vector<std::string>& GetAudioDecoders() const { return _audioDecoders; }
    void SetAudioFrameCallback(const std::function<void(std::shared_ptr<AudioFrame>)>& callback) { _audioFrameCallback = callback; }
    bool IsHardwareVideoDecoder(const std::string& codecName) const { return std::ranges::find(_hardwareVideoDecoders, codecName) != _hardwareVideoDecoders.end(); }
    bool IsHardwareAudioDecoder(const std::string& codecName) const { return std::ranges::find(_hardwareAudioDecoders, codecName) != _hardwareAudioDecoders.end(); }

    bool Init(const std::string& filePath) {
        spdlog::debug("Initializing video decoder for file: {}", filePath);

        _formatContext = avformat_alloc_context();
        int result = avformat_open_input(&_formatContext, filePath.c_str(), nullptr, nullptr);

        if (result < 0) {
            spdlog::error("Could not open input file: {}", ErrorToString(result));
            Close();
            return false;
        }

        result = avformat_find_stream_info(_formatContext, nullptr);

        if (result < 0) {
            spdlog::error("Could not find stream information: {}", ErrorToString(result));
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

        const AVCodec* defaultVideoCodec = avcodec_find_decoder(_formatContext->streams[_videoStreamIndex]->codecpar->codec_id);

        if (!defaultVideoCodec) {
            spdlog::error("Unsupported video codec");
            Close();
            return false;
        }

        _videoDecoders.clear();
        _hardwareVideoDecoders.clear();

        void* iter = nullptr;
        const AVCodec* codec = nullptr;

        while ((codec = av_codec_iterate(&iter))) {
            if (!av_codec_is_decoder(codec)) {
                continue;
            }

            if (codec->id != _formatContext->streams[_videoStreamIndex]->codecpar->codec_id) {
                continue;
            }

            _videoDecoders.emplace_back(codec->name);

            if (codec->capabilities & AV_CODEC_CAP_HARDWARE) {
                _hardwareVideoDecoders.emplace_back(codec->name);
            }
        }

        const AVCodec* videoCodec = defaultVideoCodec;

        if (!Session::Instance().videoDecoder.empty()) {
            if (const auto it = std::ranges::find(_videoDecoders, Session::Instance().videoDecoder); it != _videoDecoders.end()) {
                if (const AVCodec* requestedCodec = avcodec_find_decoder_by_name(Session::Instance().videoDecoder.c_str())) {
                    videoCodec = requestedCodec;
                } else {
                    spdlog::warn("Requested video codec '{}' not found, falling back to default", Session::Instance().videoDecoder);
                }
            } else {
                spdlog::warn("Requested video codec '{}' not supported for this video, falling back to default", Session::Instance().videoDecoder);
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

        result = avcodec_parameters_to_context(_videoCodecContext, _formatContext->streams[_videoStreamIndex]->codecpar);

        if (result < 0) {
            spdlog::error("Could not copy video codec parameters: {}", ErrorToString(result));
            Close();
            return false;
        }

        _videoCodecContext->pkt_timebase = _formatContext->streams[_videoStreamIndex]->time_base;
        result = avcodec_open2(_videoCodecContext, videoCodec, nullptr);

        if (result < 0) {
            spdlog::error("Could not open video codec: {}", ErrorToString(result));
            Close();
            return false;
        }

        if (_audioStreamIndex >= 0) {
            const AVCodec* defaultAudioCodec = avcodec_find_decoder(_formatContext->streams[_audioStreamIndex]->codecpar->codec_id);

            _audioDecoders.clear();
            _hardwareAudioDecoders.clear();

            iter = nullptr;

            while ((codec = av_codec_iterate(&iter))) {
                if (!av_codec_is_decoder(codec)) {
                    continue;
                }

                if (codec->id != _formatContext->streams[_audioStreamIndex]->codecpar->codec_id) {
                    continue;
                }

                _audioDecoders.emplace_back(codec->name);

                if (codec->capabilities & AV_CODEC_CAP_HARDWARE) {
                    _hardwareAudioDecoders.emplace_back(codec->name);
                }
            }

            if (!defaultAudioCodec) {
                spdlog::warn("Unsupported audio codec");
                _audioStreamIndex = -1;
            } else {
                const AVCodec* audioCodec = defaultAudioCodec;

                if (!Session::Instance().audioDecoder.empty()) {
                    if (const auto it = std::ranges::find(_audioDecoders, Session::Instance().audioDecoder); it != _audioDecoders.end()) {
                        if (const AVCodec* requestedCodec = avcodec_find_decoder_by_name(Session::Instance().audioDecoder.c_str())) {
                            audioCodec = requestedCodec;
                        } else {
                            spdlog::warn("Requested audio codec '{}' not found, falling back to default", Session::Instance().audioDecoder);
                        }
                    } else {
                        spdlog::warn("Requested audio codec '{}' not supported for this audio, falling back to default", Session::Instance().audioDecoder);
                    }
                }

                Session::Instance().audioDecoder = audioCodec->name;
                spdlog::info("Selected audio decoder: {} ({})", audioCodec->name, audioCodec->long_name ? audioCodec->long_name : "unknown");
                _audioCodecContext = avcodec_alloc_context3(audioCodec);

                if (!_audioCodecContext) {
                    spdlog::warn("Could not allocate audio codec context");
                    _audioStreamIndex = -1;
                } else {
                    result = avcodec_parameters_to_context(_audioCodecContext, _formatContext->streams[_audioStreamIndex]->codecpar);

                    if (result < 0) {
                        spdlog::warn("Could not copy audio codec parameters: {}", ErrorToString(result));
                        avcodec_free_context(&_audioCodecContext);
                        _audioStreamIndex = -1;
                    } else {
                        _audioCodecContext->pkt_timebase = _formatContext->streams[_audioStreamIndex]->time_base;
                        result = avcodec_open2(_audioCodecContext, audioCodec, nullptr);

                        if (result < 0) {
                            spdlog::warn("Could not open audio codec: {}", ErrorToString(result));
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
        int colorDepth = 0;
        std::string pixelFormat = "unknown";

        if (_videoCodecContext->pix_fmt != AV_PIX_FMT_NONE) {
            if (const AVPixFmtDescriptor* desc = av_pix_fmt_desc_get(_videoCodecContext->pix_fmt)) {
                colorDepth = desc->comp[0].depth;
                pixelFormat = desc->name ? desc->name : "unknown";
            }
        }

        spdlog::info("Opened video file: {}, Duration: {:.2f} seconds, Resolution: {}x{}, Framerate: {:.2f} fps, Color depth: {} bits, Pixel format: {}{}",
                     filePath,
                     _duration,
                     _videoCodecContext->width,
                     _videoCodecContext->height,
                     _frameRate,
                     colorDepth,
                     pixelFormat,
                     _audioStreamIndex >= 0 ? fmt::format(", Audio: {} channels, {} Hz", _audioCodecContext->ch_layout.nb_channels, _audioCodecContext->sample_rate) : "");

        _decodeThreadRunning = true;
        _decodeThread = std::thread(&VideoDecoder::DecodeThread, this);

        return true;
    }

    void Close() {
        spdlog::debug("Closing video decoder");
        _isPlaying = false;

        if (_decodeThreadRunning) {
            _decodeThreadRunning = false;
            _decodeSleepCV.notify_one();
        }

        if (_decodeThread.joinable()) {
            _decodeThread.join();
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
            std::lock_guard lock(_videoQueueMutex);
            std::queue<std::shared_ptr<VideoFrame>> emptyQueue;
            std::swap(_videoFrameQueue, emptyQueue);
        }

        {
            std::lock_guard lock(_audioQueueMutex);
            std::queue<std::shared_ptr<AudioFrame>> emptyQueue;
            std::swap(_audioFrameQueue, emptyQueue);
        }

        _videoStreamIndex = -1;
        _audioStreamIndex = -1;

        _seekPosition = 0.0;
        _seekRequested = false;

        _initialVideoFrame = nullptr;
        _initialVideoFrameCaptured = false;
        _initialAudioBuffered = false;
        _initialAudioSent = false;

        _nextVideoFramePresentationTime = 0.0;
        _nextAudioFramePresentationTime = 0.0;
        _slowDecodeWarningShown = false;

        _duration = 0.0;
        _currentPosition = 0.0;
        _frameRate = 0.0;
        _frameDuration = 0.0;

        _videoBufferPool.Clear();
    }

    void Play() {
        if (!_decodeThreadRunning) {
            return;
        }

        if (!_isPlaying) {
            _isPlaying = true;
            _decodeSleepCV.notify_one();
        }
    }

    void Pause() {
        if (!_decodeThreadRunning) {
            return;
        }

        if (_isPlaying) {
            _isPlaying = false;
            _initialAudioBuffered = false;
            _initialAudioSent = false;
            _slowDecodeWarningShown = false;
            _decodeSleepCV.notify_one();
        }
    }

    void Seek(double position) {
        if (!_decodeThreadRunning) {
            return;
        }

        _seekPosition = position;
        _seekRequested = true;
        _decodeSleepCV.notify_one();
    }

    std::shared_ptr<VideoFrame> GetNextVideoFrame() {
        if (_playbackMode == PlaybackMode::RealTime) {
            if (_initialVideoFrame) {
                spdlog::debug("Initial video frame used");
                auto frame = _initialVideoFrame;
                _initialVideoFrame = nullptr;
                return frame;
            }

            if (!_isPlaying) {
                return nullptr;
            }

            if (!_initialAudioSent) {
                return nullptr;
            }

            std::lock_guard lock(_videoQueueMutex);

            if (_videoFrameQueue.empty()) {
                return nullptr;
            }

            auto frame = _videoFrameQueue.front();
            auto now = std::chrono::high_resolution_clock::now();
            double elapsedTime = std::chrono::duration<double>(now - _playStartTime).count();

            if (elapsedTime < _nextVideoFramePresentationTime) {
                return nullptr;
            }

            if (!_slowDecodeWarningShown && elapsedTime > _nextVideoFramePresentationTime + _videoFrameQueue.front()->duration) {
                spdlog::warn("Video decoding cannot keep up, expect slower video playback and audio glitches");
                _slowDecodeWarningShown = true;
            }

            _nextVideoFramePresentationTime += frame->duration;
            _videoFrameQueue.pop();

            return frame;
        } else { // Immediate mode
            std::unique_lock lock(_videoQueueMutex);

            while (_videoFrameQueue.empty() && _isPlaying) {
                _videoFrameAvailableCV.wait(lock);
            }

            if (!_isPlaying || _videoFrameQueue.empty()) {
                return nullptr;
            }

            auto frame = _videoFrameQueue.front();
            _videoFrameQueue.pop();

            if (_audioStreamIndex >= 0) {
                std::lock_guard audioLock(_audioQueueMutex);
                while (!_audioFrameQueue.empty()) {
                    frame->audioFrames.push_back(_audioFrameQueue.front());
                    _audioFrameQueue.pop();
                }
            }

            return frame;
        }
    }
};
