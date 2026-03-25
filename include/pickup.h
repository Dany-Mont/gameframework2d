#ifndef __PICKUP_H__
#define __PICKUP_H__

#include "gfc_vector.h"
#include "entity.h"

/* ── Pickup types ────────────────────────────────────────────────────── */
typedef enum
{
    PT_WEAPON_UNLOCK,       /* unlocks a random weapon the player lacks    */
    PT_HEALTH_PACK,         /* restores 15% of max health                  */
    PT_WEAPON_UPGRADE,      /* +25% damage and fire rate to all weapons    */
    PT_MAX_HEALTH,          /* increases max health by 20                  */
    PT_MOVE_SPEED,          /* increases movement speed by 0.5             */
    PT_COUNT
} PickupType;

/* ── Tuning ──────────────────────────────────────────────────────────── */
#define PICKUP_HEALTH_PERCENT   0.15f   /* 15% of max health restored      */
#define PICKUP_MAX_HEALTH_BONUS 10      /* flat max health increase         */
#define PICKUP_SPEED_BONUS      0.1f    /* flat speed increase              */
#define PICKUP_DAMAGE_MULT      1.05f   /* 25% damage boost                 */
#define PICKUP_FIRERATE_MULT    0.95f   /* 25% faster fire (multiply timer) */
#define PICKUP_BOB_SPEED        0.08f   /* visual bob animation speed       */
#define PICKUP_BOB_AMOUNT       4.0f    /* pixels to bob up/down            */

typedef struct
{
    PickupType  type;
    float       bobTimer;   /* for visual bobbing effect                   */
    float       baseY;      /* world Y position before bob offset          */
} PickupData;

/* ── API ─────────────────────────────────────────────────────────────── */

/**
 * @brief spawn a pickup of a specific type at a world position
 * @param position  world position to spawn at
 * @param type      which pickup type to spawn
 * @return pointer to the pickup entity, or NULL on failure
 */
Entity *pickup_new(GFC_Vector2D position, PickupType type);

/**
 * @brief spawn a random pickup at a world position
 *        used by the enemy onDeath hook
 * @param position  world position to spawn at
 */
void pickup_drop_random(GFC_Vector2D position);

void pickup_free(Entity *self);

#endif /* __PICKUP_H__ */
