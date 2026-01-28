#pragma once

#include <string>

struct Config {
    float fDistanceMultiplier = 0.04f;
    float fMagnitudeMultiplier = 0.03f;
    float fCooldownReduction = 0.01f;
    int iMaxDragonSouls = 50;
    bool bCountSpentSouls = true;

    float fMinDistanceMultiplier = 1.0f;
    float fMinMagnitudeMultiplier = 1.0f;
    float fMinCooldownMultiplier = 0.2f;

    bool bEnableDebugLogging = true;

    Config();

    static Config* GetSingleton();
    void LoadFromINI();
};

