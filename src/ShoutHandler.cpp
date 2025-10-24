#include "ShoutHandler.h"
#include "Config.h"
#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <unordered_map>
#include <mutex>

// Store ORIGINAL effect values per spell effect
struct EffectData {
    std::uint32_t originalDuration;
    std::uint32_t originalArea;
    float originalMagnitude;
};
static std::unordered_map<RE::Effect*, EffectData> g_originalEffects;
static std::mutex g_effectsMutex;

// Store ORIGINAL projectile values
struct ProjectileData {
    float originalSpeed;
    float originalRange;
};
static std::unordered_map<RE::BGSProjectile*, ProjectileData> g_originalProjectiles;
static std::mutex g_projectilesMutex;

ShoutHandler* ShoutHandler::GetSingleton() {
    static ShoutHandler singleton;
    return &singleton;
}

RE::BSEventNotifyControl ShoutHandler::ProcessEvent(const RE::ShoutAttack::Event* a_event, RE::BSTEventSource<RE::ShoutAttack::Event>*) {
    if (!a_event || !a_event->shout) {
        return RE::BSEventNotifyControl::kContinue;
    }

    auto* config = Config::GetSingleton();
    auto* player = RE::PlayerCharacter::GetSingleton();
    
    if (!player) {
        return RE::BSEventNotifyControl::kContinue;
    }

    // NOTE: ShoutAttack::Event only fires for player shouts, not NPCs
    // Get dragon souls absorbed
    int dragonSouls = static_cast<int>(player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kDragonSouls));
    
    // Calculate separate multipliers for distance and magnitude
    float distanceMultiplier = CalculateDistanceMultiplier(dragonSouls);
    float magnitudeMultiplier = CalculateMagnitudeMultiplier(dragonSouls);

    if (config->bEnableDebugLogging) {
        SKSE::log::info("Shout detected: {}", a_event->shout->GetName());
        SKSE::log::info("  Dragon Souls: {}", dragonSouls);
        SKSE::log::info("  Distance Multiplier: {}", distanceMultiplier);
        SKSE::log::info("  Magnitude Multiplier: {}", magnitudeMultiplier);
    }

    // Modify spell magnitude AND projectile speed/range
    int effectsModified = 0;
    
    for (int i = 0; i < 3; i++) {
        auto& variation = a_event->shout->variations[i];
        if (variation.spell) {
            for (auto* effect : variation.spell->effects) {
                if (effect && effect->baseEffect) {
                    // Thread-safe: Store original effect values
                    {
                        std::lock_guard<std::mutex> lock(g_effectsMutex);
                        if (g_originalEffects.find(effect) == g_originalEffects.end()) {
                            g_originalEffects[effect] = {
                                effect->effectItem.duration,
                                effect->effectItem.area,
                                effect->effectItem.magnitude
                            };
                        }
                    }

                    // Modify magnitude - this affects the power/intensity
                    EffectData original;
                    {
                        std::lock_guard<std::mutex> lock(g_effectsMutex);
                        original = g_originalEffects[effect];
                    }
                    effect->effectItem.magnitude = original.originalMagnitude * magnitudeMultiplier;
                    
                    effectsModified++;

                    // ALSO modify the projectile's speed AND range if it has one
                    auto* projectile = effect->baseEffect->data.projectileBase;
                    if (projectile) {
                        // Thread-safe: Store original projectile values
                        {
                            std::lock_guard<std::mutex> lock(g_projectilesMutex);
                            if (g_originalProjectiles.find(projectile) == g_originalProjectiles.end()) {
                                g_originalProjectiles[projectile] = {
                                    projectile->data.speed,
                                    projectile->data.range
                                };
                            }
                        }
                        
                        // Modify BOTH speed and range using stored originals
                        ProjectileData origProj;
                        {
                            std::lock_guard<std::mutex> lock(g_projectilesMutex);
                            origProj = g_originalProjectiles[projectile];
                        }
                        projectile->data.speed = origProj.originalSpeed * distanceMultiplier;
                        projectile->data.range = origProj.originalRange * distanceMultiplier;
                        
                        if (config->bEnableDebugLogging) {
                            SKSE::log::info("  Effect #{}: mag {} -> {}, proj speed {} -> {}, range {} -> {}",
                                i + 1,
                                original.originalMagnitude, effect->effectItem.magnitude,
                                origProj.originalSpeed, projectile->data.speed,
                                origProj.originalRange, projectile->data.range);
                        }
                    } else {
                        if (config->bEnableDebugLogging) {
                            SKSE::log::info("  Effect #{}: mag {} -> {} (no projectile)",
                                i + 1,
                                original.originalMagnitude, effect->effectItem.magnitude);
                        }
                    }
                }
            }
        }
    }

    if (config->bEnableDebugLogging) {
        SKSE::log::info("  Total effects modified: {}", effectsModified);
    }

    return RE::BSEventNotifyControl::kContinue;
}

float ShoutHandler::CalculateDistanceMultiplier(int dragonSouls) {
    auto* config = Config::GetSingleton();
    
    // Clamp dragon souls to max
    int clampedSouls = std::min(dragonSouls, config->iMaxDragonSouls);
    
    // Formula: Base * (1.0 + DragonSouls * Multiplier)
    float multiplier = 1.0f + (static_cast<float>(clampedSouls) * config->fDistanceMultiplier);
    
    return multiplier;
}

float ShoutHandler::CalculateMagnitudeMultiplier(int dragonSouls) {
    auto* config = Config::GetSingleton();
    
    // Clamp dragon souls to max
    int clampedSouls = std::min(dragonSouls, config->iMaxDragonSouls);
    
    // Formula: Base * (1.0 + DragonSouls * Multiplier)
    float multiplier = 1.0f + (static_cast<float>(clampedSouls) * config->fMagnitudeMultiplier);
    
    return multiplier;
}

