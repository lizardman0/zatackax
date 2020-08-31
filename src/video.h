#ifndef VIDEO_H
#define VIDEO_H

#include <stdbool.h>

#include <SDL.h>

#include "common.h"
#include "config.h"
#include "particle.h"

/*
 * The main framebuffer surface.
 */
extern SDL_Surface *screen;
extern SDL_Texture *screenTexture;

/*
 * Surface holding graphics that don't need to change every frame
 * during a game round (i.e. everything except particles).
 */
extern SDL_Surface *gameScreen;
extern SDL_Texture *gameScreenTexture;

extern SDL_Renderer *renderer;
extern SDL_Window *window;

/*
 * Current window width and height.
 */
extern int WINDOW_W;
extern int WINDOW_H;

/*
 * True to temporarily freeze what's currently being displayed.
 */
extern bool screenFreeze;

int initScreen(void);
void renderSurface(SDL_Surface*);

#endif
