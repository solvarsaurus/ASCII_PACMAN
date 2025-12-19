# ASCII PAC-MAN Terminal

A retro-styled Pac-Man game running on ESP32 with a monochrome CRT terminal aesthetic. Play classic Pac-Man directly from your phone's browser!

## Features

- **Classic Pac-Man Gameplay**: Navigate mazes, collect pellets, avoid ghosts
- **CRT Terminal Aesthetic**: Green monochrome display with scanline effects
- **Progressive Difficulty**: Speed increases with each level
- **Power Pellets**: Turn the tables and chase ghosts
- **High Score Persistence**: Scores saved to ESP32 flash memory
- **WiFi Manager**: Easy setup via captive portal
- **mDNS Support**: Access via `espcatgames.local`
- **Mobile-Friendly**: Touch controls optimized for phones

## Hardware Requirements

- ESP32 (any variant - C3, S3, or classic)
- USB cable for programming
- Phone or computer with WiFi

## Quick Start

### 1. Flash the Firmware
```bash
# Using Arduino IDE
Open ASCII_PACMAN.ino and upload to ESP32

# Or using PlatformIO
pio run --target upload
```

### 2. WiFi Setup
1. Power on the ESP32
2. Connect to WiFi network: `ESP games`
3. Configure your home WiFi in the captive portal
4. ESP32 will restart and connect

### 3. Play the Game
Open browser and navigate to:
- `http://espcatgames.local` (recommended)
- Or use IP address shown in serial monitor

## How to Play

### Game Rules
- **Objective**: Eat all pellets to advance to the next level
- **Small Pellets**: Worth 10 points each
- **Power Pellets**: Large pellets that let you eat ghosts (200 points)
- **Ghost AI**: Blinky (red ghost) chases you intelligently
- **Game Over**: Getting caught by a ghost when not powered up

### Controls
- **▲ ▼ ◀ ▶**: D-pad for movement
- **EXIT**: Return to splash screen
- **START**: Begin game from splash screen

### Scoring
- Small pellet: 10 points
- Power pellet: 10 points + ghost vulnerability
- Eating ghost: 200 points
- High scores automatically saved

## Game Mechanics

### Level Progression
- Complete maze by eating all pellets
- Each level increases game speed
- Speed formula: `max(60ms, 150ms - level × 15ms)`
- Maze resets with same layout

### Ghost Behavior
- **Normal Mode**: Ghost chases player using shortest path
- **Frightened Mode**: Ghost runs away from player (40 ticks)
- Ghost respawns at center after being eaten

### Maze Layout
- 20×21 grid (200×210 pixels)
- Walls, pellets, power pellets, and ghost house
- Two tunnel passages on sides

## Technical Details

### Dependencies
```cpp
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <ESPmDNS.h>
#include <Preferences.h>
```

### API Endpoints
- `GET /` - Main game interface
- `GET /api/highscore` - Retrieve saved high score
- `POST /api/setscore?score=XXX` - Save new high score

### Storage
High scores stored in NVS (Non-Volatile Storage):
- Namespace: `pacman-data`
- Key: `highscore`

### Network Configuration
- Default AP: `ESP games`
- mDNS hostname: `espcatgames.local`
- Web server port: 80

## Customization

### Change WiFi AP Name
```cpp
wm.autoConnect("YourGameName");
```

### Change mDNS Hostname
```cpp
MDNS.begin("yourhostname");
// Access via: http://yourhostname.local
```

### Adjust Game Speed
Modify in JavaScript:
```javascript
let gameSpeed = 150; // Starting speed in milliseconds
gameSpeed = Math.max(60, 150 - (level * 15)); // Speed progression
```

### Modify Maze Layout
Edit `mazeTemplate` array:
- `0` = Empty space
- `1` = Wall
- `2` = Small pellet
- `3` = Power pellet

### Add More Ghosts
Duplicate ghost object and AI logic:
```javascript
let pinky = { x: 10, y: 8, isFrightened: false };
// Add movement logic in update()
```

## Troubleshooting

### Can't Connect to WiFi
- Check serial monitor (115200 baud) for status
- Reset WiFi settings by reflashing
- Ensure phone is on 2.4GHz network (ESP32 doesn't support 5GHz)

### mDNS Not Working
- Some networks block mDNS
- Use IP address from serial monitor instead
- On Android, install "Bonjour Browser" app

### Game Runs Too Fast/Slow
- Adjust `gameSpeed` variable
- Check browser performance (try different browser)
- Reduce CRT effects for better performance

### High Score Not Saving
- Check serial monitor for errors
- Ensure preferences namespace initialized
- Try erasing flash and reflashing

## Development

### File Structure
```
ASCII_PACMAN/
├── ASCII_PACMAN.ino    # Main firmware
└── README.md           # This file
```

### Adding Features Ideas
- Multiple ghost personalities (Pinky, Inky, Clyde)
- Fruit bonuses
- Lives system
- Sound effects via Web Audio API
- Leaderboard with multiple scores
- Different maze layouts per level
- Multiplayer mode

### Performance Optimization
- Canvas is 200×210px for smooth rendering
- Game loop uses `requestAnimationFrame`
- Minimal DOM updates
- Efficient pathfinding for ghost AI

## Browser Compatibility

Tested and working on:
- Chrome/Edge (mobile & desktop)
- Safari (iOS & macOS)
- Firefox (mobile & desktop)

## Credits

Inspired by the original Pac-Man (1980) by Namco. This is a tribute implementation with a retro terminal aesthetic.

## License

Open source - modify and share freely!

## Support

For issues or questions:
1. Check serial monitor output (115200 baud)
2. Verify WiFi connection status
3. Test with different browsers
4. Check ESP32 has sufficient power supply

---

**READY PLAYER ONE** 🕹️

Press START to begin!
