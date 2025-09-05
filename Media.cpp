#include "Media.h"
#include <iostream>
#include <QImage>


bool doSeek = false;
bool audioStop = false;
double PerSampleTime = 0.0f;
Media::Media(void* win)
{
    // 初始化SDL
    SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
    is_running = true;
    // 启动解码线程和视频线程
    decode_thread = std::thread(&Media::decodeThread, this);
    video_thread = std::thread(&Media::videoThread, this);
    // 分配帧内存
    audioFrame = av_frame_alloc();
    dstVideoFrame = av_frame_alloc();
    srcVideoFrame = av_frame_alloc();
    lowVideoFrame = av_frame_alloc();
    // 初始化目标视频帧
    dstVideoFrame->width = 1280;
    dstVideoFrame->height = 720;
    dstVideoFrame->format = AV_PIX_FMT_YUV420P;
    if (av_frame_get_buffer(dstVideoFrame, 32) < 0) {
        fprintf(stderr, "Could not allocate the frame data for dFrame\n");
        av_frame_free(&audioFrame);
        av_frame_free(&srcVideoFrame);
        av_frame_free(&dstVideoFrame);
        return;
    }
    // 初始化低分辨率视频帧
    lowVideoFrame->width = 640;
    lowVideoFrame->height = 480;
    lowVideoFrame->format = AV_PIX_FMT_YUV420P;
    if (av_frame_get_buffer(lowVideoFrame, 32) < 0) {
        fprintf(stderr, "Could not allocate the frame data for dFrame\n");
        av_frame_free(&audioFrame);
        av_frame_free(&srcVideoFrame);
        av_frame_free(&dstVideoFrame);
        av_frame_free(&lowVideoFrame);
        return;
    }
    // 创建SDL窗口和渲染器
    window = SDL_CreateWindowFrom(win);
    if (!window)
    {
        fprintf(stderr, "SDL: could not create window, err:%s\n", SDL_GetError());
        return;
    }
    // 基于窗口创建渲染器
    renderer = SDL_CreateRenderer(window, -1, 0);
    // 基于渲染器创建纹理
    texture = SDL_CreateTexture(renderer,
        pixformat,
        SDL_TEXTUREACCESS_STREAMING,
        1280,
        720);
    printf("Media Init ok\n");
    fflush(NULL);
}

Media::~Media()
{
    is_running = false;
    cv.notify_all();
    ClearQueue();
    if (s_audio_buf) 
    {
        free(s_audio_buf);
        s_audio_buf = nullptr;
    }
    if (video_thread.joinable())
    {
        video_thread.join();
    }
    if (decode_thread.joinable())
    {
        decode_thread.join();
    }
    deInitDecode();
    SDL_CloseAudio();
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    av_frame_free(&audioFrame);
    av_frame_free(&srcVideoFrame);
    av_frame_free(&dstVideoFrame);
    av_frame_free(&lowVideoFrame);
}

void Media::SetFileName(QString file)
{
    printf("Do setfileName: %s\n", fileName.toStdString().c_str());
    fflush(NULL);
    if (file.size() == 0)
    {
        return;
    }
    fileName = file;
    if (this->state == FINISH)
    {
        SDL_CloseAudio();
        s_audio_pos = s_audio_end;
        //将队列都清空
        ClearQueue();

        deInitDecode();
        InitDecode();
        InitAudio();

        this->state = NONE;
    }
    else if (this->state == PLAY)
    {
        this->state = STOP;
        cv.notify_all();
        SDL_CloseAudio();
        s_audio_pos = s_audio_end;
        while (!videoStop || !decodeStop)
        {
            SDL_Delay(5);
        }
        printf("All stop\n");
        fflush(NULL);
        //将队列都清空
        ClearQueue();
        printf("Clear Queue ok\n");
        fflush(NULL);
        deInitDecode();
        printf("de Init decode ok\n");
        fflush(NULL);
        InitDecode();
        printf("Init decode ok\n");
        fflush(NULL);
        InitAudio();
        this->state = NONE;
    }
    else if (this->state == STOP)
    {
        SDL_CloseAudio();
        s_audio_pos = s_audio_end;

        //将队列都清空
        ClearQueue();

        deInitDecode();
        InitDecode();
        InitAudio();
    }
    else if (this->state == NONE)
    {
        InitDecode();
        InitAudio();
    }
    LastState = SET;
    if (videoCodecCtx->width == 1280)
    {
        suppLevel = High;
    }
    else 
    {
        suppLevel = Low;
    }
    this->Time = 1.0f;
}

void Media::InitDecode()
{
    //// 初始化FFmpeg解封装器
    fmt_ctx = avformat_alloc_context();
    int ret = avformat_open_input(&fmt_ctx, fileName.toStdString().c_str(), NULL, NULL);
    if (ret < 0) 
    {
        char err_buf[128];
        av_strerror(ret, err_buf, sizeof(err_buf));
        std::cout << "Could not open source file " << fileName.toStdString() << ": " << err_buf << std::endl;
        return;
    }

    ret = avformat_find_stream_info(fmt_ctx, NULL);
    if (ret < 0) 
    {
        char err_buf[128];
        av_strerror(ret, err_buf, sizeof(err_buf));
        std::cout << "Failed to retrieve input stream information: " << err_buf << std::endl;
        avformat_close_input(&fmt_ctx);
        return;
    }

    av_dump_format(fmt_ctx, 0, fileName.toStdString().c_str(), 0);
    //// 初始化FFmpeg解码器
    uint8_t i;
    for (i = 0; i < fmt_ctx->nb_streams; i++) {
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) //视频解码器
        {
            videoIndex = i;
            const AVCodec* codec = avcodec_find_decoder(fmt_ctx->streams[i]->codecpar->codec_id);
            videoCodecCtx = avcodec_alloc_context3(codec);
            avcodec_parameters_to_context(videoCodecCtx, fmt_ctx->streams[i]->codecpar);
            avcodec_open2(videoCodecCtx, codec, NULL);
        }
        else if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) //音频解码器
        {
            audioIndex = i;
            const AVCodec* codec = avcodec_find_decoder(fmt_ctx->streams[i]->codecpar->codec_id);
            audioCodecCtx = avcodec_alloc_context3(codec);
            avcodec_parameters_to_context(audioCodecCtx, fmt_ctx->streams[i]->codecpar);
            avcodec_open2(audioCodecCtx, codec, NULL);
            audioCodecCtx->pkt_timebase = fmt_ctx->streams[i]->time_base;
        }
    }

    if (videoIndex == -1 || audioIndex == -1) {
        avformat_close_input(&fmt_ctx);
        printf("Error\n");
        return;
    }
    // 做分辨率转1280*720的转换
    swsCtx = sws_getContext(fmt_ctx->streams[videoIndex]->codecpar->width,// 原视频宽度
        fmt_ctx->streams[videoIndex]->codecpar->height,// 原视频高度
        (AVPixelFormat)fmt_ctx->streams[videoIndex]->codecpar->format,// 原视频的格式
        1280,
        720,// 转化后图片的宽高
        AV_PIX_FMT_YUV420P, SWS_BILINEAR, NULL, NULL, NULL);
    // 做yuv转rgb的转换（截图）
    saveCtx = sws_getContext(fmt_ctx->streams[videoIndex]->codecpar->width,// 原视频宽度
        fmt_ctx->streams[videoIndex]->codecpar->height,// 原视频高度
        (AVPixelFormat)fmt_ctx->streams[videoIndex]->codecpar->format,// 原视频的格式
        fmt_ctx->streams[videoIndex]->codecpar->width,
        fmt_ctx->streams[videoIndex]->codecpar->height,// 转化后图片的宽高
        AV_PIX_FMT_RGB24, SWS_BILINEAR, NULL, NULL, NULL);
    // 做1280*720 --> 640*480的转换（低分辨率）
    lowCtx = sws_getContext(fmt_ctx->streams[videoIndex]->codecpar->width,// 原视频宽度
        fmt_ctx->streams[videoIndex]->codecpar->height,// 原视频高度
        (AVPixelFormat)fmt_ctx->streams[videoIndex]->codecpar->format,// 原视频的格式
        640,
        480,// 转化后图片的宽高
        AV_PIX_FMT_YUV420P, SWS_BILINEAR, NULL, NULL, NULL);

    highCtx = sws_getContext(640,// 原视频宽度
        480,// 原视频高度
        AV_PIX_FMT_YUV420P,// 原视频的格式
        1280,
        720,// 转化后图片的宽高
        AV_PIX_FMT_YUV420P, SWS_BILINEAR, NULL, NULL, NULL);

    saveFrame = av_frame_alloc();
    saveFrame->format = fmt_ctx->streams[videoIndex]->codecpar->format;
    saveFrame->width = fmt_ctx->streams[videoIndex]->codecpar->width;
    saveFrame->height = fmt_ctx->streams[videoIndex]->codecpar->height;
    av_frame_get_buffer(saveFrame, 32);
    printf("Init Ok\n");
    fflush(NULL);

    totalTime = (int)(fmt_ctx->duration * 1.0 / AV_TIME_BASE);
}

void Media::deInitDecode()
{
    avformat_close_input(&fmt_ctx);
    avcodec_free_context(&videoCodecCtx);
    avcodec_free_context(&audioCodecCtx);
    sws_freeContext(swsCtx);
    sws_freeContext(saveCtx);
    sws_freeContext(lowCtx);
    av_frame_free(&saveFrame);
    printf("free ok\n");
    fflush(NULL);
    lowCtx = NULL;
    saveCtx = NULL;
    swsCtx = NULL;
    fmt_ctx = NULL;
    videoCodecCtx = NULL;
    audioCodecCtx = NULL;
    printf("deInitDecode\n");
    fflush(NULL);
    totalTime = 0;
}

int CopyFrame(AVFrame* srcFrame, AVFrame* dstFrame) {
    // 确保目标帧已经被分配
    if (!dstFrame->buf[0]) {
        if (av_frame_ref(dstFrame, srcFrame) < 0) {
            // 复制失败
            return -1;
        }
    }
    else {
        // 复制帧数据
        if (av_frame_copy(dstFrame, srcFrame) < 0) {
            // 复制失败
            return -1;
        }
    }
    return 0;
}

void Media::decodeThread()
{
    while (is_running)
    {
        if (this->state == NONE)
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock);
            continue;
        }
        else if (this->state == STOP)
        {
            printf("Decode Stop\n");
            fflush(NULL);
            decodeStop = true;
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock);
            decodeStop = false;
        }
        else if (this->state == PLAY)
        {
            AVPacket* pkt = av_packet_alloc();
            if (audioQueue.size() >= 200 && videoQueue.size() >= 200)
            {
                if (audioStop)
                {
                    SDL_PauseAudio(0);
                    audioStop = false;
                }
                SDL_Delay(5);
                continue;
            }
            int ret;
            ret = av_read_frame(fmt_ctx, pkt);
            if (ret == AVERROR_EOF)
            {
                printf("Read finish\n");
                fflush(NULL);
                if (audioStop)
                {
                    SDL_PauseAudio(0);
                    audioStop = false;
                }
                decodeFinish = true;
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock);
                decodeFinish = false;
                continue;
            }//数据读取完成了
            else if (pkt->stream_index == videoIndex)
            {
                videoQueue.push(pkt);
            }
            else if (pkt->stream_index == audioIndex) 
            {
                audioQueue.push(pkt);
            }
        }
    }
}

void Media::videoThread()
{
    while (is_running)
    {
        if (this->state == NONE)
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock);
        }
        else if (this->state == STOP)
        {
            printf("videoStop\n");
            fflush(NULL);
            videoStop = true;
            if (this->dt != 0)
            {
                SDL_Delay(dt * 1000);
                this->stopCallback();
                continue;
            }
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock);
            videoStop = false;
            continue;
        }
        else if (this->state == PLAY)
        {
            if (videoQueue.size() > 0) 
            {
                avcodec_send_packet(videoCodecCtx, videoQueue.front());
                av_packet_free(&videoQueue.front());
                videoQueue.pop();
                while (avcodec_receive_frame(videoCodecCtx, srcVideoFrame) >= 0) {
                    CopyFrame(srcVideoFrame, saveFrame);
                    //当前的这个视频只支持480p的画质播放，没法转换为高画质
                    if (suppLevel == Low) 
                    {
                        sws_scale(swsCtx,
                            (const uint8_t* const*)srcVideoFrame->data,
                            srcVideoFrame->linesize,
                            0,
                            videoCodecCtx->height,
                            dstVideoFrame->data,
                            dstVideoFrame->linesize);
                        RenderPicture(dstVideoFrame);
                        DelayVideo(srcVideoFrame->pts);
                    }//当前视频的画质可以高低进行切换
                    else 
                    {
                        if (this->level == High)
                        {
                            sws_scale(swsCtx,
                                (const uint8_t* const*)srcVideoFrame->data,
                                srcVideoFrame->linesize,
                                0,
                                videoCodecCtx->height,
                                dstVideoFrame->data,
                                dstVideoFrame->linesize);

                            RenderPicture(dstVideoFrame);
                            DelayVideo(srcVideoFrame->pts);
                        }
                        //需要先转为640*480，然后在转成1280*720才能正常播放
                        else 
                        {
                            sws_scale(lowCtx,
                                (const uint8_t* const*)srcVideoFrame->data,
                                srcVideoFrame->linesize,
                                0,
                                videoCodecCtx->height,
                                lowVideoFrame->data,
                                lowVideoFrame->linesize);

                            sws_scale(highCtx,
                                (const uint8_t* const*)lowVideoFrame->data,
                                lowVideoFrame->linesize,
                                0,
                                lowVideoFrame->height,
                                dstVideoFrame->data,
                                dstVideoFrame->linesize);
                            RenderPicture(dstVideoFrame);
                            DelayVideo(srcVideoFrame->pts);
                        }
                    }
                }

            }
            else 
            {
                std::cout << "VideoQueue is NONE: " << videoQueue.size() << std::endl;
                if (decodeFinish == true) 
                {
                    this->state = FINISH;
                    std::unique_lock<std::mutex> lock(mtx);
                    cv.wait(lock);
                    decodeFinish = false;
                    std::cout << "Wake up" << std::endl;
                }
                SDL_Delay(10);
            }
        }
    }
}

void AudioCallback(void* data, uint8_t* stream, int len)
{
    Media* p = static_cast<Media*>(data);
    //应该在这里更新timeStamp
    // 清空音频流
    SDL_memset(stream, 0, len);
    p->AudioTs = p->AudioTs + PerSampleTime;
    // 检查是否已经没有更多数据
    if (p->s_audio_pos >= p->s_audio_end) 
    {
        if (p->audioQueue.size() == 0) 
        {
            if (p->decodeStop == true) 
            {
                std::cout << "audio finish play" << std::endl;
                SDL_PauseAudio(1);
                return;
            }
            else 
            {
                std::cout << "AqSize: " << p->audioQueue.size() << std::endl;
                SDL_PauseAudio(1);
                audioStop = true;
                std::cout << "Pause Audio" << std::endl;
                return;
            }
        }
        avcodec_send_packet(p->audioCodecCtx, p->audioQueue.front());
        av_packet_free(&p->audioQueue.front());
        p->audioQueue.pop();
        avcodec_receive_frame(p->audioCodecCtx, p->audioFrame);
        p->AudioTs = p->audioFrame->pts * av_q2d(p->fmt_ctx->streams[p->audioIndex]->time_base);
        if (p->fmt_ctx->streams[p->audioIndex]->codecpar->format == AV_SAMPLE_FMT_FLTP) 
        {
            p->PlayFltp();
        }
        else 
        {
            p->PlayS16();
        }
        if (doSeek)
        {
            printf("do WakeUp Audio: %f\n", p->AudioTs);
            fflush(NULL);
            p->seekCv.notify_all();
            doSeek = false;
        }
    }
    if (p->AudioTs >= p->Time)
    {
        printf("Update AudioTs: %f\n", p->AudioTs);
        fflush(NULL);
        p->Time += 1.0f;
        p->LastAudioTs = p->AudioTs;
        p->updateCallback();
    }
    // 计算实际可用的数据长度
    int remain_buffer_len = p->s_audio_end - p->s_audio_pos;
    len = (len < remain_buffer_len) ? len : remain_buffer_len;
    // 将数据从缓存填充到音频流
    SDL_MixAudio(stream, p->s_audio_pos, len, p->volume);
    // 更新缓存中的当前位置
    p->s_audio_pos += len;
}

void Media::InitAudio()
{
    if (audioCodecCtx->sample_fmt == AV_SAMPLE_FMT_S16)
    {
        spec.format = AUDIO_S16;
    }
    else 
    {
        spec.format = AUDIO_F32;
    }
    spec.freq = audioCodecCtx->sample_rate * Speed;
    //spec.channels = 2;             // 单声道
    spec.channels = audioCodecCtx->channels;
    spec.silence = 0;              // 无需设置静音值（不适用浮点格式）
    spec.samples = 1024; // 每次音频回调处理的样本数量
    spec.callback = AudioCallback; // 音频数据填充回调函数
    spec.userdata = (void*)this;          // 回调函数数据（未使用）

    SDL_OpenAudio(&spec, NULL);
    if (s_audio_buf)
    {
        free(s_audio_buf);
        s_audio_buf = NULL;
    }
    s_audio_buf = (uint8_t*)malloc(audioCodecCtx->sample_rate * 2 * 4);
    PerSampleTime = (double)1024 / (double)audioCodecCtx->sample_rate;
    //进行四舍五入，只保留小数点后3位
    PerSampleTime = round(PerSampleTime * 1000) / 1000;
    printf("PerSTime: %f\n", PerSampleTime);
    fflush(NULL);
}

void Media::Seek(int pos)
{
    Time = pos;
    if (this->state == PLAY) 
    {
        s_audio_pos = s_audio_end;
        SDL_PauseAudio(1);
        this->state = STOP;
        cv.notify_all();
        while (!videoStop || !decodeStop)
        {
            SDL_Delay(5);
        }
        printf("All in Seek\n");
        fflush(NULL);
        ClearQueue();
    }
    else if (this->state == STOP)
    {
        s_audio_pos = s_audio_end;
        ClearQueue();
    }

    int64_t VideoTimeStamp = pos / av_q2d(fmt_ctx->streams[videoIndex]->time_base);
    int64_t AudioTimeStamp = pos / av_q2d(fmt_ctx->streams[audioIndex]->time_base);
    printf("VideoTs: %d   AudioTs: %d\n", VideoTimeStamp, AudioTimeStamp);
    fflush(NULL);
    avcodec_flush_buffers(audioCodecCtx);
    avcodec_flush_buffers(videoCodecCtx);

    av_seek_frame(fmt_ctx, audioIndex, AudioTimeStamp, AVSEEK_FLAG_BYTE);
    av_seek_frame(fmt_ctx, videoIndex, VideoTimeStamp, AVSEEK_FLAG_BACKWARD);
    doSeek = true;
    this->state = PLAY;
    cv.notify_all();
    printf("Seek Ok\n");
    fflush(NULL);
    while (doSeek)
    {
        SDL_Delay(5);
    }
    printf("Exit Seek\n");
    fflush(NULL);
}

void Media::Play()
{
    printf("Do Play\n");
    fflush(NULL);
    if (this->state == NONE)
    {
        LastState = state;
        this->state = PLAY;
        cv.notify_all();
        SDL_PauseAudio(0);
    }
    else if (this->state == PLAY)
    {
        LastState = state;
        this->state = STOP;
        SDL_PauseAudio(1);
    }
    else if (this->state == STOP)
    {
        LastState = this->state;
        this->state = PLAY;
        cv.notify_all();
        SDL_PauseAudio(0);
    }
    else {
        return;
    }
}

//void Media::RenderPicture(AVFrame* frame)
//{
//    SDL_UpdateYUVTexture(texture, NULL,
//        frame->data[0], frame->linesize[0],
//        frame->data[1], frame->linesize[1],
//        frame->data[2], frame->linesize[2]);
//    // 纹理渲染的位置
//    rect.x = 0;
//    rect.y = 0;
//    // 纹理渲染的窗口大小
//    rect.w = frame->width;
//    rect.h = frame->height;
//    // 清空渲染器
//    SDL_RenderClear(renderer);
//    // 将纹理的数据拷贝给渲染器
//    SDL_RenderCopy(renderer, texture, NULL, &rect);
//    // 显示
//    SDL_RenderPresent(renderer);
//}

void Media::set_show_rect(int win_w, int win_h)
{
    std::lock_guard<std::mutex> lock(rect_mutex);
    if (win_w < 1280 || win_h < 720)
    {
        return;
    }
    float video_aspect = (float)dstVideoFrame->width / dstVideoFrame->height;  // 1280/720 ≈ 1.777（16:9）
    float win_aspect = (float)win_w / win_h;

    if (win_aspect >= video_aspect)
    {
        // 窗口更宽 → 以高度为基准缩放
        show_dest_h = win_h;
        show_dest_w = (int)(show_dest_h * video_aspect);
    }
    else
    {
        // 窗口更高 → 以宽度为基准缩放
        show_dest_w = win_w;
        show_dest_h = (int)(show_dest_w / video_aspect);
    }
    show_x = (win_w - show_dest_w) / 2;
    show_y = (win_h - show_dest_h) / 2;
    std::cout << "Show_Rect:" << show_x << ":" << show_y << ":" << show_dest_w << ":" << show_dest_h << std::endl;
}

void Media::RenderPicture(AVFrame* frame)
{
    std::lock_guard<std::mutex> lock(rect_mutex);
    std::cout << "Show_Rect:" << show_x << ":" << show_y << ":" << show_dest_w << ":" << show_dest_h << std::endl;
    SDL_UpdateYUVTexture(texture, NULL,
        frame->data[0], frame->linesize[0],
        frame->data[1], frame->linesize[1],
        frame->data[2], frame->linesize[2]);
    //// 纹理渲染的位置
    SDL_Rect rect;
    rect.x = show_x; 
    rect.y = show_y;
    rect.w = show_dest_w;//渲染的宽高，可缩放
    rect.h = show_dest_h;
    // 清空渲染器
    SDL_RenderClear(renderer);
    // 将纹理的数据拷贝给渲染器
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    // 显示
    SDL_RenderPresent(renderer);
    std::cout << "RenderPicture"  << std::endl;
}

void Media::destroySDLRresources()
{
    if (texture) SDL_DestroyTexture(texture);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);

    texture = nullptr;
    renderer = nullptr;
    window = nullptr;
}

void Media::createSDLRresources(void* win)
{
    window = SDL_CreateWindowFrom(win);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    texture = SDL_CreateTexture(renderer, pixformat,
        SDL_TEXTUREACCESS_STREAMING,
        1280, 720);
}

void Media::DelayVideo(int64_t& pts)
{
    LastVideoTs = CurrVideoTs;
    CurrVideoTs = pts * av_q2d(fmt_ctx->streams[videoIndex]->time_base);

    int diff = (CurrVideoTs - AudioTs) * 1000;
    int delay = std::abs((CurrVideoTs - LastVideoTs) * 1000);
    if (doSeek)
    {
        SDL_PauseAudio(0);
        std::unique_lock<std::mutex> seekLock(seekMtx);
        seekCv.wait(seekLock);
        delay = std::abs((CurrVideoTs - AudioTs) * 1000);
        if (Speed == 1.25)
        {
            delay = delay * 0.75;
        }
        SDL_Delay(delay);
        return;
    }
    if (std::abs(diff) < 10)
    {
        if (Speed == 1.25)
        {
            delay = delay * 0.75;
        }
        SDL_Delay(delay);
    }
    else if (diff < 0)
    {
        return;
    }
    else 
    {
        SDL_Delay(delay * 2);
    }
}

void Media::PlayFltp()
{
    int i, j, len, data_size, ch;
    //将这一帧数据写入到缓存中
    data_size = av_get_bytes_per_sample((AVSampleFormat)audioFrame->format);
    if (data_size < 0) {
        return;
    }
    if (doSeek)
    {
        len = 0;
        // frame->nb_samples 这一帧音频数据有nb_samples个样本  通常这个值为1024
        for (i = 0, j = 0; i < audioFrame->nb_samples; i++)
        {
            for (ch = 0; ch < audioFrame->channels; ch++)  // 交错的方式写入, 大部分float的格式输出
            {
                memcpy(s_audio_buf + data_size * j, audioFrame->data[ch] + data_size * i, data_size);
                len += data_size;
                j++;
            }
        }
    }
    else
    {
        len = 0;
        // frame->nb_samples 这一帧音频数据有nb_samples个样本  通常这个值为1024
        for (i = 0, j = 0; i < audioFrame->nb_samples; i++)
        {
            for (ch = 0; ch < audioFrame->channels; ch++)  // 交错的方式写入, 大部分float的格式输出
            {
                memcpy(s_audio_buf + data_size * j, audioFrame->data[ch] + data_size * i, data_size);
                len += data_size;
                j++;
            }
        }
    }
    s_audio_end = s_audio_buf + len;
    s_audio_pos = s_audio_buf;
}

void Media::PlayS16()
{
    int data_size;
    //将这一帧数据写入到缓存中
    data_size = av_get_bytes_per_sample((AVSampleFormat)audioFrame->format);
    if (data_size < 0) {
        return;
    }
    s_audio_end = s_audio_buf;

    memcpy(s_audio_buf, audioFrame->data[0], audioFrame->nb_samples * audioFrame->channels * data_size);

    s_audio_end = s_audio_buf + audioFrame->nb_samples * audioFrame->channels * data_size;
    s_audio_pos = s_audio_buf;
}

void Media::ClearQueue()
{
    // 清空 videoQueue
    while (!videoQueue.empty()) 
    {
        av_packet_free(&videoQueue.front());
        videoQueue.pop();
    }

    // 清空 audioQueue
    while (!audioQueue.empty()) 
    {
        av_packet_free(&audioQueue.front());
        audioQueue.pop();
    }
    LastAudioTs = 0.0f;
    LastVideoTs = 0.0f;
    AudioTs = 0.0f;
    CurrVideoTs = 0.0f;
}

void Media::UpdateSpeed(double val)
{
    Speed = val;
    if (this->state == NONE)
    {
        if (LastState == SET)
        {
            SDL_CloseAudio();
            InitAudio();
        }
    }
    else if (this->state == PLAY)
    {
        this->state = STOP;
        SDL_CloseAudio();
        while (!videoStop)
        {
            SDL_Delay(5);
        }
        InitAudio();
        this->state = PLAY;
        cv.notify_all();
        SDL_PauseAudio(0);
    }
    else if (this->state == FINISH)
    {
        SDL_CloseAudio();
        InitAudio();
    }
    else if (this->state == STOP)
    {
        SDL_CloseAudio();
        InitAudio();
    }
}

void Media::SavePic(QString name)
{
    int numBytes = 8 * 8 * 8 * videoCodecCtx->height * videoCodecCtx->width;
    uint8_t* buff = (uint8_t*)malloc(numBytes);
    uint8_t* dst_data[4] = { buff, NULL, NULL, NULL };
    int dst_linesize[4] = { videoCodecCtx->width * 3, 0, 0, 0 };
    sws_scale(saveCtx,
        (const uint8_t* const*)saveFrame->data,
        saveFrame->linesize,
        0,
        videoCodecCtx->height,
        dst_data,
        dst_linesize);
    printf("Change success\n");
    fflush(NULL);
    QImage image((const uint8_t*)dst_data[0], videoCodecCtx->width, videoCodecCtx->height,
        dst_linesize[0], QImage::Format_RGB888);
    name += "--";
    name += QString::number(Time);
    QString filePath("./Picture/");
    filePath += name;
    printf("Save: %s\n", filePath.toStdString().c_str());
    fflush(NULL);
    filePath += ".png";
    // 保存图像
    if (!image.save(filePath, "PNG")) {
        printf("Error\n");
        fflush(NULL);
    }
    image.~QImage();
    free(buff);
}
