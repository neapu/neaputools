#include <NELogger.h>
#include <NEEventBase2.h>
#include <signal.h>
using namespace neapu;
int main()
{
    EventBase2 eb;
    eb.Init();

    int id = eb.AddTimer(1000, true, [](int id) {
        Logger(LM_INFO) << "On Timer:" << id;
    });
    eb.AddSignal(SIGINT, [&](int _signal) {
        Logger(LM_INFO) << "On Signal:" << _signal << " trigger. " << _signal;
        eb.Stop();
    });

    return eb.LoopStart();
}