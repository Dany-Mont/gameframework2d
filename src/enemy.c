#include "simple_logger.h"
#include "enemy.h"

int enemy_take_damage(Entity *self, int amount)
{
    EnemyData *data;
    int remaining = 0;

    if (!self || !self->data) return 0;
    data = (EnemyData *)self->data;

    if (data->isShielded)
    {
        //slog("enemy is shielded — damage blocked");
        return 0;
    }

    if (amount >= data->health)
    {
        remaining = amount - data->health;
        slog("enemy killed, %i damage remaining", remaining);

        if (data->onDeath)
            data->onDeath(self->position);

        entity_free(self);
        return remaining;
    }

    data->health -= amount;
    slog("enemy took %i damage, health now %i", amount, data->health);
    return 0;
}

/* ── entity_is_enemy ─────────────────────────────────────────────────────
 * Checks the free pointer since every enemy type uses enemy_free.        */
int entity_is_enemy(Entity *e)
{
    if (!e || !e->_inuse || !e->data) return 0;
    /* all enemies have an EnemyData with a damage field —
       check that the update pointer is one of the known enemy updaters     */
    return (e->free == enemy_free|| e->free == mage_free);
}

/* ── Shared free ─────────────────────────────────────────────────────────
 * Frees typeData (if any) then the EnemyData block itself.               */
void enemy_free(Entity *self)
{
    EnemyData *data;
    if (!self) return;
    if (self->data)
    {
        data = (EnemyData *)self->data;
        if (data->typeData)
        {
            free(data->typeData);
            data->typeData = NULL;
        }
        free(self->data);
        self->data = NULL;
    }
    self->_inuse = 0;
}

void enemy_apply_shields_in_radius(GFC_Vector2D position, float radius,
                                   Entity **entities, int count)
{
    int i;
    EnemyData *data;
    GFC_Vector2D diff;
    float dist;

    for (i = 0; i < count; i++)
    {
        if (!entities[i] || !entities[i]->_inuse) continue;
        if (!entity_is_enemy(entities[i]))        continue;
        if (entities[i]->free == mage_free) continue; 

        gfc_vector2d_sub(diff, entities[i]->position, position);
        dist = gfc_vector2d_magnitude(diff);
        if (dist > radius) continue;

        data = (EnemyData *)entities[i]->data;
        data->isShielded = 1;
    }
}

void enemy_remove_shields_in_radius(GFC_Vector2D position, float radius,
                                    Entity **entities, int count)
{
    int i;
    EnemyData *data;
    GFC_Vector2D diff;
    float dist;

    for (i = 0; i < count; i++)
    {
        if (!entities[i] || !entities[i]->_inuse) continue;
        if (!entity_is_enemy(entities[i]))        continue;

        gfc_vector2d_sub(diff, entities[i]->position, position);
        dist = gfc_vector2d_magnitude(diff);
        //if (dist > radius) continue;

        data = (EnemyData *)entities[i]->data;
        data->isShielded = 0;
    }
}
void enemy_separate(Entity *self, float radius)
{
    Entity      *all[1024];
    int          count, i;
    GFC_Vector2D diff, push;
    float        dist;

    if (!self) return;
    count = entity_manager_get_all_active(all, 1024);

    for (i = 0; i < count; i++)
    {
        if (!all[i] || !all[i]->_inuse) continue;
        if (!entity_is_enemy(all[i]))   continue;
        if (all[i] == self)             continue;

        gfc_vector2d_sub(diff, self->position, all[i]->position);
        dist = gfc_vector2d_magnitude(diff);

        if (dist < radius && dist > 0.001f)
        {
            /* push directly away, stronger the closer they are */
            gfc_vector2d_normalize(&diff);
            gfc_vector2d_scale(push, diff, (radius - dist) * 0.5f);
            gfc_vector2d_add(self->position, self->position, push);
        }
    }
}