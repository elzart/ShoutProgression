#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <spdlog/sinks/basic_file_sink.h>

#include "Config.h"
#include "ShoutHandler.h"

using namespace std::literals;

// ============================================
// Plugin Declaration
// ============================================
SKSEPluginInfo(
    .Version = { 1, 0, 0, 0 },
    .Name = "ShoutProgression"sv,
    .Author = "SKSE Plugin Developer"sv,
    .SupportEmail = ""sv,
    .StructCompatibility = SKSE::StructCompatibility::Independent,
    .RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary
)

// ============================================
// Setup Logging
// ============================================
void SetupLog() {
    auto logsFolder = SKSE::log::log_directory();
    if (!logsFolder) SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");
    auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
    auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
    auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
    auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));
    spdlog::set_default_logger(std::move(loggerPtr));
    spdlog::set_level(spdlog::level::info);
    spdlog::flush_on(spdlog::level::info);
}

// ============================================
// SKSE Message Handler
// ============================================
void OnSKSEMessage(SKSE::MessagingInterface::Message* message) {
    if (message->type == SKSE::MessagingInterface::kDataLoaded) {
        SKSE::log::info("Data loaded, registering event handlers...");
        
        // Load configuration
        auto* config = Config::GetSingleton();
        config->LoadFromINI();

        // Set log level based on config
        if (config->bEnableDebugLogging) {
            spdlog::set_level(spdlog::level::debug);
            SKSE::log::info("Debug logging enabled");
        }

        // Register shout event handler
        auto* shoutEventSource = RE::ShoutAttack::GetEventSource();
        if (shoutEventSource) {
            shoutEventSource->AddEventSink(ShoutHandler::GetSingleton());
            SKSE::log::info("Shout event handler registered successfully");
        } else {
            SKSE::log::error("Failed to get ShoutAttack event source");
        }

        SKSE::log::info("Shout Progression plugin initialized successfully");
    }
}


// ============================================
// Plugin Entry Point
// ============================================
SKSEPluginLoad(const SKSE::LoadInterface* skse) {
    SetupLog();

    auto* plugin = SKSE::PluginDeclaration::GetSingleton();
    auto version = plugin->GetVersion();

    SKSE::log::info("{} {} is loading...", plugin->GetName(), version);
    SKSE::Init(skse);

    // Register for SKSE messages
    auto* messaging = SKSE::GetMessagingInterface();
    if (messaging) {
        messaging->RegisterListener(OnSKSEMessage);
        SKSE::log::info("Registered for SKSE messages");
    } else {
        SKSE::log::error("Failed to get messaging interface");
        return false;
    }

    SKSE::log::info("{} has finished loading.", plugin->GetName());
    return true;
}
