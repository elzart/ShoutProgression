#include "ShoutHandler.h"
#include "Config.h"
#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <unordered_map>
#include <mutex>

struct EffectData {
    std::uint32_t originalDuration;
    std::uint32_t originalArea;
    float originalMagnitude;
};
static std::unordered_map<RE::Effect*, EffectData> g_originalEffects;
static std::mutex g_effectsMutex;

struct ProjectileData {
    float originalSpeed;
    float originalRange;
};
static std::unordered_map<RE::BGSProjectile*, ProjectileData> g_originalProjectiles;
static std::mutex g_projectilesMutex;

struct ShoutData {
    float originalRecoveryTimes[3];
};
static std::unordered_map<RE::TESShout*, ShoutData> g_originalShouts;
static std::mutex g_shoutsMutex;

ShoutHandler* ShoutHandler::GetSingleton() {
    static ShoutHandler singleton;
    return &singleton;
}

void ShoutHandler::StoreOriginalShoutData(RE::TESShout* shout) {
    std::lock_guard<std::mutex> lock(g_shoutsMutex);
    if (g_originalShouts.find(shout) == g_originalShouts.end()) {
        ShoutData data;
        for (int i = 0; i < 3; i++) {
            data.originalRecoveryTimes[i] = shout->variations[i].recoveryTime;
        }
        g_originalShouts[shout] = data;
    }
}

void ShoutHandler::RestoreNPCShoutValues(RE::TESShout* shout) {
    // Restore original shout cooldowns for all variations
    {
        std::lock_guard<std::mutex> lock(g_shoutsMutex);
        auto it = g_originalShouts.find(shout);
        if (it != g_originalShouts.end()) {
            for (int i = 0; i < 3; i++) {
                shout->variations[i].recoveryTime = it->second.originalRecoveryTimes[i];
            }
        }
    }

    // Restore original effect and projectile values
    for (int i = 0; i < 3; i++) {
        auto& variation = shout->variations[i];
        if (variation.spell) {
            for (auto* effect : variation.spell->effects) {
                if (effect && effect->baseEffect) {
                    {
                        std::lock_guard<std::mutex> lock(g_effectsMutex);
                        auto it = g_originalEffects.find(effect);
                        if (it != g_originalEffects.end()) {
                            effect->effectItem.duration = it->second.originalDuration;
                            effect->effectItem.area = it->second.originalArea;
                            effect->effectItem.magnitude = it->second.originalMagnitude;
                        }
                    }

                    auto* projectile = effect->baseEffect->data.projectileBase;
                    if (projectile) {
                        std::lock_guard<std::mutex> lock(g_projectilesMutex);
                        auto it = g_originalProjectiles.find(projectile);
                        if (it != g_originalProjectiles.end()) {
                            projectile->data.speed = it->second.originalSpeed;
                            projectile->data.range = it->second.originalRange;
                        }
                    }
                }
            }
        }
    }
}

void ShoutHandler::ApplyShoutScaling(RE::TESShout* shout, int totalSouls) {
    auto* config = Config::GetSingleton();

    float distanceMultiplier = CalculateDistanceMultiplier(totalSouls);
    float magnitudeMultiplier = CalculateMagnitudeMultiplier(totalSouls);
    float cooldownMultiplier = CalculateCooldownMultiplier(totalSouls);

    // Apply cooldown scaling to all variations
    ShoutData originalShoutData;
    {
        std::lock_guard<std::mutex> lock(g_shoutsMutex);
        originalShoutData = g_originalShouts[shout];
    }

    for (int i = 0; i < 3; i++) {
        shout->variations[i].recoveryTime = originalShoutData.originalRecoveryTimes[i] * cooldownMultiplier;
    }

    if (config->bEnableDebugLogging) {
        SKSE::log::info("Player shout detected: {} (FormID: {:08X})", shout->GetName(), shout->GetFormID());
        SKSE::log::info("  Distance Multiplier: {}", distanceMultiplier);
        SKSE::log::info("  Magnitude Multiplier: {}", magnitudeMultiplier);
        SKSE::log::info("  Cooldown Multiplier: {}", cooldownMultiplier);
        for (int i = 0; i < 3; i++) {
            SKSE::log::info("  Variation #{} Recovery Time: {} -> {}",
                i + 1, originalShoutData.originalRecoveryTimes[i], shout->variations[i].recoveryTime);
        }
    }

    int effectsModified = 0;

    for (int i = 0; i < 3; i++) {
        auto& variation = shout->variations[i];
        if (variation.spell) {
            if (config->bEnableDebugLogging) {
                SKSE::log::info("  Variation #{} - Spell: {} (FormID: {:08X}), Effect count: {}",
                    i + 1, variation.spell->GetName(), variation.spell->GetFormID(), variation.spell->effects.size());
            }

            for (auto* effect : variation.spell->effects) {
                if (effect && effect->baseEffect) {
                    if (config->bEnableDebugLogging) {
                        SKSE::log::info("    Effect: {} (FormID: {:08X}), Archetype: {:08X}",
                            effect->baseEffect->GetName(),
                            effect->baseEffect->GetFormID(),
                            static_cast<std::uint32_t>(effect->baseEffect->GetArchetype()));
                    }

                    // Store original effect values
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

                    EffectData original;
                    {
                        std::lock_guard<std::mutex> lock(g_effectsMutex);
                        original = g_originalEffects[effect];
                    }

                    // Apply magnitude scaling (inverted for SlowTime)
                    bool isSlowTime = effect->baseEffect->HasArchetype(RE::EffectArchetypes::ArchetypeID::kSlowTime);
                    if (isSlowTime) {
                        float slowedMagnitude = original.originalMagnitude / magnitudeMultiplier;
                        constexpr float MIN_TIME_SCALE = 0.05f;
                        effect->effectItem.magnitude = std::max(slowedMagnitude, MIN_TIME_SCALE);
                    } else {
                        effect->effectItem.magnitude = original.originalMagnitude * magnitudeMultiplier;
                    }

                    effectsModified++;

                    // Apply projectile scaling
                    auto* projectile = effect->baseEffect->data.projectileBase;
                    if (projectile) {
                        {
                            std::lock_guard<std::mutex> lock(g_projectilesMutex);
                            if (g_originalProjectiles.find(projectile) == g_originalProjectiles.end()) {
                                g_originalProjectiles[projectile] = {
                                    projectile->data.speed,
                                    projectile->data.range
                                };
                            }
                        }

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
}

RE::BSEventNotifyControl ShoutHandler::ProcessEvent(const SKSE::ActionEvent* a_event, RE::BSTEventSource<SKSE::ActionEvent>*) {
    if (!a_event) {
        return RE::BSEventNotifyControl::kContinue;
    }

    if (a_event->type != SKSE::ActionEvent::Type::kVoiceCast &&
        a_event->type != SKSE::ActionEvent::Type::kVoiceFire) {
        return RE::BSEventNotifyControl::kContinue;
    }

    auto* config = Config::GetSingleton();
    auto* player = RE::PlayerCharacter::GetSingleton();

    if (!player || !a_event->actor) {
        return RE::BSEventNotifyControl::kContinue;
    }

    auto* shout = a_event->sourceForm ? a_event->sourceForm->As<RE::TESShout>() : nullptr;
    if (!shout) {
        return RE::BSEventNotifyControl::kContinue;
    }

    // Store original shout data on first encounter
    StoreOriginalShoutData(shout);

    // If an NPC is casting, restore original values to prevent NPCs from using buffed shouts
    if (a_event->actor->formID != player->formID) {
        RestoreNPCShoutValues(shout);
        return RE::BSEventNotifyControl::kContinue;
    }

    // Calculate total souls for scaling
    int unspentSouls = static_cast<int>(player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kDragonSouls));
    int spentSouls = config->bCountSpentSouls ? CountUnlockedShoutWords(player) : 0;
    int totalSouls = unspentSouls + spentSouls;

    if (config->bEnableDebugLogging) {
        SKSE::log::info("  Unspent Souls: {}", unspentSouls);
        SKSE::log::info("  Spent Souls (unlocked words): {}", spentSouls);
        SKSE::log::info("  Total Souls: {}", totalSouls);
    }

    // Apply scaling to player's shout
    ApplyShoutScaling(shout, totalSouls);

    return RE::BSEventNotifyControl::kContinue;
}

float ShoutHandler::CalculateDistanceMultiplier(int dragonSouls) {
    auto* config = Config::GetSingleton();

    int clampedSouls = std::min(dragonSouls, config->iMaxDragonSouls);

    float multiplier = config->fMinDistanceMultiplier + (static_cast<float>(clampedSouls) * config->fDistanceMultiplier);

    return multiplier;
}

float ShoutHandler::CalculateMagnitudeMultiplier(int dragonSouls) {
    auto* config = Config::GetSingleton();

    int clampedSouls = std::min(dragonSouls, config->iMaxDragonSouls);

    float multiplier = config->fMinMagnitudeMultiplier + (static_cast<float>(clampedSouls) * config->fMagnitudeMultiplier);

    return multiplier;
}

float ShoutHandler::CalculateCooldownMultiplier(int dragonSouls) {
    auto* config = Config::GetSingleton();

    int clampedSouls = std::min(dragonSouls, config->iMaxDragonSouls);

    // Cooldown reduction: higher souls = lower cooldown
    // Formula: 1.0 - (souls * reduction) = cooldown multiplier
    // Example: 1.0 - (50 * 0.01) = 0.5 (50% of original cooldown)
    float multiplier = 1.0f - (static_cast<float>(clampedSouls) * config->fCooldownReduction);

    // Clamp to minimum cooldown multiplier
    multiplier = std::max(multiplier, config->fMinCooldownMultiplier);

    return multiplier;
}

int ShoutHandler::CountUnlockedShoutWords(RE::PlayerCharacter* player) {
    if (!player) {
        return 0;
    }

    int unlockedWords = 0;

    auto* dataHandler = RE::TESDataHandler::GetSingleton();
    if (!dataHandler) {
        return 0;
    }

    for (auto& shout : dataHandler->GetFormArray<RE::TESShout>()) {
        if (shout && player->HasShout(shout)) {
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


