#pragma once

#include <Arduino.h>

class NtfyClient {
public:
    static NtfyClient& getInstance();

    void begin();
    bool poll();

private:
    NtfyClient();
    static NtfyClient* s_instance;

    String _sinceParam;
    bool _initialPollDone;
};
