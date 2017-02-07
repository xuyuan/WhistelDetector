/*!
 * \brief An interface to ALSA for recording sound data.
 * \author Thomas Hamboeck, Austrian Kangaroos 2014
 */

#ifndef __AK_ALSA_RECORDER__
#define __AK_ALSA_RECORDER__

#include <alsa/asoundlib.h>
#include <vector>
#include <functional>
#include <mutex>
#include <condition_variable>

class AlsaRecorder
{
public:
    /* handler: samples, count, channels */
    AlsaRecorder(std::function<void (const int16_t*, int, short)> handler);
    virtual ~AlsaRecorder();

    void main();
    void stop();
    void setListeningPaused(bool paused);

    bool isRunning() const;
    bool isPaused() const { return mPaused; }

protected:
    void initAlsa();
    void setVolume(const char *subdevice);
    void destroyAlsa();

    int xrunRecovery(snd_pcm_t *handle, int err);

    int16_t *audioBuffer;
    int bufferSize;

    snd_pcm_t *captureHandle;

    std::function<const void (int16_t*, int, short)> handler;
    volatile bool running;

    std::mutex mPausedMutex;
    std::condition_variable mPausedCondition;
    bool mPaused;
};

#endif

