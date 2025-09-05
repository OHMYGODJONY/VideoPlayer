#pragma once
#include <SDL2/SDL.h>
#undef main
#include <thread>
#include <mutex>
#include <condition_variable>
#include <QString>
#include <queue>
#include <functional>
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}
#include "public.h"

#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "SDL2.lib")

enum State
{
    NONE = 0,
    PLAY,
    NETSTREAM,//这个状态表示播放的是网络流数据
    STOP,
    SET,
    FINISH
};

enum PicLevel {
    Low,
    High,
    Default
};

class Media
{
    friend void AudioCallback(void* data, uint8_t* stream, int len);

public:
    Media(void* win);
    ~Media();
    void destroySDLRresources();
    void createSDLRresources(void* win);

    void SetFileName(QString file);

    State GetState() {
        return state;
    }
    State LastState = NONE;
    void Seek(int pos);

    void Play();

    void setUpdateCallback(const std::function<void()>& callback) 
    {
        updateCallback = callback;
    }

    void setStopCallback(const std::function<void()>& callback)
    {
        stopCallback = callback;
    }

    void set_show_rect(int win_w, int win_h);

    int getDuration() {
        return totalTime;
    }

    QString getFileName()
    {
        return fileName;
    }

    void setVolume(int val) {
        volume = val;
        return;
    }

    double GetTime() {
        return AudioTs;
    }

    void UpdateSpeed(double val);

    void SavePic(QString name);
    //这个是当前用户选择的哪种分辨率
    PicLevel level = Default;
    //默认是两种都支持
    PicLevel suppLevel = High;

    void Delay(int t)
    {
        dt = t;
    }
private:
    int dt = 0;
    void DelayVideo(int64_t& pts);

    void RenderPicture(AVFrame* frame);

    void InitAudio();

    void decodeThread();
    void videoThread();

    void InitDecode();
    void deInitDecode();

    void PlayFltp();
    void PlayS16();

    void ClearQueue();

    

    double Time = 1.0f;

    SDL_Event event;                            // 事件
    SDL_Rect rect;                              // 矩形
    SDL_Window* window = NULL;                  // 窗口
    SDL_Renderer* renderer = NULL;              // 渲染
    SDL_Texture* texture = NULL;                // 纹理
    uint32_t pixformat = SDL_PIXELFORMAT_IYUV;  // YUV420P，即是SDL_PIXELFORMAT_IYUV

    SDL_AudioSpec spec;
    std::mutex mtx;
    std::condition_variable cv;

    std::mutex seekMtx;
    std::condition_variable seekCv;

    std::mutex rect_mutex;

    State state = NONE;

    QString fileName = "";

    AVFormatContext* fmt_ctx = NULL;
    AVCodecContext* audioCodecCtx = NULL;
    AVCodecContext* videoCodecCtx = NULL;
    // 其它分辨率转1280*720
    SwsContext* swsCtx = NULL;
    //用于做yuv转rgb
    SwsContext* saveCtx = NULL;
    // 其它分辨率转640*480
    SwsContext* lowCtx = NULL;
    // 640*480转1280*720
    SwsContext* highCtx = NULL;

    std::thread decode_thread;
    std::thread video_thread;
    std::atomic<bool> is_running{ false };

    int videoIndex = -1;
    int audioIndex = -1;

    std::queue<AVPacket*> audioQueue;
    std::queue<AVPacket*> videoQueue;

    // 更新进度条和当前的播放时间
    std::function<void()> updateCallback;
    std::function<void()> stopCallback;

    AVFrame* saveFrame = NULL;
    AVFrame* srcVideoFrame = NULL;
    // 1280*720分辨率的图像
    AVFrame* dstVideoFrame = NULL;
    // 640*480分辨率的图像
    AVFrame* lowVideoFrame = NULL;

    AVFrame* audioFrame = NULL;

    int show_x = 0;
    int show_y = 0;
    int show_dest_w = 1280; 
    int show_dest_h = 720;

    bool videoStop = false;
    bool decodeStop = false;

    bool videoFinish = false;
    bool decodeFinish = false;

    // 目前读取的位置
    Uint8* s_audio_pos = NULL;
    // 缓存结束位置
    Uint8* s_audio_end = NULL;
    Uint8* s_audio_buf = NULL;

    int RemainLen = 0;

    int volume = SDL_MIX_MAXVOLUME / 8;

    double LastVideoTs = 0.0f;
    double CurrVideoTs = 0.0f;
    double AudioTs = 0.0f;
    double LastAudioTs = 0.0f;

    int totalTime = 0;

    double Speed = 1.0f;
};

