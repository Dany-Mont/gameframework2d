#ifndef __ENEMY_PROJECTILE_H__
#define __ENEMY_PROJECTILE_H__

#include "gfc_vector.h"
#include "entity.h"

#define ENEMY_PROJECTILE_SPEED      4.0f
#define ENEMY_PROJECTILE_MAX_RANGE  1000.0f
#define ENEMY_PROJECTILE_DAMAGE     15

typedef struct
{
    GFC_Vector2D direction;
    float        distanceTraveled;
    int          damage;
} EnemyProjectileData;

/**
 * @brief spawn an enemy projectile
 * @param position  starting position
 * @param direction normalised direction vector
 * @param damage    how much damage it deals to the player
 */
Entity *enemy_projectile_new(GFC_Vector2D position,
                              GFC_Vector2D direction,
                              int          damage);

void enemy_projectile_free(Entity *self);

#endif /* __ENEMY_PROJECTILE_H__ */