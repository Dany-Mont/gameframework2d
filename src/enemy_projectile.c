#include "simple_logger.h"
#include "gf2d_sprite.h"
#include "gfc_vector.h"
#include "entity.h"
#include "player.h"
#include "enemy_projectile.h"
#include "math.h"


void enemy_projectile_free(Entity *self)
{
    if (!self) return;
    if (self->data)
    {
        free(self->data);
        self->data = NULL;
    }
    self->_inuse = 0;
}

static void enemy_projectile_think(Entity *self)
{
    EnemyProjectileData *data;
    if (!self || !self->data) return;
    data = (EnemyProjectileData *)self->data;
    if (data->distanceTraveled >= ENEMY_PROJECTILE_MAX_RANGE)
    {
        entity_free(self);
    }
}

static void enemy_projectile_update(Entity *self)
{
    EnemyProjectileData *data;
    float dx, dy;

    if (!self || !self->data) return;
    data = (EnemyProjectileData *)self->data;

    dx = self->velocity.x;
    dy = self->velocity.y;
    data->distanceTraveled += sqrt((dx * dx) + (dy * dy));

    gfc_vector2d_add(self->position, self->position, self->velocity);
}

static void enemy_projectile_on_entity_collide(Entity *self, Entity *other)
{
    EnemyProjectileData *data;
    Entity              *player;

    if (!self || !other) return;

    player = player_entity_get();
    if (other != player) return;    /* only damages the player              */

    data = (EnemyProjectileData *)self->data;
    player_take_damage(other, data->damage);
    entity_free(self);
}

Entity *enemy_projectile_new(GFC_Vector2D position,
                              GFC_Vector2D direction,
                              int          damage)
{
    Entity              *self;
    EnemyProjectileData *data;

    self = entity_new();
    if (!self) { slog("enemy_projectile_new: entity_new failed"); return NULL; }

    data = gfc_allocate_array(sizeof(EnemyProjectileData), 1);
    if (!data)
    {
        slog("enemy_projectile_new: data alloc failed");
        entity_free(self);
        return NULL;
    }

    data->direction        = direction;
    data->distanceTraveled = 0;
    data->damage           = damage;

    gfc_vector2d_scale(self->velocity, direction, ENEMY_PROJECTILE_SPEED);

    self->data            = data;
    self->position        = position;
    self->rotation        = atan2(direction.y, direction.x) * (180.0 / M_PI);
    self->rotationCenter  = gfc_vector2d(8, 8);
    self->scale           = gfc_vector2d(2, 2);
    self->bounds          = gfc_rect(-4, -4, 8, 8);
    self->sprite          = gf2d_sprite_load_all("../images/bullet.png", 16, 16, 1, 0);
    self->think           = enemy_projectile_think;
    self->update          = enemy_projectile_update;
    self->free            = enemy_projectile_free;
    self->onEntityCollide = enemy_projectile_on_entity_collide;

    return self;
}