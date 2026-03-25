#ifndef __ENTITY_H__
#define __ENTITY_H__

#include <SDL.h>
#include "gfc_text.h"
#include "gfc_shape.h"
#include "gf2d_sprite.h"


typedef struct Entity_S
{
    Uint8   _inuse;                                                     //no touchy
    GFC_TextLine    name;                                               //name of the entity for debugging purposes
    GFC_Vector2D    position;
    GFC_Vector2D    scale;
    GFC_Vector2D    velocity;
    GFC_Vector2D    rotationCenter;
    float           topSpeed;
    float           rotation;
    Sprite          *sprite;
    float           frame;
    void            (*think) (struct Entity_S *self);                   //called every frame if defined for the entity
    void            (*update) (struct Entity_S *self);                  //called every frame if defined for the entity
    void            (*free) (struct Entity_S *self);                    //called when the entity is freed
    void            (*onTileCollide)(struct Entity_S *self, GFC_Vector2D normal);
    void            (*onEntityCollide)(struct Entity_S *self, struct Entity_S *other);
    void            *data; 
    GFC_Rect bounds; // local offset hitbox, relative to position                                              //used for entity specific data

}Entity;


/**
 * @brief initialize the entity sub system
 * @
 *
 */
void entity_manager_init(Uint32 max);

/**
 * @brief clean up all active entities
 * @param ignore do not clean up this entity
 */
void entity_clear_all(Entity *ignore);

/**
 *  @brief get a pointer to a free entity
 * @return NULL if out of entities, a pointer to a blank entity otherwise
 */
Entity *entity_new();

/**
 * @brief draw all active entities
 */
void entity_draw_all();


/**
 * @brief run the think functions for all active entities
 */
void entity_manager_think_all();

void entity_think(Entity *self);
void entity_draw(Entity *self);
void entity_update(Entity *self);

/**
 * @brief run the update functions for all active entities
 */
void entity_manager_update_all();

/**
 * @brief free an entity
 * @param self the entity to free
 * @note do not use the memory again after the call
 */
void entity_free(Entity *self);

/**
 * @brief get all active entities
 * @param results array to fill with active entity pointers
 * @param maxResults size of the results array
 * @return number of active entities found
 */
int entity_manager_get_all_active(Entity **results, int maxResults);


void entity_on_tile_collide(Entity *self, GFC_Vector2D normal);



#endif
