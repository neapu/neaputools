#include "NEFile.h"
#include "NEJsonObject.h"
#include "NEJsonValue.h"
#include "NEString.h"
#include "neapu-config.h"
#include "NELogger.h"
#include "NEJsonReader.h"

using namespace neapu;

int main()
{
    File file(String(NETOOLS_SOURCE_DIR)+"/demo/json/test.json");
    if(!file.Open(File::OpenMode::ReadOnly)){
        LOG_ERROR << "file open error";
        return -1;
    }
    String data = file.Read();

    JsonValue root;
    JsonReader reader;
    int ret = reader.Parse(data, root);
    if(!ret){
        LOG_ERROR << "json parse failed";
        return -1;
    }

    if(!root.IsObject()){
        LOG_ERROR << "root is not object";
        return -1;
    }

    LOG_INFO << root;
    return 0;
}