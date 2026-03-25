#include "simple_logger.h"
#include "gf2d_sprite.h"
#include "gfc_vector.h"
#include "entity.h"
#include "player.h"
#include "weapon.h"
#include "pickup.h"
#include "math.h"

static const char *pickup_sprites[PT_COUNT] =
{
    "../images/pickup_weapon.png",
    "../images/pickup_health.png",
    "../images/pickup_upgrade.png",
    "../images/pickup_maxhealth.png",
    "../images/pickup_speed.png",
};

static const char *pickup_names[PT_COUNT] =
{
    "weapon unlock",    
    "health pack",
    "weapon upgrade",
    "max health",
    "move speed",
};

static void pickup_apply(PickupType type, Entity *player)
{
    PlayerData *data;
    int         i, unlocked;

    if (!player || !player->data) return;
    data = (PlayerData *)player->data;

    switch (type)
    {
        /* ── Weapon unlock ───────────────────────────────────────────── */
        case PT_WEAPON_UNLOCK:
        {
            /* collect all weapon slots the player doesn't have yet */
            int locked[WEAPON_COUNT];
            int lockedCount = 0;

            for (i = 1; i < WEAPON_COUNT; i++)  /* skip slot 0 = base bullet */
            {
                if (!data->hasWeapon[i])
                    locked[lockedCount++] = i;
            }

            if (lockedCount == 0)
            {
                /* all weapons owned — give a blade upgrade instead */
                slog("pickup: all weapons owned, upgrading blades");
                weapon_blade_upgrade(&data->weapons, player);
                break;
            }

            /* pick a random locked weapon */
            int slot = locked[rand() % lockedCount];
            weapon_unlock(data->hasWeapon, &data->weapons, slot);
            slog("pickup: unlocked weapon slot %i", slot);
            break;
        }

        /* ── Health pack ─────────────────────────────────────────────── */
        case PT_HEALTH_PACK:
        {
            int restore = (int)(data->maxHealth * PICKUP_HEALTH_PERCENT);
            if (restore < 1) restore = 1;
            data->health += restore;
            if (data->health > data->maxHealth) data->health = data->maxHealth;
            slog("pickup: restored %i hp, now %i/%i",
                 restore, data->health, data->maxHealth);
            break;
        }

        case PT_WEAPON_UPGRADE:
        {
            data->damageMult  *= PICKUP_DAMAGE_MULT;

            data->fireRateMult *= PICKUP_FIRERATE_MULT;

            data->weapons.cardinalTimer =
                (int)(data->weapons.cardinalTimer * PICKUP_FIRERATE_MULT);

            data->weapons.laserTimer =
                (int)(data->weapons.laserTimer * PICKUP_FIRERATE_MULT);

            data->weapons.lightningTimer =
                (int)(data->weapons.lightningTimer * PICKUP_FIRERATE_MULT);

            data->weapons.bladeSpeed *= PICKUP_DAMAGE_MULT;
            for (i = 0; i < data->weapons.bladeCount; i++)
            {
                if (!data->weapons.blades[i] ||
                    !data->weapons.blades[i]->_inuse) continue;
                BladeData *bd = (BladeData *)data->weapons.blades[i]->data;
                if (bd) bd->speed = data->weapons.bladeSpeed;
            }

            slog("pickup: weapon upgraded — damageMult=%.2f fireRateMult=%.2f",
                 data->damageMult, data->fireRateMult);
            break;
        }

        /* ── Max health increase ─────────────────────────────────────── */
        case PT_MAX_HEALTH:
        {
            data->maxHealth += PICKUP_MAX_HEALTH_BONUS;
            data->health    += PICKUP_MAX_HEALTH_BONUS; /* also heal the bonus */
            if (data->health > data->maxHealth) data->health = data->maxHealth;
            slog("pickup: max health now %i", data->maxHealth);
            break;
        }

        /* ── Move speed increase ─────────────────────────────────────── */
        case PT_MOVE_SPEED:
        {
            data->moveSpeed += PICKUP_SPEED_BONUS;
            slog("pickup: move speed now %.2f", data->moveSpeed);
            break;
        }

        default:
            slog("pickup_apply: unknown type %i", type);
            break;
    }
}


void pickup_free(Entity *self)
{
    if (!self) return;
    if (self->data)
    {
        free(self->data);
        self->data = NULL;
    }
    self->_inuse = 0;
}

static void pickup_think(Entity *self)
{
    /* nothing needed — collection handled in on_entity_collide */
}

static void pickup_update(Entity *self)
{
    PickupData *data;
    if (!self || !self->data) return;
    data = (PickupData *)self->data;

    /* gentle bob up and down */
    data->bobTimer += PICKUP_BOB_SPEED;
    self->position.y = data->baseY + sinf(data->bobTimer) * PICKUP_BOB_AMOUNT;
}

static void pickup_on_entity_collide(Entity *self, Entity *other)
{
    PickupData *data;
    Entity     *player;

    if (!self || !self->data || !other) return;
    player = player_entity_get();
    if (other != player) return;

    data = (PickupData *)self->data;
    slog("player collected: %s", pickup_names[data->type]);

    pickup_apply(data->type, player);
    entity_free(self);
}

/* ── Public API ──────────────────────────────────────────────────────── */

Entity *pickup_new(GFC_Vector2D position, PickupType type)
{
    Entity     *self;
    PickupData *data;

    if (type < 0 || type >= PT_COUNT)
    {
        slog("pickup_new: invalid type %i", type);
        return NULL;
    }

    self = entity_new();
    if (!self) { slog("pickup_new: entity_new failed"); return NULL; }

    data = gfc_allocate_array(sizeof(PickupData), 1);
    if (!data)
    {
        slog("pickup_new: data alloc failed");
        entity_free(self);
        return NULL;
    }

    data->type     = type;
    data->bobTimer = 0;
    data->baseY    = position.y;

    self->data            = data;
    self->position        = position;
    self->bounds          = gfc_rect(-8, -8, 32, 32);
    self->rotationCenter  = gfc_vector2d(16, 16);
    self->sprite          = gf2d_sprite_load_all(pickup_sprites[type],
                                                  64, 64, 1, 0);
    self->think           = pickup_think;
    self->update          = pickup_update;
    self->free            = pickup_free;
    self->onEntityCollide = pickup_on_entity_collide;
    self->scale          = gfc_vector2d(0.5f, 0.5f);

    return self;
}

void pickup_drop_random(GFC_Vector2D position)
{
    PickupType pool[] = {
        PT_HEALTH_PACK,
        PT_HEALTH_PACK,
        PT_WEAPON_UPGRADE,
        PT_WEAPON_UPGRADE,
        PT_WEAPON_UNLOCK,
        PT_MAX_HEALTH,
        PT_MOVE_SPEED,
    };
    int poolSize = sizeof(pool) / sizeof(pool[0]);
    PickupType chosen = pool[rand() % poolSize];
    pickup_new(position, chosen);
}