/*!
 * \brief Demonstration program for Whistle Detection.
 * \author Thomas Hamboeck, Austrian Kangaroos 2014
 */


#include <iostream>
#include <functional>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include "SoundConfig.h"
#include "ALSARecorder.h"
#include "STFT.h"

struct ProcessingRecord {
    float fWhistleBegin, fWhistleEnd;
    int nWhistleBegin, nWhistleEnd;
    int fSampleRate;
    int nWindowSize, nWindowSizePadded;
    int nWindowSkipping;
    float vDeviationMultiplier;
    float vWhistleThreshold;
    unsigned nWhistleMissFrames, nWhistleOkayFrames;
};

int executeAction(const ProcessingRecord &config, void (*whistleAction)(void));
int runFrequencyExtraction(ProcessingRecord &config, void (*whistleAction)(void));
void stopListening(int signal);

static AlsaRecorder *reader = NULL;

int main_loop(const std::string& configFile, void (*whistleAction)(void)) {

    ProcessingRecord config;
    boost::property_tree::ptree iniConfig;
    boost::property_tree::ini_parser::read_ini(configFile, iniConfig);

    config.fWhistleBegin            = iniConfig.get<float>("Frequencies.WhistleBegin");
    config.fWhistleEnd              = iniConfig.get<float>("Frequencies.WhistleEnd");
    config.fSampleRate              = iniConfig.get<int>("Frequencies.SampleRate");

    config.nWindowSize              = iniConfig.get<int>("Time.WindowSize");
    config.nWindowSizePadded        = iniConfig.get<int>("Time.WindowSizePadded");
    config.nWindowSkipping          = iniConfig.get<int>("Time.WindowSkipping");

    config.vWhistleThreshold        = iniConfig.get<float>("Whistle.Threshold");
    config.nWhistleOkayFrames       = iniConfig.get<unsigned>("Whistle.FrameOkays");
    config.nWhistleMissFrames       = iniConfig.get<unsigned>("Whistle.FrameMisses");

    std::cout << "---------------------------------------------------" << std::endl
              << "--- Whistle Detection                           ---" << std::endl
              << "--- Thomas Hamboeck <th@complang.tuwien.ac.at>  ---" << std::endl
              << "--- Austrian Kangaroos, 2014                    ---" << std::endl
              << "---------------------------------------------------" << std::endl;

    int result;
    result = runFrequencyExtraction(config, whistleAction);
    if(result == 0) {
        std::cout << "--------------------- Success ---------------------" << std::endl;
    } else {
        std::cout << "xxxxxxxxxxxxxxxxxxxxxxx Fail xxxxxxxxxxxxxxxxxxxxxx" << std::endl;
    }
    return result;
}

int runFrequencyExtraction(ProcessingRecord &config, void (*whistleAction)(void))
{
    /* load window times */
    config.nWhistleBegin = (config.fWhistleBegin * config.nWindowSizePadded) / config.fSampleRate;
    config.nWhistleEnd  = (config.fWhistleEnd  * config.nWindowSizePadded)   / config.fSampleRate;

    /* back calculation for displaying purposes */
    const float fWhistleBegin = (config.nWhistleBegin * static_cast<float>(config.fSampleRate)) / config.nWindowSizePadded;
    const float fWhistleEnd   = (config.nWhistleEnd *   static_cast<float>(config.fSampleRate)) / config.nWindowSizePadded;

    std::cout   << "---------------------------------------------------"   << std::endl
                << "Window:" << std::endl
                << "  Real Window:      " << config.nWindowSize            << " bins" << std::endl
                << "  Padded Window:    " << config.nWindowSizePadded      << " bins" << std::endl
                << "  Window Skip:      " << config.nWindowSkipping        << " samples" << std::endl
                << "---------------------------------------------------"   << std::endl
                << "  Whistle Begin:    " << fWhistleBegin                 << " Hz" << std::endl
                << "  Whistle End:      " << fWhistleEnd                   << " Hz" << std::endl;
    std::cout   << "---------------------------------------------------"   << std::endl;


    if(fWhistleBegin < 0) {
        std::cerr << "Whistle begin is below zero!" << std::endl;
        return -1;
    }
    if(fWhistleEnd   < 0) {
        std::cerr << "Whistle end is below zero!" << std::endl;
        return -1;
    }
    if(fWhistleBegin > (config.fSampleRate / 2)) {
        std::cerr << "Whistle begin is above Nyquist frequency!" << std::endl;
        return -1;
    }
    if(fWhistleEnd   > (config.fSampleRate / 2)) {
        std::cerr << "Whistle end is above Nyquist frequency!" << std::endl;
        return -1;
    }
    if(fWhistleBegin > fWhistleEnd) {
        std::cerr << "Whistle begin is above Whistle end!" << std::endl;
        return -1;
    }

    executeAction(config, whistleAction);

    return 0;
}

void stopListening(int signal)
{
    if(reader) {
        if(reader->isRunning()) {
            reader->stop();
        }
    }
}

int executeAction(const ProcessingRecord &config, void (*whistleAction)(void))
{
    auto calcMeanDeviation = [&] (const float *data, int length, float &mean, float &dev) {
        mean = dev = 0;
        for(int i = 0; i < length; ++i) {
            mean    += data[i];
            dev     += data[i] * data[i];
        }

        dev = std::sqrt(length * dev - mean * mean) / length;
        mean /= length;
    };

    /* start fft stuff */
    auto handleSpectrum = [&] (const float *spectrum, int length) {
        static unsigned whistleCounter(0), whistleMissCounter(0), whistleDone(false);

        float mean, dev;
        calcMeanDeviation(spectrum, length, mean, dev);

        bool found;
        const float whistleThresh = mean + config.vWhistleThreshold * dev;
        found = false;

        int i;
        for(i = config.nWhistleBegin; i < config.nWhistleEnd; ++i) {
            if(spectrum[i] > whistleThresh) {
                found = true;
                break;
            }
        }

        if(whistleDone) {
            if(!found) {
                ++whistleMissCounter;
                if(whistleMissCounter > config.nWhistleMissFrames) {
                    whistleCounter = 0;
                    whistleMissCounter = 0;
                    whistleDone = false;
                }
            }
        }
        else
        {
            if(found) {
                ++whistleCounter;
                whistleMissCounter = 0;
            } else if(whistleCounter > 0) {
                ++whistleMissCounter;
                if(whistleMissCounter > config.nWhistleMissFrames) {
                    whistleCounter = 0;
                    whistleMissCounter = 0;
                    whistleDone = false;
                }
            }
            if(whistleCounter >= config.nWhistleOkayFrames) {
                whistleAction();
                whistleCounter = 0;
                whistleMissCounter = 0;
                whistleDone = true;
            }
        }
    };

    STFT stft(0, config.nWindowSize, config.nWindowSkipping, config.nWindowSizePadded, handleSpectrum);
    reader = new AlsaRecorder(std::bind(&STFT::newData, &stft, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    std::cout << "Listening ..." << std::endl;
    reader->main();
    std::cout << "... stopped listening." << std::endl;

    return 0;
}
