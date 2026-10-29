#include "NtfyClient.h"
#include "NotificationManager.h"
#include "cards/CardRegistry.h"
#include "cards/NotificationCard.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

NtfyClient* NtfyClient::s_instance = nullptr;

NtfyClient& NtfyClient::getInstance() {
    if (!s_instance) {
        s_instance = new NtfyClient();
    }
    return *s_instance;
}

NtfyClient::NtfyClient()
    : _sinceParam("now"),
      _initialPollDone(false) {}

void NtfyClient::begin() {
    _sinceParam = "now";
    _initialPollDone = false;
}

bool NtfyClient::poll() {
    if (WiFi.status() != WL_CONNECTED) {
        return false;
    }

    NotificationCard* card = (NotificationCard*)CardRegistry::getInstance().getCard("notifications");
    if (!card || !card->isEnabled()) {
        return false;
    }

    String topic = card->getNtfyTopic();
    topic.trim();
    if (topic.length() == 0) {
        return false;
    }

    String server = card->getNtfyServer();
    server.trim();
    if (server.length() == 0) {
        server = "https://ntfy.sh";
    }
    if (server.endsWith("/")) {
        server.remove(server.length() - 1);
    }

    String url = server + "/" + topic + "/json?poll=1&since=" + _sinceParam;

    WiFiClientSecure client;
    client.setInsecure(); // Skip cert validation for embedded ESP32

    HTTPClient http;
    http.setUserAgent("esp32-clock/1.0");
    http.setTimeout(8000);

    if (!http.begin(client, url)) {
        Serial.printf("[NtfyClient] Failed to connect to %s\n", url.c_str());
        return false;
    }

    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("[NtfyClient] HTTP GET %s returned %d\n", url.c_str(), httpCode);
        http.end();
        return false;
    }

    WiFiClient* stream = http.getStreamPtr();
    if (!stream) {
        http.end();
        return false;
    }

    int messagesIngested = 0;
    while (http.connected() && (stream->available() || stream->connected())) {
        String line = stream->readStringUntil('\n');
        line.trim();
        if (line.length() == 0) continue;

        StaticJsonDocument<1024> doc;
        DeserializationError err = deserializeJson(doc, line);
        if (err) {
            continue;
        }

        const char* event = doc["event"] | "";
        if (strcmp(event, "message") == 0) {
            const char* id = doc["id"] | "";
            const char* title = doc["title"] | "";
            const char* message = doc["message"] | "";
            unsigned long msgTime = doc["time"] | 0;

            String displayTitle = String(title);
            if (displayTitle.length() == 0) {
                // If title empty, check for tags or fallback to topic
                if (doc["tags"].is<JsonArray>() && doc["tags"].size() > 0) {
                    displayTitle = doc["tags"][0].as<String>();
                } else {
                    displayTitle = "Alert";
                }
            }

            int timeoutSec = card->getTimeoutSec();
            NotificationManager::getInstance().addNotification(
                displayTitle,
                String(message),
                "ntfy",
                timeoutSec,
                String(id)
            );

            if (msgTime > 0) {
                _sinceParam = String(msgTime);
            } else if (strlen(id) > 0) {
                _sinceParam = String(id);
            }
            messagesIngested++;
        }
    }

    http.end();
    _initialPollDone = true;

    if (messagesIngested > 0) {
        Serial.printf("[NtfyClient] Ingested %d message(s) from topic '%s'\n", messagesIngested, topic.c_str());
    }
    return true;
}
