<p align="center" width="100%"><img src="/assets/logo/logo_web.png?raw=true" alt="NSMBW Practice Mode logo" width="70%" /></p>

A mod for *New Super Mario Bros. Wii* that adds a variety of features helpful for speedrunning practice.

Still in development, but already usable.

## Setup

Download the latest release from [the Releases page](https://github.com/RoadrunnerWMC/NSMBW-Practice-Mode/releases) (under "Assets").

NSMBW Practice Mode is a Riivolution patch, so install the Homebrew Channel and Riivolution first, and then install the mod to your SD card. There are many tutorials on how to set up Riivolution patches online -- I won't repeat them here. Dolphin Emulator also has native support for applying Riivolution patches to games.

## Features

- In-game timer with two decimal places ([see note](#note-regarding-the-timer))
- Save / reload state *(EARLY IN DEVELOPMENT: currently only works within the same room, and only a few things are saved/restored)*
- Reload the current room
- Switch to any powerup

## Controls

There are currently three control schemes: two for sideways Wii Remote (you can choose between them in the Riivolution menu), and one for Wii Remote + Nunchuck.

| Action | Sideways Wiimote ("A") | Sideways Wiimote ("B") | Wiimote + Nunchuk |
| - | - | - | - |
| Save state | Minus | Minus | Minus |
| Restore state | A | B | C |
| Reload room (return to original position) | Minus + 1 | Minus + 1 | Minus + B |
| Reload room (keep current position) | Minus + 2 | Minus + 2 | Minus + A |
| Cycle powerup | Minus + B | Minus + A | Minus + Z |
| Toggle invincibility | Minus + D-Pad Up | Minus + D-Pad Up | Minus + Joystick Up |
| Exit stage | Minus + D-Pad Down | Minus + D-Pad Down | Minus + Joystick Down |

## Special Thanks

- RoadrunnerWMC: main developer
- RootCubed: [original implementation of timer with decimal places](https://github.com/RootCubed/BetterIGT) (fully rewritten for this mod, but was helpful as a reference)
- Pidgey and others in the NSMBW speedrunning community who have suggested features and provided feedback

## Note Regarding the Timer

The retail game rounds timer values *up* instead of down. For example, when the timer shows "463", the *true* value would actually be somewhere in `462 < t <= 463`, for example 462.58. (This is why Mario dies on the first frame the timer hits "000": because "0.1 seconds left" is displayed as "001", not "000".)

This leads to a problem when adding fractional decimal points to the timer: what used to be an apparent timer value of "463" would now be shown as "462.58", even though the player would probably still want to see "463" as the first three digits.

To solve this, this mod effectively\* adds 0.99 seconds to the displayed time, which ensures that the integer part will always match the original timer value exactly. This makes the behavior around endpoints a bit odd (the timer starts at a number like "500.99", the "hurry up" music plays at "100.99", and Mario dies at "0.99"), but it was still considered the best option.

*\*The actual calculation is a bit more complicated, to avoid rounding errors.*
