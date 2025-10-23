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
    fMinMultiplier = static_cast<float>(ini.GetDoubleValue("ShoutProgression", "fMinMultiplier", fMinMultiplier));
    fMaxMultiplier = static_cast<float>(ini.GetDoubleValue("ShoutProgression", "fMaxMultiplier", fMaxMultiplier));
    iMaxLevel = static_cast<int>(ini.GetLongValue("ShoutProgression", "iMaxLevel", iMaxLevel));

    // Debug Settings
    bEnableDebugLogging = ini.GetBoolValue("General", "bEnableDebugLogging", bEnableDebugLogging);

    SKSE::log::info("Configuration loaded:");
    SKSE::log::info("  fMinMultiplier: {}", fMinMultiplier);
    SKSE::log::info("  fMaxMultiplier: {}", fMaxMultiplier);
    SKSE::log::info("  iMaxLevel: {}", iMaxLevel);
    SKSE::log::info("  bEnableDebugLogging: {}", bEnableDebugLogging);
}

