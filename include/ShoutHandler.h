#pragma once

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

class ShoutHandler : public RE::BSTEventSink<SKSE::ActionEvent> {
public:
    static ShoutHandler* GetSingleton();

    RE::BSEventNotifyControl ProcessEvent(const SKSE::ActionEvent* a_event, RE::BSTEventSource<SKSE::ActionEvent>*) override;

private:
    ShoutHandler() = default;
    ShoutHandler(const ShoutHandler&) = delete;
    ShoutHandler(ShoutHandler&&) = delete;
    ~ShoutHandler() override = default;

    ShoutHandler& operator=(const ShoutHandler&) = delete;
    ShoutHandler& operator=(ShoutHandler&&) = delete;

    // Helper methods
    void StoreOriginalShoutData(RE::TESShout* shout);
    void RestoreNPCShoutValues(RE::TESShout* shout);
    void ApplyShoutScaling(RE::TESShout* shout, int totalSouls);

    float CalculateDistanceMultiplier(int dragonSouls);
    float CalculateMagnitudeMultiplier(int dragonSouls);
    float CalculateCooldownMultiplier(int dragonSouls);
    int CountUnlockedShoutWords(RE::PlayerCharacter* player);
};


