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

ShoutHandler* ShoutHandler::GetSingleton() {
    static ShoutHandler singleton;
    return &singleton;
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

    if (a_event->actor->formID != player->formID) {
        return RE::BSEventNotifyControl::kContinue;
    }

    auto* shout = a_event->sourceForm ? a_event->sourceForm->As<RE::TESShout>() : nullptr;
    if (!shout) {
        return RE::BSEventNotifyControl::kContinue;
    }

    int unspentSouls = static_cast<int>(player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kDragonSouls));
    int spentSouls = config->bCountSpentSouls ? CountUnlockedShoutWords(player) : 0;
    int totalSouls = unspentSouls + spentSouls;

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

    int effectsModified = 0;

    for (int i = 0; i < 3; i++) {
        auto& variation = shout->variations[i];
        if (variation.spell) {
            for (auto* effect : variation.spell->effects) {
                if (effect && effect->baseEffect) {
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

                    bool isSlowTime = effect->baseEffect->HasArchetype(RE::EffectArchetypes::ArchetypeID::kSlowTime);

                    if (isSlowTime) {
                        float slowedMagnitude = original.originalMagnitude / magnitudeMultiplier;

                        constexpr float MIN_TIME_SCALE = 0.05f;
                        effect->effectItem.magnitude = std::max(slowedMagnitude, MIN_TIME_SCALE);
                    } else {
                        effect->effectItem.magnitude = original.originalMagnitude * magnitudeMultiplier;
                    }

                    effectsModified++;

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


