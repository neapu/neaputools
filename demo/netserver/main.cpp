#include "NTString.h"
#include "NTLogger.h"
#include "Server.h"

using namespace neapu;
int main()
{
    Server app;
    int rst = app.start(7669, 1);
    Logger(LM_INFO)<<"Server stop:"<<rst;
    return rst;
}