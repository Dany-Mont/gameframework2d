#include "simple_logger.h"
#include "gf2d_sprite.h"
#include "gfc_vector.h"
#include "entity.h"
#include "player.h"
#include "enemy.h"
#include "pickup.h"

#define TANK_SPEED      1.0f
#define TANK_HEALTH     400
#define TANK_DAMAGE     35
#define TANK_DAMAGE_CD  90 

static void tank_think(Entity *self)
{
    Entity      *player;
    EnemyData   *data;
    TankData    *tank;
    GFC_Vector2D dir = {0};

    if (!self || !self->data) return;
    data = (EnemyData *)self->data;
    tank = (TankData *)data->typeData;

    if (tank->damageCooldown > 0) tank->damageCooldown--;

    player = player_entity_get();
    if (!player) return;

    gfc_vector2d_sub(dir, player->position, self->position);
    gfc_vector2d_normalize(&dir);
    gfc_vector2d_scale(self->velocity, dir, TANK_SPEED);
}

static void tank_update(Entity *self)
{
    if (!self) return;
    self->frame += 0.05f;   /* slower animation to match slow movement     */
    if (self->frame >= 104) self->frame = 96;
    gfc_vector2d_add(self->position, self->position, self->velocity);
     enemy_separate(self, 60.0f);   /* match roughly to bounds size */
}

static void tank_on_entity_collide(Entity *self, Entity *other)
{
    EnemyData *data;
    TankData  *tank;

    if (!self || !self->data || !other) return;
    if (other->free != enemy_free && other == player_entity_get())
    {
        data = (EnemyData *)self->data;
        tank = (TankData *)data->typeData;
        if (tank->damageCooldown <= 0)
        {
            player_take_damage(other, data->damage);
            tank->damageCooldown = TANK_DAMAGE_CD;
        }
    }
}

Entity *tank_new(GFC_Vector2D position)
{
    Entity    *self;
    EnemyData *data;
    TankData  *tank;

    self = entity_new();
    if (!self) { slog("tank_new: entity_new failed"); return NULL; }

    data = gfc_allocate_array(sizeof(EnemyData), 1);
    if (!data) { slog("tank_new: EnemyData alloc failed"); entity_free(self); return NULL; }

    tank = gfc_allocate_array(sizeof(TankData), 1);
    if (!tank) { slog("tank_new: TankData alloc failed"); free(data); entity_free(self); return NULL; }

    data->health      = TANK_HEALTH;
    data->maxHealth   = TANK_HEALTH;
    data->damage      = TANK_DAMAGE;
    data->isShielded  = 0;
    data->typeData    = tank;
    data->onDeath = pickup_drop_random;

    tank->damageCooldown = 0;

    self->data            = data;
    self->position        = position;
    self->frame           = 96;
    self->rotationCenter  = gfc_vector2d(64, 64);
    self->bounds          = gfc_rect(-40, -40, 80, 80);  
    self->sprite          = gf2d_sprite_load_all("../images/space_bug.png", 128, 128, 16, 0);
    self->think           = tank_think;
    self->update          = tank_update;
    self->free            = enemy_free;
    self->onEntityCollide = tank_on_entity_collide;
    

    return self;
}