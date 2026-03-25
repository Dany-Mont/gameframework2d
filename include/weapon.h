#ifndef __WEAPON_H__
#define __WEAPON_H__

#include "gfc_vector.h"
#include "gfc_color.h"
#include "entity.h"

#define WEAPON_BULLET       0   
#define WEAPON_BLADE        1  
#define WEAPON_CARDINAL     2 
#define WEAPON_LASER        3  
#define WEAPON_LIGHTNING    4 
#define WEAPON_COUNT        5


#define BLADE_ORBIT_RADIUS      120.0f
#define BLADE_BASE_COUNT        1
#define BLADE_MAX_COUNT         4
#define BLADE_BASE_SPEED        3.0f    
#define BLADE_DAMAGE            20
#define BLADE_DAMAGE_CD         30   

/* ── Cardinal bullet config ──────────────────────────────────────────── */
#define CARDINAL_FIRE_RATE      90      /* frames between bursts            */
#define CARDINAL_DAMAGE         15

/* ── Laser config ────────────────────────────────────────────────────── */
#define LASER_RANGE             400.0f
#define LASER_DAMAGE            30
#define LASER_BURST_DURATION    20      /* frames beam is visible           */
#define LASER_COOLDOWN          120     /* frames between bursts            */

/* ── Lightning config ────────────────────────────────────────────────── */
#define LIGHTNING_CHAINS        3       /* enemies hit per cast             */
#define LIGHTNING_RANGE         300.0f  /* max jump distance                */
#define LIGHTNING_DAMAGE        25
#define LIGHTNING_COOLDOWN      180     /* frames between casts             */
#define LIGHTNING_DURATION      10      /* frames arc is visible            */

/* ── Blade entity data ───────────────────────────────────────────────── */
typedef struct
{
    float   angle;          /* current orbit angle in radians               */
    float   speed;          /* radians per frame                            */
    int     orbitIndex;     /* which blade slot (0..BLADE_MAX_COUNT-1)      */
    int     damageCooldown[64]; /* per-enemy cooldown tracked by slot       */
} BladeData;

/* ── Lightning arc (visual only entity) ─────────────────────────────── */
typedef struct
{
    GFC_Vector2D from;
    GFC_Vector2D to;
    int          timer;     /* frames remaining to display                  */
} LightningArcData;

/* ── Per-weapon timer state (stored in WeaponSystem) ────────────────── */
typedef struct
{
    /* blade */
    int     bladeCount;         /* how many blades are orbiting             */
    float   bladeSpeed;         /* current orbit speed                      */
    Entity *blades[BLADE_MAX_COUNT];

    /* cardinal */
    int     cardinalTimer;

    /* laser */
    int     laserTimer;         /* counts up: 0..COOLDOWN = cooldown,
                                   then COOLDOWN..COOLDOWN+BURST = firing  */
    int     laserFired;         /* 1 if damage already dealt this burst     */

    /* lightning */
    int     lightningTimer;
} WeaponSystem;

/* ── API ─────────────────────────────────────────────────────────────── */

/**
 * @brief initialise a WeaponSystem to defaults
 */
void weapon_system_init(WeaponSystem *ws);

/**
 * @brief update all active weapons each frame
 * @param ws        the weapon system
 * @param player    the player entity
 * @param hasWeapon array of WEAPON_COUNT flags from PlayerData
 */
void weapon_system_update(WeaponSystem *ws, Entity *player, int *hasWeapon);

/**
 * @brief draw laser beam and lightning arcs (call in render section)
 * @param ws     the weapon system
 * @param player the player entity
 */
void weapon_system_draw(WeaponSystem *ws, Entity *player);

/**
 * @brief give the player a weapon slot
 * @param hasWeapon array of WEAPON_COUNT flags from PlayerData
 * @param ws        weapon system to initialise new weapon state in
 * @param slot      WEAPON_* constant
 */
void weapon_unlock(int *hasWeapon, WeaponSystem *ws, int slot);

/**
 * @brief upgrade the blade weapon (more blades, faster spin)
 * @param ws     the weapon system
 * @param player the player entity
 */
void weapon_blade_upgrade(WeaponSystem *ws, Entity *player);

#endif /* __WEAPON_H__ */