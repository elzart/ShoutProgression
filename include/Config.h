#pragma once

#include <string>

// ============================================
// Shout Progression Configuration
// ============================================
struct Config {
    // Shout scaling multipliers
    float fDistanceMultiplier = 0.04f;   // Distance scaling per dragon soul
    float fMagnitudeMultiplier = 0.03f;  // Magnitude scaling per dragon soul
    int iMaxDragonSouls = 50;            // Maximum dragon souls for scaling cap
    bool bCountSpentSouls = true;        // Count souls spent on unlocking shout words

    // Debug settings
    bool bEnableDebugLogging = false;

    Config();

    static Config* GetSingleton();
    void LoadFromINI();
};

