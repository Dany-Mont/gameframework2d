#include "simple_logger.h"
#include "gf2d_sprite.h"
#include "gf2d_draw.h"
#include "gfc_vector.h"
#include "entity.h"
#include "entity.h"     /* for entity_get_all / entity_get_count   */
#include "player.h"
#include "enemy.h"
#include "camera.h"
#include "pickup.h"


#define MAGE_SPEED          1.0f
#define MAGE_HEALTH         50
#define MAGE_DAMAGE         0
#define MAGE_FLEE_RANGE     250.0f  /* flees if player gets this close      */
#define MAGE_PULSE_RATE     60      /* frames between shield reapply        */
#define MAGE_SHIELD_RADIUS  420.0f  /* radius of shield pulse effect        */

static void mage_think(Entity *self)
{
    Entity       *player;
    EnemyData    *data;
    MageData     *mage;
    GFC_Vector2D  toPlayer, dir;
    float         dist;
    int           count;
    Entity       *all[1024];

    if (!self || !self->data) return;
    data = (EnemyData *)self->data;
    mage = (MageData *)data->typeData;

    mage->shieldPulseTimer--;
    if (mage->shieldPulseTimer <= 0)
    {
        mage->shieldPulseTimer = MAGE_PULSE_RATE;
        count = entity_manager_get_all_active(all, 1024);
        enemy_apply_shields_in_radius(self->position, mage->radius, all, count);
    }

    player = player_entity_get();
    if (!player) return;

    gfc_vector2d_sub(toPlayer, player->position, self->position);
    dist = gfc_vector2d_magnitude(toPlayer);

    if (dist < MAGE_FLEE_RANGE)
    {
        /* too close — flee directly away */
        gfc_vector2d_scale(dir, toPlayer, -1.0f);
        gfc_vector2d_normalize(&dir);
        gfc_vector2d_scale(self->velocity, dir, MAGE_SPEED);
    }
    else if (dist > MAGE_FLEE_RANGE + 100.0f)
    {
        /* too far — close in slowly */
        gfc_vector2d_normalize(&toPlayer);
        gfc_vector2d_scale(self->velocity, toPlayer, MAGE_SPEED);
    }
    else
    {
        /* in preferred band — strafe sideways */
        dir.x = -toPlayer.y / (dist + 0.001f);
        dir.y =  toPlayer.x / (dist + 0.001f);
        gfc_vector2d_scale(self->velocity, dir, MAGE_SPEED * 0.5f);
    }
}

static void mage_update(Entity *self)
{
    EnemyData    *data;
    MageData     *mage;
    GFC_Color     shieldColor;
    GFC_Vector2D  screenPos;
    GFC_Vector2D  offset;

    if (!self) return;

    self->frame += 0.1f;
    if (self->frame >= 16) self->frame = 0;
    gfc_vector2d_add(self->position, self->position, self->velocity);
     enemy_separate(self, 60.0f);   /* match roughly to bounds size */

    data = (EnemyData *)self->data;
    if (data)
    {
        mage = (MageData *)data->typeData;
        if (mage)
        {
            offset = camera_get_offset();
            screenPos.x = self->position.x + offset.x;
            screenPos.y = self->position.y + offset.y;

            float pulse = (float)mage->shieldPulseTimer / (float)MAGE_PULSE_RATE;
            shieldColor = gfc_color(0.4f, 0.4f, 1.0f, 1.0f); 
            gf2d_draw_circle(screenPos, (int)mage->radius, shieldColor);
        }
    }
}

/* On death: remove shields from all enemies in radius */
void mage_free(Entity *self)
{
    int       count;
    Entity  **all;

    if (!self) return;

    if (self->data)
    {
        EnemyData *data = (EnemyData *)self->data;
        MageData  *mage = (MageData *)data->typeData;
        if (mage)
        {
            int count;
            Entity *all[1024];
            count = entity_manager_get_all_active(all, 1024);
            enemy_remove_shields_in_radius(self->position, mage->radius, all, count);
        }
    }

    /* then do normal enemy cleanup */
    enemy_free(self);
}

Entity *mage_new(GFC_Vector2D position)
{
    Entity    *self;
    EnemyData *data;
    MageData  *mage;

    self = entity_new();
    if (!self) { slog("mage_new: entity_new failed"); return NULL; }

    data = gfc_allocate_array(sizeof(EnemyData), 1);
    if (!data) { slog("mage_new: EnemyData alloc failed"); entity_free(self); return NULL; }

    mage = gfc_allocate_array(sizeof(MageData), 1);
    if (!mage) { slog("mage_new: MageData alloc failed"); free(data); entity_free(self); return NULL; }

    data->health      = MAGE_HEALTH;
    data->maxHealth   = MAGE_HEALTH;
    data->damage      = MAGE_DAMAGE;
    data->isShielded  = 0;
    data->typeData    = mage;
    data->onDeath = pickup_drop_random;

    mage->shieldPulseTimer = MAGE_PULSE_RATE;
    mage->radius           = MAGE_SHIELD_RADIUS;

    self->data           = data;
    self->position       = position;
    self->frame          = 0;
    self->rotationCenter = gfc_vector2d(64, 64);
    self->bounds         = gfc_rect(-20, -20, 40, 40);
    self->sprite         = gf2d_sprite_load_all("../images/space_bug_top.png", 128, 128, 16, 0);
    self->think          = mage_think;
    self->update         = mage_update;
    self->free           = mage_free;   /* custom free to strip shields on death */

    return self;
}

void mage_draw_shield(Entity *self)
{
    EnemyData    *data;
    GFC_Color     shieldColor;
    GFC_Vector2D  screenPos, offset;
    Entity       *all[1024];
    int           count, i;

    if (!self || !self->data) return;

    count = entity_manager_get_all_active(all, 1024);
    offset = camera_get_offset();
    shieldColor = gfc_color8(100, 100, 255, 180);

    for (i = 0; i < count; i++)
    {
        if (!all[i] || !all[i]->_inuse) continue;
        if (!entity_is_enemy(all[i]))   continue;
        if (all[i]->free == mage_free)  continue;  /* skip mages themselves */

        data = (EnemyData *)all[i]->data;
        if (!data || !data->isShielded) continue;

        screenPos.x = all[i]->position.x + offset.x;
        screenPos.y = all[i]->position.y + offset.y;
        gf2d_draw_circle(screenPos, 40, shieldColor);  /* 40px around each enemy */
    }
}