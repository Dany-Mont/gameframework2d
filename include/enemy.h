#ifndef __ENEMY_H__
#define __ENEMY_H__

#include "gfc_vector.h"
#include "gfc_color.h"
#include "entity.h"

#define MAGE_SHIELD_RADIUS      200.0f  
#define SUMMONER_SPAWN_COOLDOWN 300
#define SUMMONER_MAX_ACTIVE     10

typedef struct
{
    int     health;
    int     maxHealth;
    int     damage;
    int     isShielded;     /* 1 = immune to damage, svisually highlighted   */
    void   *typeData;       /* points to Grunt/Tank/Ranger/Mage/SummonerData */

    /**
     * Optional death callback — set this to trigger a drop or effect.
     * Called automatically by enemy_take_damage just before the entity
     * is freed.  position is the enemy's world position at time of death.
     *
     * Example usage (in a future pickup.c):
     *   data->onDeath = drop_random_pickup;
     */
    void  (*onDeath)(GFC_Vector2D position);
} EnemyData;

typedef struct
{
    int damageCooldown;
} GruntData;

typedef struct
{
    int damageCooldown;
} TankData;

typedef struct
{
    float preferredRange;   /* distance ranger tries to maintain from player */
    int   fireTimer;        /* countdown between shots                        */
    int   fireRate;         /* frames between shots                           */
} RangerData;

typedef struct
{
    int   shieldPulseTimer; /* visual pulse tick                              */
    float radius;           /* shield radius                                  */
} MageData;

typedef struct
{
    int      spawnCooldown;         /* frames until next spawn attempt        */
    int      activeGrunts;          /* how many spawned grunts are alive      */
    Entity  *grunts[SUMMONER_MAX_ACTIVE]; /* tracked grunt pointers           */
} SummonerData;


/**
 * @brief deal damage to an enemy, respecting isShielded
 * @param self   the enemy entity
 * @param amount how much damage to deal
 * @return remaining damage after killing the enemy (0 if enemy survived)
 */
int enemy_take_damage(Entity *self, int amount);

/**
 * @brief check if an entity is any kind of enemy
 *        (compares update pointer — all enemies share enemy_update_base)
 */
int entity_is_enemy(Entity *e);

/**
 * @brief shared free — frees typeData then EnemyData
 */
void enemy_free(Entity *self);

/**
 * @brief apply a shield to all enemies within radius of position
 */
void enemy_apply_shields_in_radius(GFC_Vector2D position, float radius, Entity **entities, int count);

/**
 * @brief remove shields from all enemies previously shielded by this mage
 */
void enemy_remove_shields_in_radius(GFC_Vector2D position, float radius,Entity **entities, int count);

Entity *grunt_new(GFC_Vector2D position);


Entity *tank_new(GFC_Vector2D position);


Entity *ranger_new(GFC_Vector2D position);

Entity *mage_new(GFC_Vector2D position);

Entity *summoner_new(GFC_Vector2D position);

void mage_free(Entity *self);

void mage_draw_shield(Entity *self);

/**
 * @brief push this enemy away from other nearby enemies
 * @param self   the enemy entity
 * @param radius separation distance in pixels
 */
void enemy_separate(Entity *self, float radius);

#endif
