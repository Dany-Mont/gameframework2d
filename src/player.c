#include "simple_logger.h"
#include "gf2d_sprite.h"

#include "gfc_shape.h"


#include "camera.h"
#include "player.h"
#include "math.h"
#include "enemy.h"
#include "bullet.h"
#include "weapon.h"

Entity *thePlayer = NULL;




Entity *player_entity_get()
{
    if (!thePlayer)
    {
        slog("player entity has not been created yet");
        return NULL;
    }
    return thePlayer;
}
void player_update(Entity *self)
{
    
    if (!self)return;
    PlayerData *data = (PlayerData *)self->data;
    if (self->data)
    {
        if( data->damageCooldown > 0) data->damageCooldown--;
        if (data->fireTimer > 0)      data->fireTimer--;
    }
    self -> frame += 0.1;
    if (self -> frame >= 16)self -> frame = 0;

    gfc_vector2d_add(self ->position,self ->position,self ->velocity);

    camera_center_on(self ->position);
}  
void player_free(Entity *self)
{
    if (!self)return;
    if(self->data)
    {
        free(self->data);
        self->data = NULL;
    }
    self->_inuse = 0;
}
void player_think(Entity *self)
{
    GFC_Vector2D screen;
    GFC_Vector2D dir = {0,0};
    GFC_Vector2D shootDir = {0,0};
    PlayerData *data;
    int mx = 0, my = 0;
    if (!self || !self->data)return;
    data = (PlayerData *)self->data;   
    screen = camera_get_position();
    SDL_GetMouseState(&mx,&my);
    mx += screen.x;
    my += screen.y;
    if(self ->position.x<mx)dir.x = 1;
    if(self ->position.x>mx)dir.x = -1;
    if(self ->position.y<my)dir.y = 1;
    if(self ->position.y>my)dir.y = -1;
    gfc_vector2d_normalize(&dir);
    gfc_vector2d_scale(self->velocity, dir, data->moveSpeed);

     float dx = mx - self->position.x;
    float dy = my - self->position.y;

    self->rotation = atan2(dy, dx) + (M_PI / 2);
    self->rotation *= (180.0 / M_PI); 
        if (data->fireTimer <= 0)
    {
        shootDir.x = dx;
        shootDir.y = dy;
        gfc_vector2d_normalize(&shootDir);
        bullet_new(self->position, shootDir, (int)(25 * data->damageMult));
        data->fireTimer = (int)(BULLET_FIRE_RATE * data->fireRateMult);
    }
    weapon_system_update(&data->weapons, self, data->hasWeapon);
}
    
Entity *player_new()
{
    Entity *self;
    PlayerData *data;
    
    self = entity_new();
    if (!self)
    {
            slog("failed to create player entity");
            return NULL;
    }
        data = gfc_allocate_array(sizeof(PlayerData), 1);
    if (!data)
    {
        slog("failed to allocate player data");
        entity_free(self);
        return NULL;
    }
    self->data = data;

    data->health    = 100;
    data->maxHealth = 100;
    data->damageCooldown = 0;
    data->damageMult   = 1.0f;
    data->fireRateMult = 1.0f;
    data->moveSpeed    = 3.0f;
    weapon_system_init(&data->weapons);
    data->hasWeapon[WEAPON_BULLET] = 1;
 
    
    

    self ->sprite = gf2d_sprite_load_all("../images/ed210_top.png",128,128,16,0);
    self ->frame = 1;
    self ->position = gfc_vector2d(200,200);
    self ->think = player_think;
    self ->update = player_update;
    self ->free = player_free;
    self ->onEntityCollide = player_on_entity_collide;
    self ->rotationCenter = gfc_vector2d(64,64);
    self->bounds = gfc_rect(-40, -40, 80, 80);

    thePlayer = self;
    return self;
}
void player_take_damage(Entity *self, int amount)
{
    PlayerData *data;
    if (!self || !self->data) return;
    data = (PlayerData *)self->data;
    if (data->damageCooldown > 0) return;
    data->damageCooldown = 60;
    data->health -= amount;
    slog("player took %i damage, health is now %i", amount, data->health);
    if (data->health <= 0)
    {
        slog("player has died");
        data->health = 0;
        /* TODO: handle player death */
        thePlayer = NULL;
        player_free(self);
        entity_free(self);
        exit(0);
    }
}


void player_on_entity_collide(Entity *a, Entity *b)
{
    Entity *player = player_entity_get();

    if (a == player)
    {
        /* only take damage from enemies, not bullets */
        if (b->free == enemy_free)
            player_take_damage(a, ((EnemyData *)b->data)->damage);
        return;
    }
    if (b == player)
    {
        if (a->free == enemy_free)
            player_take_damage(b, ((EnemyData *)a->data)->damage);
        return;
    }
}
int player_get_health(Entity *self)
{
    PlayerData *data;
    if (!self || !self->data) return 0;
    data = (PlayerData *)self->data;
    return data->health;
}

int player_get_max_health(Entity *self)
{
    PlayerData *data;
    if (!self || !self->data) return 0;
    data = (PlayerData *)self->data;
    return data->maxHealth;
}
void player_unlock_weapon(Entity *self, int slot)
{
    PlayerData *data;
    if (!self || !self->data) return;
    data = (PlayerData *)self->data;
    weapon_unlock(data->hasWeapon, &data->weapons, slot);
}
 
void player_upgrade_blades(Entity *self)
{
    PlayerData *data;
    if (!self || !self->data) return;
    data = (PlayerData *)self->data;
    weapon_blade_upgrade(&data->weapons, self);
}