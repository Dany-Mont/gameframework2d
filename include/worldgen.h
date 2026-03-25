#ifndef __WORLDGEN_H__
#define __WORLDGEN_H__

#include "world.h"

/**
 * @brief generate a world using perlin-grey.png for elevation
 * @param width   map width in tiles
 * @param height  map height in tiles
 * @return NULL on error, a fully built World otherwise
 */
World *worldgen_generate(Uint32 width, Uint32 height);

#endif
