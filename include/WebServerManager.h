#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ESPmDNS.h>

class WebServerManager {
public:
    static WebServerManager& getInstance();

    void begin();
    void stop();

private:
    WebServerManager();
    ~WebServerManager() = default;
    WebServerManager(const WebServerManager&) = delete;
    WebServerManager& operator=(const WebServerManager&) = delete;

    void setupRoutes();

    AsyncWebServer _server;
    bool _isStarted;
};
