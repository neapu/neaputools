#include <iostream>
#include <NEHttpServer.h>
#include <NEUtil.h>
#include <neapu-config.h>
#include <NEHttpHandle.h>
#include <signal.h>
#include <base/NESettings.h>
using namespace neapu;

inline constexpr unsigned long long operator"" _kb(unsigned long long i)
{
    return i * 1024;
}

int main()
{
    Settings set;
    IPAddress::Type type = IPAddress::Type::IPv4;
    String address;
    int port = 9884;
    if (set.Init(String(NETOOLS_SOURCE_DIR) + "/demo/configs/http.conf") == 0) {
        if (set.GetValue("type", "IPv4") == "IPv6") {
            type = IPAddress::Type::IPv6;
        }

        address = set.GetValue("address", String());
        port = (int)set.GetValue("port", "9884").ToInt();
    }

    int rc = 0;
    HttpServer server;
    server.HttpLog();
    rc = server.Init(1, IPAddress::MakeAddress(type, address, port));
    if (rc < 0) {
        Logger(LM_ERROR) << "Http Server Init Error:" << rc << "|" << server.GetError();
        return rc;
    }
    server.StaticPath("/", String(NETOOLS_SOURCE_DIR) + "/demo/http/files", 10_kb);
    server.Get("/api/get_data", [](std::shared_ptr<HttpHandle> _handle) {
        _handle->SetContentType(HttpHandle::ContentType::Json);
        _handle->SendResponse("{\"data\":\"test data\"}");
    });
    server.Get("/api/set_cookie", [](HttpHandlePtr _handle) {
        _handle->SetContentType(HttpHandle::ContentType::Json);
        _handle->AddCookie({.key = "test",
                            .value = "123456",
                            .httpOnly = true});
        _handle->SendResponse("{\"data\":\"ok\"}");
    });
    server.Get("/api/get_cookie", [](HttpHandlePtr _handle) {
        auto test = _handle->GetCookie("test");
        _handle->SetContentType(HttpHandle::ContentType::Json);
        _handle->SendResponse(String("{\"data\":\"%1\"}").Argument(test));
    });
    server.AddSignal(SIGINT, false, [&](int, EventHandle) {
        Logger(LM_INFO) << "SIGINT trigger";
        server.Stop();
    });
    rc = server.Listen();
    if (rc < 0) {
        Logger(LM_ERROR) << "Http Server Listen Error:" << rc << "|" << server.GetError();
        return rc;
    }
    Logger(LM_INFO) << "Http Server Listened:" << server.GetAddress();
    rc = server.Run();
    if (rc < 0) {
        Logger(LM_ERROR) << "Event Loop Error:" << rc;
    }
    return rc;
}