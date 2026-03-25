#include "simple_logger.h"
#include "gf2d_sprite.h"
#include "world.h"
#include "worldgen.h"

/* ── Tile IDs ────────────────────────────────────────────────────────── */
#define TILE_BORDER     1
#define TILE_STONE      2
#define TILE_GRAVEL     3
#define TILE_FOREST     4
#define TILE_GRASS      5
#define TILE_DIRT       6
#define TILE_SAND       7
#define TILE_WATER      8

/* ── Elevation thresholds (-1.0 to 1.0) ─────────────────────────────── */
#define LEVEL_WATER    -0.50f
#define LEVEL_SAND     -0.35f
#define LEVEL_DIRT      -0.20f
#define LEVEL_GRASS     0.05f
#define LEVEL_FOREST    0.15f
#define LEVEL_GRAVEL    0.30f
#define LEVEL_STONE     0.50f
/* above LEVEL_STONE = border/impassable */

/* ── Perlin image ────────────────────────────────────────────────────── */
static Sprite *gPerlinSprite = NULL;

static void perlin_load()
{
    if (gPerlinSprite) return;
    gPerlinSprite = gf2d_sprite_load_all(
        "../images/perlin-grey.png", 0, 0, 1, 1);
    if (!gPerlinSprite)
        slog("worldgen: failed to load perlin-grey.png");
    else
        slog("worldgen: perlin image loaded");
}

/* Sample pixel brightness at normalised coords (0..1), returns -1.0 to 1.0 */
static float perlin_sample(float nx, float ny)
{
    SDL_Surface *surf;
    Uint8       *p;
    Uint32       pixel;
    Uint8        r, g, b, a;
    int          px, py;

    if (!gPerlinSprite || !gPerlinSprite->surface) return 0.0f;
    surf = gPerlinSprite->surface;

    px = (int)(nx * surf->w) % surf->w;
    py = (int)(ny * surf->h) % surf->h;
    if (px < 0) px = 0;
    if (py < 0) py = 0;

    p = (Uint8 *)surf->pixels
        + py * surf->pitch
        + px * surf->format->BytesPerPixel;

    pixel = 0;
    switch (surf->format->BytesPerPixel)
    {
        case 1: pixel = *p;                                  break;
        case 2: pixel = *(Uint16 *)p;                        break;
        case 3: pixel = p[0] | (p[1] << 8) | (p[2] << 16); break;
        case 4: pixel = *(Uint32 *)p;                        break;
    }
    SDL_GetRGBA(pixel, surf->format, &r, &g, &b, &a);

    /* 0-255 → -1.0 to 1.0 */
    return ((float)r / 127.5f) - 1.0f;
}

/* ── Elevation → tile ────────────────────────────────────────────────── */
static Uint8 elevation_to_tile(float elev)
{
    if (elev < LEVEL_WATER)  return TILE_WATER;
    if (elev < LEVEL_SAND)   return TILE_SAND;
    if (elev < LEVEL_DIRT)   return TILE_DIRT;
    if (elev < LEVEL_GRASS)  return TILE_GRASS;
    if (elev < LEVEL_FOREST) return TILE_FOREST;
    if (elev < LEVEL_GRAVEL) return TILE_GRAVEL;
    if (elev < LEVEL_STONE)  return TILE_STONE;
    return TILE_BORDER;
}

/* ── Public API ──────────────────────────────────────────────────────── */

World *worldgen_generate(Uint32 width, Uint32 height)
{
    World  *world;
    Uint32  x, y, index;
    float   nx, ny, elev;

    world = world_new(width, height);
    if (!world)
    {
        slog("worldgen_generate: world_new failed");
        return NULL;
    }

    perlin_load();
    float offsetX = (float)(SDL_GetTicks() % 1000) / 1000.0f;
    float offsetY = (float)((SDL_GetTicks() * 7) % 1000) / 1000.0f;

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            index = x + (y * width);

            /* hard border on all edges */
            if (x == 0 || y == 0 || x == width - 1 || y == height - 1)
            {
                world->tileMap[index] = TILE_BORDER;
                continue;
            }

            /* sample perlin image with random offset */
            nx   = (float)x / (float)width  + offsetX;
            ny   = (float)y / (float)height + offsetY;
            elev = perlin_sample(nx, ny);

            world->tileMap[index] = elevation_to_tile(elev);
        }
    }

    world->background = gf2d_sprite_load_image(
        "../images/backgrounds/bg_flat.png");
    world->tileSet    = gf2d_sprite_load_all(
        "../images/backgrounds/tileset2.png", 64, 64, 1, 1);

    if (!world->tileSet)
    {
        slog("worldgen_generate: failed to load tileset");
        world_free(world);
        return NULL;
    }

    world_tile_layer_build(world);
    slog("worldgen_generate: built %ux%u map", width, height);
    return world;
}