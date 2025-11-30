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

RE::BSEventNotifyControl ShoutHandler::ProcessEvent(const SKSE::ActionEvent* a_event, RE::BSTEventSource<SKSE::ActionEvent>*) {
    if (!a_event) {
        return RE::BSEventNotifyControl::kContinue;
    }

    // Only handle voice cast/fire events (shouts)
    if (a_event->type != SKSE::ActionEvent::Type::kVoiceCast && 
        a_event->type != SKSE::ActionEvent::Type::kVoiceFire) {
        return RE::BSEventNotifyControl::kContinue;
    }

    auto* config = Config::GetSingleton();
    auto* player = RE::PlayerCharacter::GetSingleton();
    
    if (!player || !a_event->actor) {
        return RE::BSEventNotifyControl::kContinue;
    }

    // CRITICAL: Only modify if the PLAYER is shouting, not NPCs!
    if (a_event->actor->formID != player->formID) {
        return RE::BSEventNotifyControl::kContinue;
    }

    // Get the shout from the event
    auto* shout = a_event->sourceForm ? a_event->sourceForm->As<RE::TESShout>() : nullptr;
    if (!shout) {
        return RE::BSEventNotifyControl::kContinue;
    }
    
    // Get dragon souls for scaling
    int unspentSouls = static_cast<int>(player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kDragonSouls));
    int spentSouls = config->bCountSpentSouls ? CountUnlockedShoutWords(player) : 0;
    int totalSouls = unspentSouls + spentSouls;
    
    // Calculate separate multipliers for distance and magnitude
    float distanceMultiplier = CalculateDistanceMultiplier(totalSouls);
    float magnitudeMultiplier = CalculateMagnitudeMultiplier(totalSouls);

    if (config->bEnableDebugLogging) {
        SKSE::log::info("Player shout detected: {}", shout->GetName());
        SKSE::log::info("  Unspent Souls: {}", unspentSouls);
        SKSE::log::info("  Spent Souls (unlocked words): {}", spentSouls);
        SKSE::log::info("  Total Souls: {}", totalSouls);
        SKSE::log::info("  Distance Multiplier: {}", distanceMultiplier);
        SKSE::log::info("  Magnitude Multiplier: {}", magnitudeMultiplier);
    }

    // Modify spell magnitude AND projectile speed/range
    int effectsModified = 0;
    
    for (int i = 0; i < 3; i++) {
        auto& variation = shout->variations[i];
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

                    // Special handling for Slow Time shout
                    // Slow Time magnitude is a time scale multiplier (e.g., 0.3 = 30% speed)
                    // LOWER values = slower time, so we need to DIVIDE instead of multiply
                    bool isSlowTime = effect->baseEffect->HasArchetype(RE::EffectArchetypes::ArchetypeID::kSlowTime);

                    if (isSlowTime) {
                        // For slow time: divide by multiplier to make time slower
                        // This makes the time scale smaller (slower) with more souls
                        float slowedMagnitude = original.originalMagnitude / magnitudeMultiplier;

                        // CRITICAL: Clamp to minimum 0.05 (5% time scale) to prevent freeze
                        // Without this, high multipliers can make magnitude approach 0,
                        // freezing time and preventing the effect timer from ticking
                        constexpr float MIN_TIME_SCALE = 0.05f;
                        effect->effectItem.magnitude = std::max(slowedMagnitude, MIN_TIME_SCALE);
                    } else {
                        // For all other effects: multiply normally to increase power
                        effect->effectItem.magnitude = original.originalMagnitude * magnitudeMultiplier;
                    }
                    
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
                            SKSE::log::info("  Effect #{}: mag {} -> {}, proj speed {} -> {}, range {} -> {} {}",
                                i + 1,
                                original.originalMagnitude, effect->effectItem.magnitude,
                                origProj.originalSpeed, projectile->data.speed,
                                origProj.originalRange, projectile->data.range,
                                isSlowTime ? "(SlowTime - inverted)" : "");
                        }
                    } else {
                        if (config->bEnableDebugLogging) {
                            SKSE::log::info("  Effect #{}: mag {} -> {} (no projectile){}",
                                i + 1,
                                original.originalMagnitude, effect->effectItem.magnitude,
                                isSlowTime ? " (SlowTime - inverted)" : "");
                        }
                    }
                }
            }
        }
    }

    if (config->bEnableDebugLogging) {
        SKSE::log::info("  Total effects modified: {}", effectsModified);
    }

    // NOTE: We do NOT restore values here because:
    // 1. We always recalculate from stored originals each cast
    // 2. Values update dynamically as player's soul count changes
    // 3. NPCs will use current values, but they update with each player shout

    return RE::BSEventNotifyControl::kContinue;
}

float ShoutHandler::CalculateDistanceMultiplier(int dragonSouls) {
    auto* config = Config::GetSingleton();

    // Clamp dragon souls to max
    int clampedSouls = std::min(dragonSouls, config->iMaxDragonSouls);

    // Formula: MinMultiplier + (DragonSouls * Multiplier)
    // This allows shouts to start weaker (if fMinDistanceMultiplier < 1.0)
    // Example: fMinDistanceMultiplier=0.5, at 0 souls = 50% power, scales up from there
    float multiplier = config->fMinDistanceMultiplier + (static_cast<float>(clampedSouls) * config->fDistanceMultiplier);

    return multiplier;
}

float ShoutHandler::CalculateMagnitudeMultiplier(int dragonSouls) {
    auto* config = Config::GetSingleton();

    // Clamp dragon souls to max
    int clampedSouls = std::min(dragonSouls, config->iMaxDragonSouls);

    // Formula: MinMultiplier + (DragonSouls * Multiplier)
    // This allows shouts to start weaker (if fMinMagnitudeMultiplier < 1.0)
    // Example: fMinMagnitudeMultiplier=0.5, at 0 souls = 50% power, scales up from there
    float multiplier = config->fMinMagnitudeMultiplier + (static_cast<float>(clampedSouls) * config->fMagnitudeMultiplier);

    return multiplier;
}

int ShoutHandler::CountUnlockedShoutWords(RE::PlayerCharacter* player) {
    if (!player) {
        return 0;
    }

    int unlockedWords = 0;
    
    // Get all shouts in the game
    auto* dataHandler = RE::TESDataHandler::GetSingleton();
    if (!dataHandler) {
        return 0;
    }

    // Iterate through all shouts
    for (auto& shout : dataHandler->GetFormArray<RE::TESShout>()) {
        if (shout && player->HasShout(shout)) {
            // Check each of the 3 word variations
            for (int i = 0; i < 3; i++) {
                auto* word = shout->variations[i].word;
                if (word && word->GetKnown()) {
                    unlockedWords++;
                }
            }
        }
    }

    return unlockedWords;
}


