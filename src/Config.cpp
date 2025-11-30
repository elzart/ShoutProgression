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

    // Try loading from MCM settings (user settings) first
    auto mcmUserPath = std::filesystem::path("Data") / "MCM" / "Settings" / "ShoutProgression - MCM.ini";
    auto mcmDefaultPath = std::filesystem::path("Data") / "MCM" / "Config" / "ShoutProgression - MCM" / "settings.ini";
    auto legacyPath = std::filesystem::path("Data") / "SKSE" / "Plugins" / "ShoutProgression.ini";

    std::filesystem::path configPath;

    // Priority: MCM user settings > MCM defaults > Legacy SKSE plugin INI
    if (std::filesystem::exists(mcmUserPath)) {
        configPath = mcmUserPath;
        SKSE::log::info("Loading configuration from MCM user settings: {}", configPath.string());
    } else if (std::filesystem::exists(mcmDefaultPath)) {
        configPath = mcmDefaultPath;
        SKSE::log::info("Loading configuration from MCM defaults: {}", configPath.string());
    } else if (std::filesystem::exists(legacyPath)) {
        configPath = legacyPath;
        SKSE::log::info("Loading configuration from legacy INI: {}", configPath.string());
    } else {
        SKSE::log::info("No configuration file found, using default values");
        return;
    }

    if (ini.LoadFile(configPath.string().c_str()) < 0) {
        SKSE::log::warn("Failed to load configuration from {}, using default values", configPath.string());
        return;
    }

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

