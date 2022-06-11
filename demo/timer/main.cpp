#include <NELogger.h>
#include <NEEventBase.h>
#include <signal.h>
using namespace neapu;
int main()
{
    EventBase eb;
    eb.Init();

    eb.AddTimer(1000, true, [](EventHandle _handle) {
        Logger(LM_INFO) << "On Timer:" << String::ToString(reinterpret_cast<uint64_t>(_handle));
        });
    eb.AddSignal(SIGINT, false, [&](int _signal, EventHandle _handle) {
        Logger(LM_INFO) << "On Signal:" << _signal << " trigger. " << String::ToString(reinterpret_cast<uint64_t>(_handle));
        eb.EventLoopBreak();
        });

    return eb.EventLoop();
}