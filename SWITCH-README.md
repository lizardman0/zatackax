# Zatacka X - Switch port

Quick and dirty port of zatackax.

## Known issues

- No support for single joycon mode
- No icon
- Config is hardcoded and not saved on exit. This was mostly deliberate, but I should probably add a
  way where overrides are saved to the sd card.

## Building

Install devkitpro, the required port libs, and run make on `switch-build` folder. Ideally I
should've integrated the build with the original project's autotools configuration, but this keeps
things simple for now.
