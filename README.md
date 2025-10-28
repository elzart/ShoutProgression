# Shout Progression

A Skyrim SKSE plugin that adds progression to shout mechanics by scaling shout distance and power based on dragon souls absorbed.

## Description

In vanilla Skyrim, shout mechanics are limited to discovering new words and shouts. This plugin introduces a dynamic progression element that makes shouts more exciting and rewarding as you absorb dragon souls.

**Key Features:**
- **Progressive Shout Distance**: As you absorb more dragon souls, your shouts travel faster and reach farther distances.
- **Progressive Shout Magnitude**: Dragon souls increase your shout power, damage, and effect strength.
- **Dragon Soul Based**: Progression tied to dragon souls absorbed, making dragon hunting more meaningful.
- **Fully Customizable**: Configure the scaling through a simple INI file with separate multipliers for distance and magnitude.
- **Lightweight**: Minimal performance impact, only processes shout events.

## How It Works

The plugin uses a simple additive formula to calculate multipliers based on dragon souls absorbed:

```
Modified Value = Base Value × (1.0 + min(Dragon Souls, Max) × Multiplier)
```

**Separate scaling for:**
- **Distance**: Affects projectile speed and range (default: 0.15 per soul)
- **Magnitude**: Affects damage, power, and effect strength (default: 0.10 per soul)

**Example with default settings:**
- 0 souls: 1.0× distance, 1.0× magnitude (base values)
- 5 souls: 1.75× distance, 1.5× magnitude
- 10 souls: 2.5× distance, 2.0× magnitude
- 25 souls (cap): 4.75× distance, 3.5× magnitude

This means a Fus Ro Dah with 10 dragon souls will travel 2.5× faster/farther and have 2.0× the force!

## Installation

1. Install [SKSE64](https://skse.silverlock.org/) for your version of Skyrim
2. Copy `ShoutProgression.dll` to `Data/SKSE/Plugins/`
3. Copy `ShoutProgression.ini` to `Data/SKSE/Plugins/` (optional, defaults will be used if not present)

## Configuration

Edit `Data/SKSE/Plugins/ShoutProgression.ini` to customize the behavior:

```ini
[ShoutProgression]
; Distance scaling multiplier per dragon soul
; Default: 0.15 (increases projectile speed and range)
fDistanceMultiplier = 0.15

; Magnitude scaling multiplier per dragon soul
; Default: 0.10 (increases shout power/damage/effect strength)
fMagnitudeMultiplier = 0.10

; Maximum dragon souls for scaling cap
; Default: 25
iMaxDragonSouls = 25

[General]
; Enable detailed debug logging
bEnableDebugLogging = false
```

### Configuration Options

- **fDistanceMultiplier**: Distance scaling per dragon soul (default: 0.04)
  - Controls how much faster/farther shout projectiles travel per soul absorbed
  - Set to 0.0 to disable distance scaling

- **fMagnitudeMultiplier**: Magnitude scaling per dragon soul (default: 0.03)
  - Controls how much stronger shout effects become per soul absorbed
  - Affects damage, push force, duration, etc.
  - Set to 0.0 to disable magnitude scaling
  - **Note**: Slow Time shout uses inverse scaling (divides instead of multiplies) so it gets slower with more souls

- **fMinDistanceMultiplier**: Starting distance multiplier at 0 souls (default: 1.0)
  - Sets the base power for shout range/speed at the start of the game
  - Set to 1.0 for 100% of vanilla power (no penalty at start)
  - Set lower (e.g., 0.5) to make shouts start weaker and scale up with souls
  - Example: 0.5 = 50% range at 0 souls, scaling to 250% at 50 souls (0.5 + 50×0.04)

- **fMinMagnitudeMultiplier**: Starting magnitude multiplier at 0 souls (default: 1.0)
  - Sets the base power for shout damage/effects at the start of the game
  - Set to 1.0 for 100% of vanilla power (no penalty at start)
  - Set lower (e.g., 0.5) to make shouts start weaker and scale up with souls
  - Example: 0.5 = 50% power at 0 souls, scaling to 200% at 50 souls (0.5 + 50×0.03)

- **iMaxDragonSouls**: Maximum dragon souls for scaling (default: 50)
  - Caps scaling to prevent excessive power with very high soul counts
  - Increase for longer progression curve
  - Main quest provides ~15-20 dragon souls

- **bCountSpentSouls**: Toggle between total absorbed souls or only unspent souls (default: true)
  - Set to true to track total souls absorbed (spent + unspent)
  - Set to false to only track current unspent dragon souls
  - With false, you choose between powerful shouts or unlocking new words

- **bEnableDebugLogging**: Enable detailed logging (default: false)
  - Set to true to see dragon souls and multiplier calculations in the log
  - Shows actual magnitude and distance changes per shout
  - Log file location: `Documents/My Games/Skyrim Special Edition/SKSE/ShoutProgression.log`

## Compatibility

- **Skyrim Special Edition (SE)**: Yes
- **Skyrim Anniversary Edition (AE)**: Yes
- **Skyrim VR**: Yes (via CommonLibSSE-NG)
- **Other Shout Mods**: Should be compatible with most shout-related mods

## Building from Source

### Requirements
- [CMake](https://cmake.org/) 3.21+
- [vcpkg](https://github.com/microsoft/vcpkg)
- Visual Studio 2022 with C++23 support

### Build Steps
1. Clone the repository with submodules:
   ```bash
   git clone --recursive https://github.com/yourusername/ShoutProgression.git
   ```

2. Configure the project:
   ```bash
   cmake --preset vs2022-windows
   ```

3. Build:
   ```bash
   cmake --build build --config Release
   ```

The compiled DLL will be in `build/Release/ShoutProgression.dll`

## Credits

Built with [CommonLibSSE-NG](https://github.com/CharmedBaryon/CommonLibSSE-NG)

## License

MIT License - See LICENSE file for details.
