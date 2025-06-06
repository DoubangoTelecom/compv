/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2016 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

/**
 *  \file SDL_test_images.h
 *
 *  Include file for SDL test framework.
 *
 *  This code is a part of the SDL2_test library, not the main SDL library.
 */

/*

 Defines some images for tests.

*/

#ifndef _SDL_test_images_h
#define _SDL_test_images_h

#include "SDL.h"

#include "begin_code.h"
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/**
 *Type for test images.
 */
typedef struct SDLTest_SurfaceImage_s {
    int width;
    int height;
    unsigned int bytes_per_pixel; /* 3:RGB, 4:RGBA */
    const char *pixel_data;
} SDLTest_SurfaceImage_t;

/* Test images */
SDL_Surface *SDLTest_ImageBlit();
SDL_Surface *SDLTest_ImageBlitColor();
SDL_Surface *SDLTest_ImageBlitAlpha();
SDL_Surface *SDLTest_ImageBlitBlendAdd();
SDL_Surface *SDLTest_ImageBlitBlend();
SDL_Surface *SDLTest_ImageBlitBlendMod();
SDL_Surface *SDLTest_ImageBlitBlendNone();
SDL_Surface *SDLTest_ImageBlitBlendAll();
SDL_Surface *SDLTest_ImageFace();
SDL_Surface *SDLTest_ImagePrimitives();
SDL_Surface *SDLTest_ImagePrimitivesBlend();

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
#include "close_code.h"

#endif /* _SDL_test_images_h */

/* vi: set ts=4 sw=4 expandtab: */
