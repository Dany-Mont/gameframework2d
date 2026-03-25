#include "simple_logger.h"
#include "gfc_shape.h"
#include "quadtree.h"
#include "gf2d_draw.h"
#include "camera.h"

static int tile_is_solid(Uint8 tile)
{
    switch(tile)
    {
        case 0: return 0; 
        case 2: return 0; 
        case 3: return 0; 
        case 4: return 0; 
        case 5: return 0; 
        case 6: return 0; 
        case 7: return 0; 
        default: return 1;
    }
}


/* -------------------------------------------------------------------------
   Internal helpers
   ---------------------------------------------------------------------- */

void quadtree_draw_entity_bounds(Entity *entity, GFC_Color color)
{
    GFC_Rect bounds;
    GFC_Vector2D offset;
    if (!entity) return;
    offset = camera_get_offset();
    bounds = quadtree_entity_bounds(entity);
    bounds.x += offset.x;
    bounds.y += offset.y;
    gf2d_draw_rect(bounds, color);
}


GFC_Rect quadtree_entity_bounds(Entity *entity)
{
    GFC_Rect r = {0};
    if (!entity) return r;
    return gfc_rect(
        entity->position.x + entity->bounds.x,
        entity->position.y + entity->bounds.y,
        entity->bounds.w,
        entity->bounds.h
    );
}


static QuadtreeNode *qt_node_new(GFC_Rect bounds, int depth)
{
    QuadtreeNode *node = gfc_allocate_array(sizeof(QuadtreeNode), 1);
    if (!node)
    {
        slog("quadtree: failed to allocate node");
        return NULL;
    }
    node->bounds = bounds;
    node->depth  = depth;
    return node;
}

static void qt_node_free(QuadtreeNode *node)
{
    int i;
    if (!node) return;
    for (i = 0; i < 4; i++)
        qt_node_free(node->children[i]);
    free(node);
}

static void qt_node_clear(QuadtreeNode *node)
{
    int i;
    if (!node) return;
    node->entityCount = 0;
    for (i = 0; i < 4; i++)
        qt_node_clear(node->children[i]);
}

static void qt_node_subdivide(QuadtreeNode *node)
{
    float hw = node->bounds.w / 2.0f;
    float hh = node->bounds.h / 2.0f;
    float x  = node->bounds.x;
    float y  = node->bounds.y;
    int   d  = node->depth + 1;

    /* NW */ node->children[0] = qt_node_new(gfc_rect(x,      y,      hw, hh), d);
    /* NE */ node->children[1] = qt_node_new(gfc_rect(x + hw, y,      hw, hh), d);
    /* SW */ node->children[2] = qt_node_new(gfc_rect(x,      y + hh, hw, hh), d);
    /* SE */ node->children[3] = qt_node_new(gfc_rect(x + hw, y + hh, hw, hh), d);
}

static int qt_is_leaf(QuadtreeNode *node)
{
    return node->children[0] == NULL;
}

/* Insert an entity into a node, subdividing if needed */
static void qt_node_insert(QuadtreeNode *node, Entity *entity)
{
    int i;
    GFC_Rect eb;

    if (!node || !entity) return;

    eb = quadtree_entity_bounds(entity);

    /* If entity doesn't overlap this node at all, skip */
    if (!gfc_rect_overlap(node->bounds, eb)) return;

    /* If we're at max depth, store here regardless */
    if (node->depth >= QUADTREE_MAX_DEPTH)
    {
        if (node->entityCount < QUADTREE_MAX_ENTITIES)
            node->entities[node->entityCount++] = entity;
        return;
    }

    if (qt_is_leaf(node) && node->entityCount < QUADTREE_MAX_ENTITIES)
    {
        node->entities[node->entityCount++] = entity;
        return;
    }


    if (qt_is_leaf(node))
    {
        qt_node_subdivide(node);
        for (i = 0; i < node->entityCount; i++)
        {
            int c;
            for (c = 0; c < 4; c++)
                qt_node_insert(node->children[c], node->entities[i]);
        }
        node->entityCount = 0;
    }

    /* Push new entity into children */
    for (i = 0; i < 4; i++)
        qt_node_insert(node->children[i], entity);
}

/* Collect all entities in nodes that overlap range into results */
static int qt_node_query(QuadtreeNode *node, GFC_Rect range, Entity **results, int maxResults, int found)
{
    int i;
    if (!node) return found;
    if (!gfc_rect_overlap(node->bounds, range)) return found;

    for (i = 0; i < node->entityCount && found < maxResults; i++)
    {
        /* Avoid duplicates */
        int dup = 0, k;
        for (k = 0; k < found; k++)
        {
            if (results[k] == node->entities[i]) { dup = 1; break; }
        }
        if (!dup)
            results[found++] = node->entities[i];
    }

    if (!qt_is_leaf(node))
    {
        for (i = 0; i < 4; i++)
            found = qt_node_query(node->children[i], range, results, maxResults, found);
    }
    return found;
}


Quadtree *quadtree_new(World *world)
{
    Quadtree *qt;
    GFC_Rect bounds;

    if (!world)
    {
        slog("quadtree_new: NULL world");
        return NULL;
    }

    qt = gfc_allocate_array(sizeof(Quadtree), 1);
    if (!qt)
    {
        slog("quadtree_new: failed to allocate quadtree");
        return NULL;
    }

    /* Derive pixel bounds from the world's tile layer */
    if (world->tileLayer && world->tileLayer->surface)
    {
        bounds = gfc_rect(0, 0,
            (float)world->tileLayer->surface->w,
            (float)world->tileLayer->surface->h);
    }
    else
    {
        /* Fallback: tilecount * tileSet frame size */
        bounds = gfc_rect(0, 0,
            (float)(world->tileWidth  * (world->tileSet ? world->tileSet->frame_w : 16)),
            (float)(world->tileHeight * (world->tileSet ? world->tileSet->frame_h : 16)));
    }

    qt->tileSize = world->tileSet ? (float)world->tileSet->frame_w : 16.0f;
    qt->root = qt_node_new(bounds, 0);
    if (!qt->root)
    {
        free(qt);
        return NULL;
    }

    slog("quadtree created: %.0fx%.0f px, tileSize:%.0f", bounds.w, bounds.h, qt->tileSize);
    return qt;
}

void quadtree_free(Quadtree *qt)
{
    if (!qt) return;
    qt_node_free(qt->root);
    free(qt);
}

void quadtree_clear(Quadtree *qt)
{
    if (!qt) return;
    qt_node_clear(qt->root);
}

void quadtree_insert(Quadtree *qt, Entity *entity)
{
    if (!qt || !entity || !entity->_inuse) return;
    qt_node_insert(qt->root, entity);
}

int quadtree_query_rect(Quadtree *qt, GFC_Rect range, Entity **results, int maxResults)
{
    if (!qt || !results || maxResults <= 0) return 0;
    return qt_node_query(qt->root, range, results, maxResults, 0);
}


/* -------------------------------------------------------------------------
   Entity vs Tile collision
   ---------------------------------------------------------------------- */

int quadtree_check_entity_vs_tiles(Entity *entity, World *world, GFC_Vector2D *outNormal)
{
    GFC_Rect eb, tileBounds;
    int tileX0, tileY0, tileX1, tileY1;
    int tx, ty;
    float tileSize;
    Uint32 index;
    int hit = 0;

    float overlapLeft, overlapRight, overlapTop, overlapBottom;
    float minOverlap;

    if (!entity || !world || !world->tileMap) return 0;

    tileSize = world->tileSet ? (float)world->tileSet->frame_w : 16.0f;

    for (ty = 0; ty <= 1; ty++) /* two passes: x then y */
    {
        eb = quadtree_entity_bounds(entity);

        tileX0 = (int)(eb.x / tileSize);
        tileY0 = (int)(eb.y / tileSize);
        tileX1 = (int)((eb.x + eb.w - 1) / tileSize);
        tileY1 = (int)((eb.y + eb.h - 1) / tileSize);

        if (tileX0 < 0) tileX0 = 0;
        if (tileY0 < 0) tileY0 = 0;
        if (tileX1 >= (int)world->tileWidth)  tileX1 = (int)world->tileWidth  - 1;
        if (tileY1 >= (int)world->tileHeight) tileY1 = (int)world->tileHeight - 1;

        for (int j = tileY0; j <= tileY1; j++)
        {
            for (int i = tileX0; i <= tileX1; i++)
            {
                index = i + (j * world->tileWidth);
                if (!tile_is_solid(world->tileMap[index])) continue;

                tileBounds = gfc_rect(i * tileSize, j * tileSize, tileSize, tileSize);

                if (!gfc_rect_overlap(eb, tileBounds)) continue;

                /* Calculate penetration depth on each axis */
                overlapLeft   = (eb.x + eb.w) - tileBounds.x;
                overlapRight  = (tileBounds.x + tileBounds.w) - eb.x;
                overlapTop    = (eb.y + eb.h) - tileBounds.y;
                overlapBottom = (tileBounds.y + tileBounds.h) - eb.y;

                /* Resolve on the axis of least penetration */
                float minX = overlapLeft < overlapRight ? overlapLeft : overlapRight;
                float minY = overlapTop  < overlapBottom ? overlapTop  : overlapBottom;

                if (minX < minY)
                {
                    /* Resolve on X axis */
                    if (overlapLeft < overlapRight)
                    {
                        entity->position.x -= overlapLeft;
                        if (outNormal) { outNormal->x = -1; outNormal->y = 0; }
                    }
                    else
                    {
                        entity->position.x += overlapRight;
                        if (outNormal) { outNormal->x = 1; outNormal->y = 0; }
                    }
                    entity->velocity.x = 0;
                }
                else
                {
                    /* Resolve on Y axis */
                    if (overlapTop < overlapBottom)
                    {
                        entity->position.y -= overlapTop;
                        if (outNormal) { outNormal->x = 0; outNormal->y = -1; }
                    }
                    else
                    {
                        entity->position.y += overlapBottom;
                        if (outNormal) { outNormal->x = 0; outNormal->y = 1; }
                    }
                    entity->velocity.y = 0;
                }
                hit = 1;
            }
        }
    }
    return hit;
}



void quadtree_collide_all(
    Quadtree *qt,
    World *world,
    Entity **entities,
    int entityCount,
    void (*onEntityCollide)(Entity *a, Entity *b),
    void (*onTileCollide)(Entity *entity, GFC_Vector2D normal))
{
    int i, j;
    GFC_Vector2D normal = {0};

    if (!qt || !entities) return;

    for (i = 0; i < entityCount; i++)
    {
        Entity *a = entities[i];
        if (!a || !a->_inuse) continue;

        if (world && onTileCollide)
        {
            normal.x = 0; normal.y = 0;
            if (quadtree_check_entity_vs_tiles(a, world, &normal))
                onTileCollide(a, normal);
        }

        /* Entity vs Entity — use quadtree to find nearby candidates only */
        if (onEntityCollide)
        {
            GFC_Rect ab = quadtree_entity_bounds(a);
            Entity *nearby[64];
            int nearCount = quadtree_query_rect(qt, ab, nearby, 64);
            //slog("entity %p has %i nearby entities", (void*)a, nearCount);

            for (j = 0; j < nearCount; j++)
            {
                Entity *b = nearby[j];
                if (!b || !b->_inuse || b == a) continue;
                /* Only process each pair once */
                if (b < a) continue;

               if (gfc_rect_overlap(ab, quadtree_entity_bounds(b)))
                {
                    if (a->onEntityCollide)
                        a->onEntityCollide(a, b);
                    if (b->onEntityCollide)
                        b->onEntityCollide(b, a);
                    if (!a->onEntityCollide && !b->onEntityCollide && onEntityCollide)
                        onEntityCollide(a, b);
                }
            }
        }
    }
}
/* https://box2d.org/files/ErinCatto_DynamicBVH_Full.pdf    */
/* https://pvigier.github.io/2019/08/04/quadtree-collision-detection.html */
