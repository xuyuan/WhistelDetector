// stand alone whistel detector for testing

#include <iostream>
#include <csignal>

extern int main_loop(const std::string& configFile, void (*whistleAction)(void));
extern void stopListening(int signal);

void whistleAction(void)
{
    std::cout << "  !!! Whistle heard !!!" << std::endl;
}

int main(int argc, char **argv)
{
    signal(SIGINT,  &stopListening);
    signal(SIGTERM, &stopListening);
    main_loop("WhistleConfig.ini", &whistleAction);
}
