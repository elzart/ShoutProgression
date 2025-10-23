# Shout Progression

A Skyrim SKSE plugin that adds progression to shout mechanics by scaling shout distance based on player level.

## Description

In vanilla Skyrim, shout mechanics are limited to discovering new words and shouts. This plugin introduces a dynamic progression element that makes shouts more exciting and rewarding as you level up.

**Key Features:**
- **Progressive Shout Distance**: At lower levels, your shouts reach shorter distances. At higher levels, they can exceed standard distances by multiple times.
- **Fully Customizable**: Configure the scaling through a simple INI file with an easy-to-understand linear formula.
- **Lightweight**: Minimal performance impact, only processes shout events.

## How It Works

The plugin uses a linear interpolation formula to calculate a range multiplier based on your current level:

```
multiplier = MinMultiplier + (MaxMultiplier - MinMultiplier) × (currentLevel / maxLevel)
finalRange = baseRange × multiplier
```

**Example with default settings:**
- Level 1: 0.5× range (50% of base)
- Level 20: ~0.97× range (97% of base)
- Level 41: ~1.5× range (150% of base)
- Level 81: 2.0× range (200% of base)

## Installation

1. Install [SKSE64](https://skse.silverlock.org/) for your version of Skyrim
2. Copy `ShoutProgression.dll` to `Data/SKSE/Plugins/`
3. Copy `ShoutProgression.ini` to `Data/SKSE/Plugins/` (optional, defaults will be used if not present)

## Configuration

Edit `Data/SKSE/Plugins/ShoutProgression.ini` to customize the behavior:

```ini
[ShoutProgression]
; Minimum range multiplier at level 1
fMinMultiplier = 0.5

; Maximum range multiplier at max level
fMaxMultiplier = 2.0

; Maximum level for scaling
iMaxLevel = 81

[General]
; Enable detailed debug logging
bEnableDebugLogging = false
```

### Configuration Options

- **fMinMultiplier**: Range multiplier at level 1 (default: 0.5)
  - Set to 1.0 for no penalty at low levels
  - Set lower for more dramatic progression
  
- **fMaxMultiplier**: Range multiplier at maximum level (default: 2.0)
  - Set to 3.0 or higher for longer ranges at max level
  - Set to 1.0 to disable progression entirely
  
- **iMaxLevel**: Maximum level for scaling calculations (default: 81)
  - Increase if using mods that uncap the level system
  
- **bEnableDebugLogging**: Enable detailed logging (default: false)
  - Set to true to see shout range calculations in the log file
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
