#include "ShoutHandler.h"
#include "Config.h"
#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <unordered_map>

// Store ORIGINAL effect values per spell effect
struct EffectData {
    std::uint32_t originalDuration;
    std::uint32_t originalArea;
    float originalMagnitude;
};
static std::unordered_map<RE::Effect*, EffectData> g_originalEffects;

// Store ORIGINAL projectile values
struct ProjectileData {
    float originalSpeed;
    float originalRange;
};
static std::unordered_map<RE::BGSProjectile*, ProjectileData> g_originalProjectiles;

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

    // Get player level
    int playerLevel = player->GetLevel();
    
    // Calculate range multiplier
    float rangeMultiplier = CalculateRangeMultiplier(playerLevel);

    if (config->bEnableDebugLogging) {
        SKSE::log::info("Shout detected: {}", a_event->shout->GetName());
        SKSE::log::info("  Player Level: {}", playerLevel);
        SKSE::log::info("  Range Multiplier: {}", rangeMultiplier);
    }

    // Modify spell magnitude AND projectile speed/range
    int effectsModified = 0;
    
    for (int i = 0; i < 3; i++) {
        auto& variation = a_event->shout->variations[i];
        if (variation.spell) {
            for (auto* effect : variation.spell->effects) {
                if (effect && effect->baseEffect) {
                    // Store original effect values
                    if (g_originalEffects.find(effect) == g_originalEffects.end()) {
                        g_originalEffects[effect] = {
                            effect->effectItem.duration,
                            effect->effectItem.area,
                            effect->effectItem.magnitude
                        };
                    }

                    // Modify magnitude - this affects the power/intensity
                    auto& original = g_originalEffects[effect];
                    effect->effectItem.magnitude = original.originalMagnitude * rangeMultiplier;
                    
                    effectsModified++;

                    // ALSO modify the projectile's speed AND range if it has one
                    auto* projectile = effect->baseEffect->data.projectileBase;
                    if (projectile) {
                        // Store original projectile values
                        if (g_originalProjectiles.find(projectile) == g_originalProjectiles.end()) {
                            g_originalProjectiles[projectile] = {
                                projectile->data.speed,
                                projectile->data.range
                            };
                        }
                        
                        // Modify BOTH speed and range using stored originals
                        auto& origProj = g_originalProjectiles[projectile];
                        projectile->data.speed = origProj.originalSpeed * rangeMultiplier;
                        projectile->data.range = origProj.originalRange * rangeMultiplier;
                        
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

float ShoutHandler::CalculateRangeMultiplier(int playerLevel) {
    auto* config = Config::GetSingleton();
    
    // Clamp player level to max level
    int clampedLevel = std::min(playerLevel, config->iMaxLevel);
    
    // Linear interpolation formula:
    // multiplier = MinMultiplier + (MaxMultiplier - MinMultiplier) * (currentLevel / maxLevel)
    float levelRatio = static_cast<float>(clampedLevel) / static_cast<float>(config->iMaxLevel);
    float multiplier = config->fMinMultiplier + (config->fMaxMultiplier - config->fMinMultiplier) * levelRatio;
    
    return multiplier;
}

