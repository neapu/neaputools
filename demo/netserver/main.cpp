#include "NEString.h"
#include "NELogger.h"
#include "Server.h"
#include "NEUtil.h"

using namespace neapu;
int main(int argc, char** argv)
{
    Arguments arg(argc, argv);
    int port = arg.GetValue("port", "7669").ToInt();
    Server app;
    int rst = app.start(port, 1);
    Logger(LM_INFO)<<"Server stop:"<<rst;
    return rst;
}