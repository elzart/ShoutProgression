#pragma once

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

// ============================================
// Shout Event Handler with Serialization
// ============================================
class ShoutHandler : public RE::BSTEventSink<SKSE::ActionEvent> {
public:
    static ShoutHandler* GetSingleton();

    // Event handler
    RE::BSEventNotifyControl ProcessEvent(const SKSE::ActionEvent* a_event, RE::BSTEventSource<SKSE::ActionEvent>*) override;

    // Helper methods
    float CalculateDistanceMultiplier(int dragonSouls);
    float CalculateMagnitudeMultiplier(int dragonSouls);
    int GetSpentSoulCount(RE::PlayerCharacter* player);
    
    // Serialization callbacks
    static void OnGameSaved(SKSE::SerializationInterface* a_intfc);
    static void OnGameLoaded(SKSE::SerializationInterface* a_intfc);
    static void OnRevert(SKSE::SerializationInterface* a_intfc);

private:
    ShoutHandler() = default;
    ShoutHandler(const ShoutHandler&) = delete;
    ShoutHandler(ShoutHandler&&) = delete;
    ~ShoutHandler() override = default;

    ShoutHandler& operator=(const ShoutHandler&) = delete;
    ShoutHandler& operator=(ShoutHandler&&) = delete;

    // Soul tracking methods
    void UpdateSoulTracking(RE::PlayerCharacter* player);
    int CountUnlockedShoutWords(RE::PlayerCharacter* player); // Used only for initial calibration

    // Serialization data
    std::uint32_t m_totalSoulsEverEarned = 0;
    std::uint32_t m_lastKnownSoulCount = 0;
    bool m_isInitialized = false;
};


