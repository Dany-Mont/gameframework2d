#include "simple_logger.h"
#include "gf2d_sprite.h"
#include "player.h"
#include "enemy.h"
#include "pickup.h"

#define GRUNT_SPEED         2.0f
#define GRUNT_HEALTH        50
#define GRUNT_DAMAGE        4
#define GRUNT_DAMAGE_CD     90      /* frames between damage ticks          */

static void grunt_think(Entity *self)
{
    Entity      *player;
    EnemyData   *data;
    GruntData   *grunt;
    GFC_Vector2D dir = {0};

    if (!self || !self->data) return;
    data  = (EnemyData *)self->data;
    grunt = (GruntData *)data->typeData;

    if (grunt->damageCooldown > 0) grunt->damageCooldown--;

    player = player_entity_get();
    if (!player) return;

    gfc_vector2d_sub(dir, player->position, self->position);
    gfc_vector2d_normalize(&dir);
    gfc_vector2d_scale(self->velocity, dir, GRUNT_SPEED);
}

static void grunt_update(Entity *self)
{
    if (!self) return;
    self->frame += 0.1f;
    if (self->frame >= 16) self->frame = 0;
    gfc_vector2d_add(self->position, self->position, self->velocity);
    enemy_separate(self, 60.0f);   /* match roughly to bounds size */
}

static void grunt_on_entity_collide(Entity *self, Entity *other)
{
    EnemyData *data;
    GruntData *grunt;

    if (!self || !self->data || !other) return;
    if (other->free != enemy_free && other == player_entity_get())
    {
        data  = (EnemyData *)self->data;
        grunt = (GruntData *)data->typeData;
        if (grunt->damageCooldown <= 0)
        {
            player_take_damage(other, data->damage);
            grunt->damageCooldown = GRUNT_DAMAGE_CD;
        }
    }
}

Entity *grunt_new(GFC_Vector2D position)
{
    Entity    *self;
    EnemyData *data;
    GruntData *grunt;

    self = entity_new();
    if (!self) { slog("grunt_new: entity_new failed"); return NULL; }

    data = gfc_allocate_array(sizeof(EnemyData), 1);
    if (!data) { slog("grunt_new: EnemyData alloc failed"); entity_free(self); return NULL; }

    grunt = gfc_allocate_array(sizeof(GruntData), 1);
    if (!grunt) { slog("grunt_new: GruntData alloc failed"); free(data); entity_free(self); return NULL; }

    data->health      = GRUNT_HEALTH;
    data->maxHealth   = GRUNT_HEALTH;
    data->damage      = GRUNT_DAMAGE;
    data->isShielded  = 0;
    data->typeData    = grunt;
    data->onDeath = pickup_drop_random;

    grunt->damageCooldown = 0;

    self->data            = data;
    self->position        = position;
    self->frame           = 0;
    self->rotationCenter  = gfc_vector2d(64, 64);
    self->bounds          = gfc_rect(-30, -30, 60, 60);
    self->sprite          = gf2d_sprite_load_all("../images/space_bug.png", 128, 128, 16, 0);
    self->think           = grunt_think;
    self->update          = grunt_update;
    self->free            = enemy_free;
    self->onEntityCollide = grunt_on_entity_collide;

    return self;
}