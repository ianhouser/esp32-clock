#include "WebServerManager.h"
#include <WiFi.h>
#include <LittleFS.h>
#include "ConfigManager.h"
#include "cards/CardRegistry.h"

WebServerManager& WebServerManager::getInstance() {
    static WebServerManager instance;
    return instance;
}

WebServerManager::WebServerManager()
    : _server(80), _isStarted(false) {}

void WebServerManager::begin() {
    if (_isStarted) return;

    const auto& sysConfig = ConfigManager::getInstance().getSystemConfig();
    const char* hostname = sysConfig.mdnsHostname.length() > 0 ? sysConfig.mdnsHostname.c_str() : "esp32-clock";

    if (MDNS.begin(hostname)) {
        MDNS.addService("http", "tcp", 80);
        Serial.printf("[WebServerManager] mDNS responder started: http://%s.local\n", hostname);
    } else {
        Serial.println("[WebServerManager] Error setting up mDNS responder!");
    }

    setupRoutes();
    _server.begin();
    _isStarted = true;
    Serial.println("[WebServerManager] HTTP server started on port 80.");
}

void WebServerManager::stop() {
    if (!_isStarted) return;
    _server.end();
    _isStarted = false;
    Serial.println("[WebServerManager] HTTP server stopped.");
}

void WebServerManager::setupRoutes() {
    // 1. Static Web Control Panel Files
    _server.serveStatic("/", LittleFS, "/")
           .setDefaultFile("index.html")
           .setCacheControl("max-age=300");

    // 2. GET /api/config
    _server.on("/api/config", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncResponseStream *response = request->beginResponseStream("application/json");
        response->addHeader("Access-Control-Allow-Origin", "*");

        JsonDocument doc;
        ConfigManager::getInstance().serializeConfig(doc);
        serializeJson(doc, *response);
        request->send(response);
    });

    // 3. POST /api/config
    AsyncCallbackJsonWebHandler *configPostHandler = new AsyncCallbackJsonWebHandler("/api/config", 
        [](AsyncWebServerRequest *request, JsonVariant &json) {
            if (!json.is<JsonObject>()) {
                request->send(400, "application/json", "{\"error\":\"Invalid JSON payload\"}");
                return;
            }

            JsonDocument doc;
            doc.set(json);

            bool success = ConfigManager::getInstance().deserializeConfig(doc);
            if (success) {
                ConfigManager::getInstance().save();
                AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"status\":\"ok\"}");
                response->addHeader("Access-Control-Allow-Origin", "*");
                request->send(response);
            } else {
                request->send(500, "application/json", "{\"error\":\"Failed to apply config\"}");
            }
        });
    _server.addHandler(configPostHandler);

    // 4. GET /api/status
    _server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncResponseStream *response = request->beginResponseStream("application/json");
        response->addHeader("Access-Control-Allow-Origin", "*");

        JsonDocument doc;
        doc["uptime_ms"] = millis();
        doc["free_heap"] = ESP.getFreeHeap();
        doc["heap_size"] = ESP.getHeapSize();
        
        JsonObject wifiObj = doc["wifi"].to<JsonObject>();
        wifiObj["connected"] = (WiFi.status() == WL_CONNECTED);
        wifiObj["ip"] = WiFi.localIP().toString();
        wifiObj["rssi"] = WiFi.RSSI();
        wifiObj["ssid"] = WiFi.SSID();

        serializeJson(doc, *response);
        request->send(response);
    });

    // 5. POST /api/restart
    _server.on("/api/restart", HTTP_POST, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"status\":\"restarting\"}");
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);

        // Schedule restart after responding
        xTaskCreate([](void*) {
            vTaskDelay(pdMS_TO_TICKS(500));
            ESP.restart();
        }, "restart_task", 2048, NULL, 1, NULL);
    });

    // 6. Handle OPTIONS for CORS
    _server.onNotFound([](AsyncWebServerRequest *request) {
        if (request->method() == HTTP_OPTIONS) {
            AsyncWebServerResponse *response = request->beginResponse(204);
            response->addHeader("Access-Control-Allow-Origin", "*");
            response->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
            response->addHeader("Access-Control-Allow-Headers", "Content-Type");
            request->send(response);
        } else {
            request->send(404, "text/plain", "Not found");
        }
    });
}
