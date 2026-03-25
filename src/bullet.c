#include "simple_logger.h"
#include "gf2d_sprite.h"
#include "gfc_vector.h"

#include "entity.h"
#include "enemy.h"
#include "bullet.h"



void bullet_free(Entity *self)
{
    if (!self) return;
    if (self->data)
    {
        free(self->data);
        self->data = NULL;
    }
}

void bullet_think(Entity *self)
{
    BulletData *data;
    if (!self || !self->data) return;
    data = (BulletData *)self->data;
    if (data->distanceTraveled >= BULLET_MAX_RANGE)
    {
        entity_free(self);
        return;
    }
}


void bullet_update(Entity *self)
{
    BulletData *data;
    float dx, dy, distThisFrame;

    if (!self || !self->data) return;
    data = (BulletData *)self->data;

    dx = self->velocity.x;
    dy = self->velocity.y;
    distThisFrame = sqrt((dx * dx) + (dy * dy));
    data->distanceTraveled += distThisFrame;


    gfc_vector2d_add(self->position, self->position, self->velocity);

    self->rotation = atan2(data->direction.y, data->direction.x) * (180.0 / M_PI);
}
void bullet_on_entity_collide(Entity *self, Entity *other)
{
    if (!self || !other) return;
     //slog("bullet_on_entity_collide fired, is_enemy: %i", entity_is_enemy(other));
    if (entity_is_enemy(other))
        bullet_on_hit_enemy(self, other);
}
void bullet_on_hit_enemy(Entity *bullet, Entity *enemy)
{
    BulletData *data;
    int remaining;

    if (!bullet || !bullet->data || !enemy) return;
    data = (BulletData *)bullet->data;

    remaining = enemy_take_damage(enemy, data->remainingDamage);

    if (remaining <= 0)
    {
        /* enemy survived, bullet is spent */
        entity_free(bullet);
        return;
    }

    /* enemy died, bullet keeps going with leftover damage */
    data->remainingDamage = remaining;
}

void bullet_on_hit_tile(Entity *bullet, GFC_Vector2D normal)
{
    BulletData *data;
    if (!bullet || !bullet->data) return;
    data = (BulletData *)bullet->data;

    data->bounceCount++;
    if (data->bounceCount > BULLET_MAX_BOUNCES)
    {
        entity_free(bullet);
        return;
    }

    /* Reflect direction off the tile normal */
    if (normal.x != 0)
        data->direction.x = -data->direction.x;
    if (normal.y != 0)
        data->direction.y = -data->direction.y;

    /* Update velocity to match new direction */
    gfc_vector2d_scale(bullet->velocity, data->direction, BULLET_SPEED);
}


Entity *bullet_new(GFC_Vector2D position, GFC_Vector2D direction, int damage)
{
    Entity *self;
    BulletData *data;

    self = entity_new();
    if (!self)
    {
        slog("failed to create bullet entity");
        return NULL;
    }

    data = gfc_allocate_array(sizeof(BulletData), 1);
    if (!data)
    {
        slog("failed to allocate bullet data");
        entity_free(self);
        return NULL;
     }
    data->damage = damage;
    data->remainingDamage = damage;
    data->distanceTraveled = 0;

    data->bounceCount = 0;
    data->direction = direction;
    gfc_vector2d_scale(self->velocity, direction, BULLET_SPEED);
    self->data = data;

    self->sprite = gf2d_sprite_load_all("../images/bullet.png",16,16,1,0);
    self->frame = 0;
    self->position = position;
    self->rotation = atan2(direction.y, direction.x);
    self->rotation *= (180.0 / M_PI);
    self->rotationCenter = gfc_vector2d(8,8);
    self->scale = gfc_vector2d(3,3);
    self->bounds   = gfc_rect(-4, -4, 8, 8);

    self->think = bullet_think;
    self->update = bullet_update;
    self->free = bullet_free;
    self->onTileCollide = bullet_on_hit_tile;
    self->onEntityCollide = bullet_on_entity_collide;
    return self;
}



