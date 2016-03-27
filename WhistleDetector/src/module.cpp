// Whistel detection module for NaoQi

#include <boost/shared_ptr.hpp>

#include <alcommon/albroker.h>
#include <alcommon/albrokermanager.h>
#include <alcommon/altoolsmain.h>
#include <alproxies/almemoryproxy.h>

#define ALCALL

extern int main_loop(const std::string& configFile, void (*whistleAction)(void));
extern void stopListening(int signal);


class WhistelDetector: public AL::ALModule {
public:
    WhistelDetector(boost::shared_ptr<AL::ALBroker> broker, const std::string &name): AL::ALModule(broker, name), mWhistelCount(0) {
        setModuleDescription("Whistle Detector");
    }

    virtual ~WhistelDetector() {
        mSelf = NULL;
    }

    virtual void init() {
        if (mSelf == NULL) {
            mSelf = this;
        }
        mMemoryProxy.declareEvent("WhistleHeard");
        mThread = boost::thread(&WhistelDetector::main, this);
        pthread_setname_np(mThread.native_handle(), "WhistleDetector");
    }

    virtual void exit() {
        stopListening(0);
        mThread.join();
    }

private:
    int main() {
        return main_loop("/home/nao/WhistleConfig.ini", &WhistelDetector::whistleActionWrapper);
    }

    static void whistleActionWrapper() {
        mSelf->whistleAction();
    }

    void whistleAction() {
        mWhistelCount++;
        mMemoryProxy.raiseEvent("WhistleHeard", mWhistelCount);
    }

private:
    static WhistelDetector* mSelf;
    int mWhistelCount;
    boost::thread mThread;
    AL::ALMemoryProxy mMemoryProxy;
};

WhistelDetector* WhistelDetector::mSelf = NULL;

extern "C"
{
    ALCALL int _createModule(boost::shared_ptr<AL::ALBroker> pBroker) {
        AL::ALBrokerManager::setInstance(pBroker->fBrokerManager.lock());
        AL::ALBrokerManager::getInstance()->addBroker(pBroker);

        AL::ALModule::createModule<WhistelDetector>(pBroker,"WhistleDetector" );

        return 0;
    }

    ALCALL int _closeModule() {
        return 0;
    }

}

#ifdef MODULE_IS_REMOTE

int main(int argc, char *argv[]) {
    // pointer on createModule
    TMainType sig;
    sig = &_createModule;

    // call main
    ALTools::mainFunction("WhistelDetector", argc, argv, sig);
}
#endif

