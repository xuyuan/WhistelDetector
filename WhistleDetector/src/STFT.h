/*!
 * \brief Short Time Fourier Transform.
 * \author Thomas Hamboeck, Austrian Kangaroos 2014
 */

#ifndef __AK_STFT__
#define __AK_STFT__

#include <fftw3.h>
#include <complex>
#include <functional>

class STFT
{
public:
    STFT(const int channelOffset, const int windowTime, const int windowTimeStep, const int windowFrequency,
         std::function<void (const float *spectrum, int length)> handleSpectrum);
    virtual ~STFT();

    void newData(const int16_t *data, int length, short channels);

protected:
    void intToFloat(const int16_t &in, float &out);

    const int offset;
    const int windowTime, windowTimeStep, windowFrequency, windowFrequencyHalf;
    std::function<void (const float *spectrum, int length)> handleSpectrum;

    int nOverflow;
    int16_t *overflownData;
    float *input;
    fftwf_complex *output;
    float *outputMag;

    fftwf_plan plan;
};

#endif
