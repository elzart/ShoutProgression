#pragma once

#include <string>

// ============================================
// Shout Progression Configuration
// ============================================
struct Config {
    // Shout distance scaling
    float fMinMultiplier = 0.5f;   // Multiplier at level 1
    float fMaxMultiplier = 2.0f;   // Multiplier at max level
    int iMaxLevel = 81;            // Maximum level for scaling

    // Debug settings
    bool bEnableDebugLogging = false;

    Config();

    static Config* GetSingleton();
    void LoadFromINI();
};

