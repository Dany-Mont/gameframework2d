#include "simple_logger.h"
#include "gf2d_sprite.h"
#include "enemy.h"
#include "enemy_projectile.h"
#include "pickup.h"
#include "player.h"

#define RANGER_SPEED            2.0f
#define RANGER_HEALTH           120
#define RANGER_DAMAGE           3
#define RANGER_PREFERRED_RANGE  300.0f 
#define RANGER_FIRE_RATE        90      
#define RANGER_TOO_CLOSE        200.0f  

static void ranger_think(Entity *self)
{
    Entity       *player;
    EnemyData    *data;
    RangerData   *ranger;
    GFC_Vector2D  toPlayer, dir = {0};
    GFC_Vector2D  shootDir;
    float         dist;

    if (!self || !self->data) return;
    data   = (EnemyData *)self->data;
    ranger = (RangerData *)data->typeData;

    if (ranger->fireTimer > 0) ranger->fireTimer--;

    player = player_entity_get();
    if (!player) return;

    gfc_vector2d_sub(toPlayer, player->position, self->position);
    dist = gfc_vector2d_magnitude(toPlayer);

    if (dist < RANGER_TOO_CLOSE)
    {
        gfc_vector2d_scale(dir, toPlayer, -1.0f);
        gfc_vector2d_normalize(&dir);
        gfc_vector2d_scale(self->velocity, dir, RANGER_SPEED);
    }
    else if (dist > RANGER_PREFERRED_RANGE)
    {
        gfc_vector2d_normalize(&toPlayer);
        gfc_vector2d_scale(self->velocity, toPlayer, RANGER_SPEED);
    }
    else
    {
        dir.x = -toPlayer.y / dist;
        dir.y =  toPlayer.x / dist;
        gfc_vector2d_scale(self->velocity, dir, RANGER_SPEED * 0.5f);
    }

    self->rotation = atan2(toPlayer.y / (dist + 0.001f),
                           toPlayer.x / (dist + 0.001f)) * (180.0 / M_PI) + 270;

    if (ranger->fireTimer <= 0)
    {
        shootDir = toPlayer;
        gfc_vector2d_normalize(&shootDir);
        enemy_projectile_new(self->position, shootDir, data->damage);
        ranger->fireTimer = ranger->fireRate;
    }
}

static void ranger_update(Entity *self)
{
    if (!self) return;
    self->frame += 0.1f;
    if (self->frame >= 16) self->frame = 0;
    gfc_vector2d_add(self->position, self->position, self->velocity);
     enemy_separate(self, 60.0f);   /* match roughly to bounds size */
}

Entity *ranger_new(GFC_Vector2D position)
{
    Entity     *self;
    EnemyData  *data;
    RangerData *ranger;

    self = entity_new();
    if (!self) { slog("ranger_new: entity_new failed"); return NULL; }

    data = gfc_allocate_array(sizeof(EnemyData), 1);
    if (!data) { slog("ranger_new: EnemyData alloc failed"); entity_free(self); return NULL; }

    ranger = gfc_allocate_array(sizeof(RangerData), 1);
    if (!ranger) { slog("ranger_new: RangerData alloc failed"); free(data); entity_free(self); return NULL; }

    data->health      = RANGER_HEALTH;
    data->maxHealth   = RANGER_HEALTH;
    data->damage      = RANGER_DAMAGE;
    data->isShielded  = 0;
    data->typeData    = ranger;
    data->onDeath     = pickup_drop_random;

    ranger->preferredRange = RANGER_PREFERRED_RANGE;
    ranger->fireTimer      = RANGER_FIRE_RATE;
    ranger->fireRate       = RANGER_FIRE_RATE;

    self->data           = data;
    self->position       = position;
    self->frame          = 0;
    self->rotationCenter = gfc_vector2d(64, 64);
    
    self->bounds         = gfc_rect(-25, -25, 50, 50);
    self->sprite         = gf2d_sprite_load_all("../images/space_bug_top.png", 128, 128, 16, 0);
    self->think          = ranger_think;
    self->update         = ranger_update;
    self->free           = enemy_free;

    return self;
}