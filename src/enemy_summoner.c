#include "simple_logger.h"
#include "gf2d_sprite.h"
#include "enemy.h"
#include "pickup.h"
#include "player.h"

#define SUMMONER_SPEED      1.2f
#define SUMMONER_HEALTH     50
#define SUMMONER_DAMAGE     0
#define SUMMONER_FLEE_RANGE 300.0f 
#define SPAWN_OFFSET        80.0f  

static void summoner_think(Entity *self)
{
    Entity        *player;
    EnemyData     *data;
    SummonerData  *summoner;
    GFC_Vector2D   toPlayer, fleeDir, spawnPos;
    float          dist;
    int            i;

    if (!self || !self->data) return;
    data     = (EnemyData *)self->data;
    summoner = (SummonerData *)data->typeData;

    if (summoner->spawnCooldown > 0) summoner->spawnCooldown--;

    player = player_entity_get();
    if (!player) return;

      gfc_vector2d_sub(toPlayer, player->position, self->position);
    dist = gfc_vector2d_magnitude(toPlayer);

    if (dist < SUMMONER_FLEE_RANGE)
    {
        gfc_vector2d_scale(fleeDir, toPlayer, -1.0f);
        gfc_vector2d_normalize(&fleeDir);
        gfc_vector2d_scale(self->velocity, fleeDir, SUMMONER_SPEED);
    }
    else if (dist > SUMMONER_FLEE_RANGE + 100.0f)
    {
        gfc_vector2d_normalize(&toPlayer);
        gfc_vector2d_scale(self->velocity, toPlayer, SUMMONER_SPEED);
    }
    else
    {
        fleeDir.x = -toPlayer.y / (dist + 0.001f);
        fleeDir.y =  toPlayer.x / (dist + 0.001f);
        gfc_vector2d_scale(self->velocity, fleeDir, SUMMONER_SPEED * 0.5f);
    }

    /* count how many tracked grunts are still alive */
    summoner->activeGrunts = 0;
    for (i = 0; i < SUMMONER_MAX_ACTIVE; i++)
    {
        if (summoner->grunts[i] && summoner->grunts[i]->_inuse)
            summoner->activeGrunts++;
        else
            summoner->grunts[i] = NULL;   /* clear dead pointers            */
    }

    /* spawn if below cap and cooled down */
    if (summoner->activeGrunts < SUMMONER_MAX_ACTIVE &&
        summoner->spawnCooldown <= 0)
    {
        /* find an empty slot */
        for (i = 0; i < SUMMONER_MAX_ACTIVE; i++)
        {
            if (summoner->grunts[i] == NULL)
            {
                /* spawn offset from summoner position */
                spawnPos.x = self->position.x + ((i % 2 == 0) ?  SPAWN_OFFSET : -SPAWN_OFFSET);
                spawnPos.y = self->position.y + ((i < 2)      ? -SPAWN_OFFSET :  SPAWN_OFFSET);

                summoner->grunts[i]    = grunt_new(spawnPos);
                summoner->spawnCooldown = SUMMONER_SPAWN_COOLDOWN;
                summoner->activeGrunts++;
                slog("summoner spawned grunt %i", i);
                break;
            }
        }
    }
}

static void summoner_update(Entity *self)
{
    if (!self) return;
    self->frame += 0.08f;
    if (self->frame >= 32) self->frame = 24;
    gfc_vector2d_add(self->position, self->position, self->velocity);
     enemy_separate(self, 60.0f);   /* match roughly to bounds size */
}

Entity *summoner_new(GFC_Vector2D position)
{
    Entity       *self;
    EnemyData    *data;
    SummonerData *summoner;

    self = entity_new();
    if (!self) { slog("summoner_new: entity_new failed"); return NULL; }

    data = gfc_allocate_array(sizeof(EnemyData), 1);
    if (!data) { slog("summoner_new: EnemyData alloc failed"); entity_free(self); return NULL; }

    summoner = gfc_allocate_array(sizeof(SummonerData), 1);
    if (!summoner) { slog("summoner_new: SummonerData alloc failed"); free(data); entity_free(self); return NULL; }

    data->health      = SUMMONER_HEALTH;
    data->maxHealth   = SUMMONER_HEALTH;
    data->damage      = SUMMONER_DAMAGE;
    data->isShielded  = 0;
    data->typeData    = summoner;
    data->onDeath = pickup_drop_random;

    summoner->spawnCooldown = SUMMONER_SPAWN_COOLDOWN;
    summoner->activeGrunts  = 0;

    self->data           = data;
    self->position       = position;
    self->frame          = 24;
    self->rotationCenter = gfc_vector2d(64, 64);
    self->bounds         = gfc_rect(-30, -30, 60, 60);
    self->sprite         = gf2d_sprite_load_all("../images/space_bug.png", 128, 128, 16, 0);
    self->think          = summoner_think;
    self->update         = summoner_update;
    self->free           = enemy_free;
    

    return self;
}