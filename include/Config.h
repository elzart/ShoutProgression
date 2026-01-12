#pragma once

#include <string>

struct Config {
    float fDistanceMultiplier = 0.04f;
    float fMagnitudeMultiplier = 0.03f;
    int iMaxDragonSouls = 50;
    bool bCountSpentSouls = true;

    float fMinDistanceMultiplier = 1.0f;
    float fMinMagnitudeMultiplier = 1.0f;

    bool bEnableDebugLogging = false;

    Config();

    static Config* GetSingleton();
    void LoadFromINI();
};

