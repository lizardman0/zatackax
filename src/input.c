/* input -- keyboard, mouse and joystick/gamepad handling.
 * Copyright (C) 2010-2020 Simen Heggestøyl <simenheg@runbox.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "input.h"

const int BUTTON_NAME_MAX_LEN = 14;

/*
 * The joystick button that is treated as an enter button in the
 * menus.
 */
const Uint8 JOY_ENTER_BUTTON = 1;

// 322 is the number of possible SDL keys (see SDL_keysym.h).
bool keyDown[512];

/*
 * A separate map is kept for each possible joystick (0-7). See the
 * `button` type in `input.h` for details about how buttons and axes
 * are represented.
 */
bool joyButtonDown[MAX_PLAYERS][128];

unsigned int numJoys = 0;

SDL_Joystick *joys[MAX_PLAYERS];
Uint32 lastAnalogMove = 0;

/**
 * Open all connected joysticks for interaction.
 */
void openJoysticks(void)
{
    numJoys = SDL_NumJoysticks();

    for (unsigned int i = 0; i < numJoys; i++) {
        joys[i] = SDL_JoystickOpen(i);
    }
}

/**
 * Close all open joysticks.
 */
void closeJoysticks(void)
{
    for (unsigned int i = 0; i < numJoys; i++) {
        SDL_JoystickClose(joys[i]);
    }
}

/**
 * Return true if button `b` belongs to a joystick.
 */
bool isJoyButton(button b)
{
    return ((b >> 9) > 0) && ((b & 0xf0) == 0);
}

bool isJoyAxisValidMotion(SDL_Event* event) {
  return (event->type == SDL_JOYAXISMOTION &&
          (event->jaxis.value > JOYAXIS_DEADZONE || event->jaxis.value < -JOYAXIS_DEADZONE));
}

/**
 * Return true if button `b` represents a joystick axis.
 */
bool isJoyAxis(button b)
{
    return ((b >> 9) > 0) && ((b & 0xf0) > 0);
}

/**
 * Return the joystick device index for button `b`, or -1 if none.
 */
int joyIndex(button b)
{
    if (isJoyButton(b) || isJoyAxis(b)) {
        return (b >> 9) - 1;
    }

    return -1;
}

/**
 * Return the button number for joystick button `b`.
 */
int joyButtonNumber(button b)
{
    return b & 0x7f;
}

/**
 * Return the axis direction for joystick button `b`.
 */
enum joyDir joyAxisDir(button b)
{
    return (b & 0x70) >> 4;
}

/**
 * Return the axis number corresponding to axis event `e`.
 */
int axisNumber(SDL_JoyAxisEvent e)
{
    if (e.axis) {
        return e.value < 0 ? JOY_DIR_UP : JOY_DIR_DOWN;
    }

    return e.value < 0 ? JOY_DIR_LEFT : JOY_DIR_RIGHT;
}

/**
 * Return true if one of the enter buttons are currently down.
 */
bool enterButtonDown(void)
{
    if (keyDown[SDL_SCANCODE_SPACE] || keyDown[SDL_SCANCODE_RETURN]) {
        return true;
    }

    for (unsigned int i = 0; i < numJoys; i++) {
        if (joyButtonDown[i][JOY_ENTER_BUTTON]) {
            return true;
        }
    }

    return false;
}

/**
 * Mark all enter buttons as not down.
 */
void clearEnterButtons(void)
{
    keyDown[SDL_SCANCODE_RETURN] = false;
    keyDown[SDL_SCANCODE_SPACE] = false;

    for (unsigned int i = 0; i < numJoys; i++) {
        joyButtonDown[i][JOY_ENTER_BUTTON] = false;
    }
}

/**
 * Return true if button `b` is currently down.
 */
bool buttonDown(button b)
{
    if (isJoyButton(b) || isJoyAxis(b)) {
        return joyButtonDown[joyIndex(b)][joyButtonNumber(b)];
    }

    return keyDown[b];
}

/**
 * Mark button `b` as not down anymore.
 */
void clearButton(button b)
{
    if (isJoyButton(b) || isJoyAxis(b)) {
        joyButtonDown[joyIndex(b)][joyButtonNumber(b)] = false;
    }
    else {
        keyDown[b] = false;
    }
}

/**
 * Return true if one of the generic left buttons are down, and reset
 * it in such case.
 */
bool menuButtonQuery(enum keySymbol ks)
{
  SDL_Scancode lkeys[4][3] = {
    {SDL_SCANCODE_UP, SDL_SCANCODE_K, SDL_SCANCODE_P},
    {SDL_SCANCODE_RIGHT, SDL_SCANCODE_L, SDL_SCANCODE_F},
    {SDL_SCANCODE_DOWN, SDL_SCANCODE_J, SDL_SCANCODE_N},
    {SDL_SCANCODE_LEFT, SDL_SCANCODE_H, SDL_SCANCODE_B}
  };

    for (int i = 0; i < 3; i++) {
        if (keyDown[lkeys[ks][i]]) {
            keyDown[lkeys[ks][i]] = false;
            return true;
        }
    }

    Uint32 currentTime = SDL_GetTicks();
    Uint32 analogMoveDelta = currentTime - lastAnalogMove;

    for (unsigned int i = 0; i < numJoys; i++) {
      if (analogMoveDelta > 200)  {
        if (ks == ZK_KEY_UP && joyButtonDown[i][JOY_DIR_UP << 4]) {
            lastAnalogMove = currentTime;
            return true;
        }
        if (ks == ZK_KEY_RIGHT && joyButtonDown[i][JOY_DIR_RIGHT << 4]) {
            lastAnalogMove = currentTime;
            return true;
        }
        if (ks == ZK_KEY_DOWN && joyButtonDown[i][JOY_DIR_DOWN << 4]) {
            lastAnalogMove = currentTime;
            return true;
        }
        if (ks == ZK_KEY_LEFT && joyButtonDown[i][JOY_DIR_LEFT << 4]) {
            lastAnalogMove = currentTime;
            return true;
        }
      }
    }

    return false;
}

/**
 * Return an appropriate name for button `b`. Return '\0' if the
 * button could not be named.
 */
char *buttonName(button b)
{
    char *keyname = calloc(BUTTON_NAME_MAX_LEN, sizeof(char));

    if (isJoyButton(b)) {
        if (numJoys > 1) {
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "joy-%d (%d)",
                     joyButtonNumber(b) + 1, joyIndex(b) + 1);
        }
        else {
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "joy-%d",
                     joyButtonNumber(b) + 1);
        }
    }
    else if (isJoyAxis(b)) {
        int axisNumber = joyAxisDir(b);
        char *axisName = "";

        switch (axisNumber) {
        case JOY_DIR_UP:
            axisName = "up"; break;
        case JOY_DIR_RIGHT:
            axisName = "right"; break;
        case JOY_DIR_DOWN:
            axisName = "down"; break;
        case JOY_DIR_LEFT:
            axisName = "left"; break;
        }

        if (numJoys > 1) {
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "joy-%s (%d)",
                     axisName, joyIndex(b) + 1);
        }
        else {
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "joy-%s", axisName);
        }
    }
    else if ((b >= SDLK_a && b <= SDLK_z)
                 || (b >= SDLK_0 && b <= SDLK_9)) {
        snprintf(keyname, BUTTON_NAME_MAX_LEN, "%c", b);
    }
    else if (b >= SDLK_F1 && b <= SDLK_F15) {
        snprintf(keyname, BUTTON_NAME_MAX_LEN, "F%d", b - SDLK_F1 + 1);
    }
    else {
        switch (b) {
        case SDL_SCANCODE_UNKNOWN:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "none"); break;
        case SDL_SCANCODE_LEFT:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "left"); break;
        case SDL_SCANCODE_RIGHT:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "right"); break;
        case SDL_SCANCODE_UP:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "up"); break;
        case SDL_SCANCODE_DOWN:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "down"); break;
        case SDLK_SCROLLLOCK:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "scr-lk"); break;
        case SDLK_PAUSE:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "pause"); break;
        case SDLK_DELETE:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "del"); break;
        case SDLK_INSERT:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "ins"); break;
        case SDLK_HOME:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "home"); break;
        case SDLK_END:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "end"); break;
        case SDLK_MENU:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "menu"); break;
        case SDLK_PRINTSCREEN:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "prt-sc"); break;
        case SDLK_PAGEUP:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "pg up"); break;
        case SDLK_PAGEDOWN:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "pg dn"); break;
        case SDLK_RSHIFT:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "r-shift"); break;
        case SDLK_LSHIFT:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "l-shift"); break;
        case SDLK_RCTRL:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "r-ctrl"); break;
        case SDLK_LCTRL:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "l-ctrl"); break;
        case SDLK_RALT:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "r-alt"); break;
        case SDLK_LALT:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "l-alt"); break;
        case SDLK_MODE:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "alt gr"); break;
        case SDLK_RGUI:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "r-super"); break;
        case SDLK_LGUI:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "l-super"); break;
        case SDLK_TAB:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "tab"); break;
        case SDLK_PERIOD:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "."); break;
        case SDLK_COMMA:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, ","); break;
        case SDLK_SEMICOLON:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, ";"); break;
        case SDLK_MINUS:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "-"); break;
        case SDLK_QUOTE:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "'"); break;
        case SDLK_BACKQUOTE:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "`"); break;
        case SDLK_PLUS:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "+"); break;
        case SDLK_EQUALS:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "="); break;
        case SDLK_CARET:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "^"); break;
        case SDLK_SLASH:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "/"); break;
        case SDLK_BACKSLASH:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "\\"); break;
        case SDLK_LESS:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "<"); break;
        case SDLK_LEFTBRACKET:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "["); break;
        case SDLK_RIGHTBRACKET:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "]"); break;
        case SDLK_BACKSPACE:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "b-space"); break;
        case SDLK_RETURN:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "enter"); break;
        case SDLK_SPACE:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "space"); break;
        case SDL_BUTTON_LEFT:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "l-mouse"); break;
        case SDL_BUTTON_MIDDLE:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "m-mouse"); break;
        case SDL_BUTTON_RIGHT:
            snprintf(keyname, BUTTON_NAME_MAX_LEN, "r-mouse"); break;
        default:
            break;
        }
    }
    return keyname;
}
