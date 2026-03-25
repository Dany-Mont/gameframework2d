#ifndef __BULLET_H__
#define __BULLET_H__

#include"simple_logger.h"
#include "entity.h"


#define BULLET_SPEED 6
#define BULLET_MAX_RANGE 2000
#define BULLET_MAX_BOUNCES 3
#define BULLET_FIRE_RATE 60

typedef struct
{
    int damage;
    int remainingDamage;
    float distanceTraveled;
    int bounceCount;
    GFC_Vector2D direction;
} BulletData;

Entity *bullet_new(GFC_Vector2D position, GFC_Vector2D direction, int damage);

void bullet_think(Entity *self);
void bullet_update(Entity *self);
void bullet_free(Entity *self);

void bullet_on_entity_collide(Entity *self, Entity *other);

void bullet_on_tile_collide(Entity *self, GFC_Vector2D normal);

void bullet_on_hit_enemy(Entity *bullet, Entity *enemy);
#endif
