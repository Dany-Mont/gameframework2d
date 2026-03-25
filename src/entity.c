#include "simple_logger.h"



#include "gf2d_sprite.h"
#include "gf2d_graphics.h"


#include "camera.h"
#include "entity.h"


typedef struct
{
    Entity *entityList;
    Uint32 entityMax;

}EntityManager;


static EntityManager entityManager = {0};


void entity_manager_init(Uint32 max)
{
    if (entityManager.entityList)
    {
        slog("entity system already initialized");
        return;
    }
    if (!max)
    {
        slog("cannot initialize entity system with zero entities");
        return;
    }
    entityManager.entityList = gfc_allocate_array(sizeof(Entity),max);
    if (!entityManager.entityList)
    {
        slog("failed to allocate %i entities",max);
        return;
    }
    entityManager.entityMax = max;
   
    slog("Initialized entity system");
}

void entity_manager_close()
{
    entity_clear_all(NULL);
    if (entityManager.entityList)free(entityManager.entityList);
    memset(&entityManager,0,sizeof(EntityManager));
}

Entity *entity_new()
{
    int i;
    if (!entityManager.entityList)
    {
        slog ("entity system has not been initialized!");
        return NULL;
    }
    for (i = 0; i < entityManager.entityMax;i++)
    {
        if(entityManager.entityList[i]._inuse)continue;
        memset(&entityManager.entityList[i],0,sizeof(Entity));
        entityManager.entityList[i]._inuse = 1;
        //set defaults
        entityManager.entityList[i].scale.x = 1;
        entityManager.entityList[i].scale.y = 1;
        return &entityManager.entityList[i];
    }
    slog("no free entities");
    return NULL;
}

void entity_clear_all(Entity *ignore)
{
    int i;
    for (i=0;i < entityManager.entityMax;i++)
    {
        if(&entityManager.entityList[i] == ignore)continue;
        if (!entityManager.entityList[i]._inuse)continue;
        entity_free(&entityManager.entityList[i]);
    }
}

void entity_free(Entity *self)
{
    if (!self) return;
    gf2d_sprite_free(self->sprite);
    self->sprite = NULL;
    if (self->free) self->free(self);
    self->_inuse = 0;
}
void entity_draw(Entity *self)
{
    GFC_Vector2D offset, position;
    if (!self)return;
    offset = camera_get_offset();
    position.x = self ->position.x + offset.x;
    position.y = self ->position.y + offset.y;
    gf2d_sprite_draw(
        self ->sprite,
        position,
        &self ->scale,
        &self ->rotationCenter,
        &self ->rotation,
        NULL,
        NULL,
        (Uint32)self ->frame);
}
void entity_think(Entity *self)
{
    if (!self)return;
    if (self ->think)self ->think(self);
}
void entity_manager_think_all()
{
    int i;
    for (i = 0; i < entityManager.entityMax;i++)
    {
        if(!entityManager.entityList[i]._inuse)continue;
        entity_think(&entityManager.entityList[i]);
    }

}
void entity_update(Entity *self)
{
    if (!self)return;
    if (self ->update)self ->update(self);
}
void entity_manager_update_all()
{
    int i;
    for (i = 0; i < entityManager.entityMax;i++)
    {
        if(!entityManager.entityList[i]._inuse)continue;
        entity_update(&entityManager.entityList[i]);
    }
}

void entity_draw_all()
{   
    int i;
    for (i = 0; i < entityManager.entityMax;i++)
    {
        if(!entityManager.entityList[i]._inuse)continue;
        entity_draw(&entityManager.entityList[i]);
    }
}

int entity_manager_get_all_active(Entity **results, int maxResults)
{
    int i, count = 0;
    for (i = 0; i < entityManager.entityMax && count < maxResults; i++)
    {
        if (!entityManager.entityList[i]._inuse) continue;
        results[count++] = &entityManager.entityList[i];
    }
    return count;
    
}


void entity_on_tile_collide(Entity *self, GFC_Vector2D normal)
{
    if (!self) return;
    if (self->onTileCollide)
        self->onTileCollide(self, normal);
}
