/* video -- window and screen handling.
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

#include "video.h"

SDL_Surface *screen = NULL;

SDL_Texture *screenTexture = NULL;

SDL_Window *window = NULL;

SDL_Surface *gameScreen = NULL;

SDL_Texture *gameScreenTexture = NULL;

SDL_Renderer *renderer = NULL;

int WINDOW_W = DEFAULT_WINDOW_W;

int WINDOW_H = DEFAULT_WINDOW_H;

bool screenFreeze = false;

/**
 * Initialize the application window and main screen surfaces.
 *
 * @return 1 if the initialization was successful, 0 if not.
 */
int initScreen(void)
{
    SDL_FreeSurface(screen);
    // TODO, proper init surface
    window = SDL_CreateWindow("Zatacka X", SDL_WINDOWPOS_UNDEFINED,
                     SDL_WINDOWPOS_UNDEFINED,
                     WINDOW_W,
                     WINDOW_H,
                     SDL_WINDOW_OPENGL | (fullscreen ? SDL_WINDOW_FULLSCREEN : SDL_WINDOW_RESIZABLE));
    renderer = SDL_CreateRenderer(window, -1, 0);

    screen = SDL_CreateRGBSurface(0, WINDOW_W, WINDOW_H, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    screenTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WINDOW_W, WINDOW_H);

    SDL_FreeSurface(gameScreen);
    gameScreen = SDL_CreateRGBSurface(SDL_SWSURFACE,
                                      screen->w,
                                      screen->h,
                                      screen->format->BitsPerPixel,
                                      screen->format->Rmask,
                                      screen->format->Gmask,
                                      screen->format->Bmask,
                                      0);
    gameScreenTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WINDOW_W, WINDOW_H);
    SDL_SetColorKey(gameScreen, SDL_TRUE, 0);

    initParticleScreen(WINDOW_W, WINDOW_H);

    return (screen == NULL || gameScreen == NULL || screenTexture == NULL || gameScreenTexture == NULL) ? 0 : 1;
}

void renderSurface(SDL_Surface* surface) {
    SDL_UpdateTexture(screenTexture, NULL, surface->pixels, surface->pitch);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, screenTexture, NULL, NULL);
    SDL_RenderPresent(renderer);
}
