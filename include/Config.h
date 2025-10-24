#pragma once

#include <string>

// ============================================
// Shout Progression Configuration
// ============================================
struct Config {
    // Shout scaling multipliers
    float fDistanceMultiplier = 0.15f;   // Distance scaling per dragon soul
    float fMagnitudeMultiplier = 0.10f;  // Magnitude scaling per dragon soul
    int iMaxDragonSouls = 25;            // Maximum dragon souls for scaling cap

    // Debug settings
    bool bEnableDebugLogging = false;

    Config();

    static Config* GetSingleton();
    void LoadFromINI();
};

