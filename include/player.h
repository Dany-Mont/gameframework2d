#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "entity.h"
#include "weapon.h"

/**
 * @brief spawn a player
 * @return NULL if the player entity has not been created, a pointer to the player entity otherwise
 */
Entity *player_new();

typedef struct
{
    int          health;
    int          maxHealth;
    int          damageCooldown;
    int          fireTimer;
    float        damageMult;
    float        fireRateMult;
    float        moveSpeed;

    /* weapon system */
    int          hasWeapon[WEAPON_COUNT];   /* 1 = unlocked, 0 = locked     */
    WeaponSystem weapons;
} PlayerData;


void player_update(Entity *self);
void player_free(Entity *self);
void player_think(Entity *self);

/**
 * @brief apply damage to the player
 * @param amount how much damage to deal
 */
void player_take_damage(Entity *self, int amount);

Entity *player_entity_get();

void player_on_entity_collide(Entity *a, Entity *b);

int player_get_health(Entity *self);
int player_get_max_health(Entity *self);

/**
 * @brief unlock a weapon for the player
 * @param self  the player entity
 * @param slot  WEAPON_* constant from weapon.h
 */
void player_unlock_weapon(Entity *self, int slot);

/**
 * @brief upgrade the blade weapon
 * @param self the player entity
 */
void player_upgrade_blades(Entity *self);

#endif
