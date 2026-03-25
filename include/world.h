#ifndef __WORLD_H__
#define __WORLD_H__
#include "gf2d_sprite.h"

typedef struct
{
    Sprite *background;         //background image for the world
    Sprite *tileLayer;           //prerendered tile layer for the world
    Sprite *tileSet;            //sprite containing the tiles to draw with
    Uint8 *tileMap;             //The tiles that make up the world
    Uint32 tileHeight, tileWidth;    //the size of the world in tiles
}World;

/**
 * @brief create a new world
 * @return NULL on error, a blank world otherwise
 */
World *world_new();

/**
 * @brief free a world from memory
 * @param world the world to free
 */
void world_free(World *world);

/**
 * @brief draw the world to the screen
 * @param world the world to draw
 */
void world_draw(World *world);

/**
 * @brief create a test world
 * @return NULL on error, the test world otherwise
 */
World *world_test_new();


/**
 * @brief set up the camera bounds for a world
 * @param world the world to set up the camera for  
 */
void world_setup_camera(World *world);

/**
 * @brief load a world from a json file
 * @param filename the file to load from
 * @return NULL on error, the loaded world otherwise
 */
World *world_load(const char *filename);

void world_tile_layer_build(World *world);

#endif
