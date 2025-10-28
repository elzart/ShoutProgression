#include "Config.h"
#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <SimpleIni.h>
#include <filesystem>

Config::Config() {

}

Config* Config::GetSingleton() {
    static Config instance;
    return &instance;
}

void Config::LoadFromINI() {
    CSimpleIniA ini;
    ini.SetUnicode();

    auto configPath = std::filesystem::path("Data") / "SKSE" / "Plugins" / "ShoutProgression.ini";

    if (ini.LoadFile(configPath.string().c_str()) < 0) {
        SKSE::log::info("INI file not found at {}, using default values", configPath.string());
        return;
    }

    SKSE::log::info("Loading configuration from {}", configPath.string());

    // Shout Progression Settings
    fDistanceMultiplier = static_cast<float>(ini.GetDoubleValue("ShoutProgression", "fDistanceMultiplier", fDistanceMultiplier));
    fMagnitudeMultiplier = static_cast<float>(ini.GetDoubleValue("ShoutProgression", "fMagnitudeMultiplier", fMagnitudeMultiplier));
    iMaxDragonSouls = static_cast<int>(ini.GetLongValue("ShoutProgression", "iMaxDragonSouls", iMaxDragonSouls));
    bCountSpentSouls = ini.GetBoolValue("ShoutProgression", "bCountSpentSouls", bCountSpentSouls);

    // Minimum Multiplier Settings
    fMinDistanceMultiplier = static_cast<float>(ini.GetDoubleValue("ShoutProgression", "fMinDistanceMultiplier", fMinDistanceMultiplier));
    fMinMagnitudeMultiplier = static_cast<float>(ini.GetDoubleValue("ShoutProgression", "fMinMagnitudeMultiplier", fMinMagnitudeMultiplier));

    // Debug Settings
    bEnableDebugLogging = ini.GetBoolValue("General", "bEnableDebugLogging", bEnableDebugLogging);

    SKSE::log::info("Configuration loaded:");
    SKSE::log::info("  fDistanceMultiplier: {}", fDistanceMultiplier);
    SKSE::log::info("  fMagnitudeMultiplier: {}", fMagnitudeMultiplier);
    SKSE::log::info("  iMaxDragonSouls: {}", iMaxDragonSouls);
    SKSE::log::info("  bCountSpentSouls: {}", bCountSpentSouls);
    SKSE::log::info("  fMinDistanceMultiplier: {}", fMinDistanceMultiplier);
    SKSE::log::info("  fMinMagnitudeMultiplier: {}", fMinMagnitudeMultiplier);
    SKSE::log::info("  bEnableDebugLogging: {}", bEnableDebugLogging);
}

