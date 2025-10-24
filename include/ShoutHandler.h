#pragma once

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

// ============================================
// Shout Event Handler
// ============================================
class ShoutHandler : public RE::BSTEventSink<RE::ShoutAttack::Event> {
public:
    static ShoutHandler* GetSingleton();

    // Event handler
    RE::BSEventNotifyControl ProcessEvent(const RE::ShoutAttack::Event* a_event, RE::BSTEventSource<RE::ShoutAttack::Event>*) override;

    // Helper methods
    float CalculateDistanceMultiplier(int dragonSouls);
    float CalculateMagnitudeMultiplier(int dragonSouls);

private:
    ShoutHandler() = default;
    ShoutHandler(const ShoutHandler&) = delete;
    ShoutHandler(ShoutHandler&&) = delete;
    ~ShoutHandler() override = default;

    ShoutHandler& operator=(const ShoutHandler&) = delete;
    ShoutHandler& operator=(ShoutHandler&&) = delete;
};


