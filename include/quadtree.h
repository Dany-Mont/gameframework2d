#ifndef __QUADTREE_H__
#define __QUADTREE_H__

#include "gfc_shape.h"
#include "entity.h"
#include "world.h"

#define QUADTREE_MAX_DEPTH      5       // how deep the tree can subdivide
#define QUADTREE_MAX_ENTITIES   4       // entities per node before subdividing

typedef struct QuadtreeNode_S
{
    GFC_Rect bounds;                            // the region this node covers
    int depth;                                  // current depth of this node

    Entity *entities[QUADTREE_MAX_ENTITIES];    // entities stored in this node
    int entityCount;

    struct QuadtreeNode_S *children[4];         // NW, NE, SW, SE (NULL if leaf)
} QuadtreeNode;

typedef struct
{
    QuadtreeNode *root;
    float tileSize;                             // pixel size of a tile (for tile collision)
} Quadtree;


/* --- Lifecycle --- */

/**
 * @brief create a new quadtree spanning the given world bounds
 * @param world the world to base the bounds on
 * @return NULL on error, a new quadtree otherwise
 */
Quadtree *quadtree_new(World *world);

/**
 * @brief free the quadtree and all its nodes
 * @param qt the quadtree to free
 */
void quadtree_free(Quadtree *qt);

/**
 * @brief clear all entities from the quadtree (keeps structure, ready for re-insert)
 * @param qt the quadtree to clear
 */
void quadtree_clear(Quadtree *qt);


/* --- Population --- */

/**
 * @brief insert a single entity into the quadtree
 * @param qt the quadtree to insert into
 * @param entity the entity to insert
 */
void quadtree_insert(Quadtree *qt, Entity *entity);


/* --- Query --- */

/**
 * @brief get all entities whose bounds overlap the given rect
 * @param qt the quadtree to query
 * @param range the rectangle to search within
 * @param results array to fill with found entities
 * @param maxResults size of the results array
 * @return number of entities found
 */
int quadtree_query_rect(Quadtree *qt, GFC_Rect range, Entity **results, int maxResults);

/**
 * @brief check a single entity against the world's tile map for collision
 * @param entity the entity to check
 * @param world the world to check against
 * @param outNormal optional - set to the collision normal if a collision is found
 * @return 1 if colliding with a solid tile, 0 otherwise
 */
int quadtree_check_entity_vs_tiles(Entity *entity, World *world, GFC_Vector2D *outNormal);

/**
 * @brief run all entity vs entity and entity vs tile checks, calling the provided callbacks
 * @param qt the quadtree to use
 * @param world the world to check tiles against
 * @param onEntityCollide called when two entities overlap - given both entities
 * @param onTileCollide called when an entity hits a tile - given the entity and collision normal
 */
void quadtree_collide_all(
    Quadtree *qt,
    World *world,
    Entity **entities,
    int entityCount,
    void (*onEntityCollide)(Entity *a, Entity *b),
    void (*onTileCollide)(Entity *entity, GFC_Vector2D normal)
);


/* --- Helpers --- */

/**
 * @brief get the AABB for an entity based on its sprite frame size and position
 * @param entity the entity to get bounds for
 * @return a GFC_Rect representing the entity's bounding box
 */
GFC_Rect quadtree_entity_bounds(Entity *entity);

void quadtree_draw_entity_bounds(Entity *entity, GFC_Color color);

#endif
