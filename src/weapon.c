#include "simple_logger.h"
#include "gf2d_sprite.h"
#include "gf2d_draw.h"
#include "gfc_vector.h"
#include "gfc_shape.h"
#include "entity.h"
#include "enemy.h"
#include "player.h"
#include "bullet.h"
#include "camera.h"
#include "weapon.h"
#include "math.h"

/* ═══════════════════════════════════════════════════════════════════════
   Internal helpers
   ═══════════════════════════════════════════════════════════════════════ */

/* Find the nearest enemy to a position within range.
   Returns NULL if none found. */
static Entity *weapon_nearest_enemy(GFC_Vector2D pos, float range,
                                    Entity *exclude)
{
    Entity  *all[1024];
    Entity  *nearest = NULL;
    float    bestDist = range * range;  /* compare squared to avoid sqrt   */
    int      count, i;
    GFC_Vector2D diff;

    count = entity_manager_get_all_active(all, 1024);
    for (i = 0; i < count; i++)
    {
        if (!all[i] || !all[i]->_inuse)   continue;
        if (!entity_is_enemy(all[i]))      continue;
        if (all[i] == exclude)             continue;

        gfc_vector2d_sub(diff, all[i]->position, pos);
        float d = diff.x * diff.x + diff.y * diff.y;
        if (d < bestDist)
        {
            bestDist = d;
            nearest  = all[i];
        }
    }
    return nearest;
}

/* Draw a line in screen space between two world positions */
static void draw_world_line(GFC_Vector2D a, GFC_Vector2D b, GFC_Color color)
{
    GFC_Vector2D offset = camera_get_offset();
    GFC_Vector2D sa = { a.x + offset.x, a.y + offset.y };
    GFC_Vector2D sb = { b.x + offset.x, b.y + offset.y };
    gf2d_draw_line(sa, sb, color);
}

/* ═══════════════════════════════════════════════════════════════════════
   Blade weapon
   ═══════════════════════════════════════════════════════════════════════ */

static void blade_think(Entity *self)
{
    BladeData    *data;
    Entity       *all[1024];
    int           count, i;
    GFC_Vector2D  diff;
    float         dist;

    if (!self || !self->data) return;
    data = (BladeData *)self->data;

    /* tick all per-enemy damage cooldowns */
    for (i = 0; i < 64; i++)
        if (data->damageCooldown[i] > 0) data->damageCooldown[i]--;
}

static void blade_update(Entity *self)
{
    BladeData    *data;
    Entity       *player;
    GFC_Vector2D  offset;

    if (!self || !self->data) return;
    data   = (BladeData *)self->data;
    player = player_entity_get();
    if (!player) return;

    /* advance angle */
    data->angle += data->speed;
    if (data->angle > M_PI * 2) data->angle -= M_PI * 2;

    /* orbit offset for this blade's slot */
    float slotAngle = data->angle + (data->orbitIndex * (M_PI * 2.0f / BLADE_MAX_COUNT));
    self->position.x = player->position.x + cosf(slotAngle) * BLADE_ORBIT_RADIUS;
    self->position.y = player->position.y + sinf(slotAngle) * BLADE_ORBIT_RADIUS;
    self->rotation   = slotAngle * (180.0f / M_PI);

}

static void blade_on_entity_collide(Entity *self, Entity *other)
{
    BladeData *data;
    int        slot;

    if (!self || !self->data || !other) return;
    if (!entity_is_enemy(other)) return;

    data = (BladeData *)self->data;

    /* use lower bits of pointer as a stable per-enemy slot index */
    slot = (int)((size_t)other / sizeof(Entity)) % 64;

    if (data->damageCooldown[slot] > 0) return;

    enemy_take_damage(other, BLADE_DAMAGE);
    data->damageCooldown[slot] = BLADE_DAMAGE_CD;
}

static void blade_free(Entity *self)
{
    if (!self) return;
    if (self->data) { free(self->data); self->data = NULL; }
    slog("Blade freed!");
    self->_inuse = 0;
}

static Entity *blade_new(int orbitIndex, float speed)
{
    Entity    *self;
    BladeData *data;

    self = entity_new();
    if (!self) { slog("blade_new: entity_new failed"); return NULL; }

    data = gfc_allocate_array(sizeof(BladeData), 1);
    if (!data) { entity_free(self); return NULL; }

    data->angle      = 0;
    data->speed      = speed;
    data->orbitIndex = orbitIndex;

    self->data           = data;
    self->bounds         = gfc_rect(-12, -12, 24, 24);
    self->rotationCenter = gfc_vector2d(16, 16);
    self->sprite         = gf2d_sprite_load_all("../images/blade.png", 32, 32, 1, 0);
    self->think          = blade_think;
    self->update         = blade_update;
    self->free           = blade_free;
    self->onEntityCollide = blade_on_entity_collide;

    return self;
}

/* ═══════════════════════════════════════════════════════════════════════
   Cardinal bullets
   ═══════════════════════════════════════════════════════════════════════ */

static void weapon_fire_cardinal(Entity *player)
{
    GFC_Vector2D dirs[4] = {
        {  0, -1 },   /* N */
        {  0,  1 },   /* S */
        {  1,  0 },   /* E */
        { -1,  0 }    /* W */
    };
    int i;
    for (i = 0; i < 4; i++)
        bullet_new(player->position, dirs[i], CARDINAL_DAMAGE);
}

/* ═══════════════════════════════════════════════════════════════════════
   Laser
   ═══════════════════════════════════════════════════════════════════════ */

typedef struct
{
    GFC_Vector2D from;
    GFC_Vector2D to;
    int          timer;
    int          active;
} LaserBeam;

static LaserBeam gLaser = {0};  /* one laser beam at a time */

static void weapon_fire_laser(Entity *player)
{
    Entity       *target;
    GFC_Vector2D  dir;

    target = weapon_nearest_enemy(player->position, LASER_RANGE, NULL);
    if (!target) return;

    enemy_take_damage(target, LASER_DAMAGE);

    gLaser.from   = player->position;
    gLaser.to     = target->position;
    gLaser.timer  = LASER_BURST_DURATION;
    gLaser.active = 1;
}

/* ═══════════════════════════════════════════════════════════════════════
   Lightning
   ═══════════════════════════════════════════════════════════════════════ */

#define MAX_ARCS (LIGHTNING_CHAINS + 1)

typedef struct
{
    GFC_Vector2D from;
    GFC_Vector2D to;
    int          timer;
} LightningArc;

static LightningArc gArcs[MAX_ARCS];
static int          gArcCount = 0;

static void weapon_fire_lightning(Entity *player)
{
    Entity       *hit[LIGHTNING_CHAINS];
    Entity       *last;
    GFC_Vector2D  lastPos;
    int           i, found = 0;

    gArcCount = 0;
    last      = NULL;
    lastPos   = player->position;

    for (i = 0; i < LIGHTNING_CHAINS; i++)
    {
        Entity *next = weapon_nearest_enemy(lastPos, LIGHTNING_RANGE, last);
        if (!next) break;

        /* check not already hit */
        int dup = 0, k;
        for (k = 0; k < found; k++)
            if (hit[k] == next) { dup = 1; break; }
        if (dup) break;

        enemy_take_damage(next, LIGHTNING_DAMAGE);

        /* store arc for drawing */
        if (gArcCount < MAX_ARCS)
        {
            gArcs[gArcCount].from  = lastPos;
            gArcs[gArcCount].to    = next->position;
            gArcs[gArcCount].timer = LIGHTNING_DURATION;
            gArcCount++;
        }

        hit[found++] = next;
        last         = next;
        lastPos      = next->position;
    }
}

/* ═══════════════════════════════════════════════════════════════════════
   Public API
   ═══════════════════════════════════════════════════════════════════════ */

void weapon_system_init(WeaponSystem *ws)
{
    if (!ws) return;
    ws->bladeCount    = 0;
    ws->bladeSpeed    = BLADE_BASE_SPEED * 0.016f; /* convert to per-frame */
    ws->cardinalTimer = 0;
    ws->laserTimer    = 0;
    ws->laserFired    = 0;
    ws->lightningTimer = 0;
}

void weapon_unlock(int *hasWeapon, WeaponSystem *ws, int slot)
{
    if (!hasWeapon || !ws) return;
    if (slot < 0 || slot >= WEAPON_COUNT) return;
    hasWeapon[slot] = 1;

    /* initialise weapon-specific state on first unlock */
    if (slot == WEAPON_BLADE && ws->bladeCount == 0)
    {
        ws->bladeCount = BLADE_BASE_COUNT;
        /* blades are spawned in weapon_system_update on first frame */
    }
    slog("weapon slot %i unlocked", slot);
}

void weapon_blade_upgrade(WeaponSystem *ws, Entity *player)
{
    if (!ws || !player) return;
    if (ws->bladeCount >= BLADE_MAX_COUNT)
    {
        /* max blades — increase speed instead */
        ws->bladeSpeed *= 1.05f;
        int i;
        for (i = 0; i < ws->bladeCount; i++)
        {
            if (ws->blades[i] && ws->blades[i]->_inuse)
            {
                BladeData *bd = (BladeData *)ws->blades[i]->data;
                if (bd) bd->speed = ws->bladeSpeed;
            }
        }
        slog("blade speed upgraded to %f", ws->bladeSpeed);
        return;
    }

    /* add a new blade */
    ws->blades[ws->bladeCount] = blade_new(ws->bladeCount, ws->bladeSpeed);
    ws->bladeCount++;
    slog("blade count upgraded to %i", ws->bladeCount);
}

void weapon_system_update(WeaponSystem *ws, Entity *player, int *hasWeapon)
{
    int i;
    if (!ws || !player || !hasWeapon) return;

    /* ── Blades ──────────────────────────────────────────────────────── */
    if (hasWeapon[WEAPON_BLADE])
    {

        for (i = 0; i < ws->bladeCount; i++)
        {
            if (!ws->blades[i] || !ws->blades[i]->_inuse)
                ws->blades[i] = blade_new(i, ws->bladeSpeed);
        }
    } 


    /* ── Cardinal bullets ────────────────────────────────────────────── */
    if (hasWeapon[WEAPON_CARDINAL])
    {
        ws->cardinalTimer--;
        if (ws->cardinalTimer <= 0)
        {
            weapon_fire_cardinal(player);
            ws->cardinalTimer = CARDINAL_FIRE_RATE;
        }
    }

    /* ── Laser ───────────────────────────────────────────────────────── */
    if (hasWeapon[WEAPON_LASER])
    {
        ws->laserTimer--;
        if (ws->laserTimer <= 0)
        {
            if (!ws->laserFired)
            {
                weapon_fire_laser(player);
                ws->laserFired = 1;
            }
            /* reset after burst duration */
            if (ws->laserTimer <= -LASER_BURST_DURATION)
            {
                ws->laserTimer = LASER_COOLDOWN;
                ws->laserFired = 0;
            }
        }
        /* tick beam visibility */
        if (gLaser.active && gLaser.timer > 0)
            gLaser.timer--;
        else
            gLaser.active = 0;
    }

    /* ── Lightning ───────────────────────────────────────────────────── */
    if (hasWeapon[WEAPON_LIGHTNING])
    {
        ws->lightningTimer--;
        if (ws->lightningTimer <= 0)
        {
            weapon_fire_lightning(player);
            ws->lightningTimer = LIGHTNING_COOLDOWN;
        }
        /* tick arc visibility */
        for (i = 0; i < gArcCount; i++)
            if (gArcs[i].timer > 0) gArcs[i].timer--;
    }
}

void weapon_system_draw(WeaponSystem *ws, Entity *player)
{
    int i, t;
    if (!ws || !player) return;

    /* ── Laser beam ──────────────────────────────────────────────────── */
    if (gLaser.active && gLaser.timer > 0)
    {
        float alpha = (float)gLaser.timer / (float)LASER_BURST_DURATION;
        GFC_Color laserCore  = gfc_color(1.0f, 0.3f, 0.3f, alpha);        /* bright red center  */
        GFC_Color laserOuter = gfc_color(0.8f, 0.1f, 0.1f, alpha * 0.6f); /* darker red edges   */

        GFC_Vector2D offset = camera_get_offset();
        GFC_Vector2D from = { gLaser.from.x + offset.x, gLaser.from.y + offset.y };
        GFC_Vector2D to   = { gLaser.to.x   + offset.x, gLaser.to.y   + offset.y };

        /* 7 lines total: 3 outer, 1 core, 3 outer */
        for (t = -3; t <= 3; t++)
        {
            GFC_Vector2D f = { from.x + t, from.y };
            GFC_Vector2D e = { to.x   + t, to.y   };
            gf2d_draw_line(f, e, (t == 0) ? laserCore : laserOuter);
        }
        /* second pass — offset on y axis for a rounder beam */
        for (t = -2; t <= 2; t++)
        {
            GFC_Vector2D f = { from.x, from.y + t };
            GFC_Vector2D e = { to.x,   to.y   + t };
            gf2d_draw_line(f, e, (t == 0) ? laserCore : laserOuter);
        }
    }

    /* ── Lightning arcs ──────────────────────────────────────────────── */
    for (i = 0; i < gArcCount; i++)
    {
        if (gArcs[i].timer <= 0) continue;

        float alpha = (float)gArcs[i].timer / (float)LIGHTNING_DURATION;
        GFC_Color boltCore  = gfc_color(0.9f, 0.9f, 1.0f, alpha);        /* white core         */
        GFC_Color boltOuter = gfc_color(0.4f, 0.4f, 1.0f, alpha * 0.6f); /* blue edges         */

        GFC_Vector2D a = gArcs[i].from;
        GFC_Vector2D b = gArcs[i].to;

        /* jagged midpoints — recalculated each frame for flicker */
        GFC_Vector2D mid1, mid2;
        mid1.x = a.x + (b.x - a.x) * 0.33f + (float)((rand() % 24) - 12);
        mid1.y = a.y + (b.y - a.y) * 0.33f + (float)((rand() % 24) - 12);
        mid2.x = a.x + (b.x - a.x) * 0.66f + (float)((rand() % 24) - 12);
        mid2.y = a.y + (b.y - a.y) * 0.66f + (float)((rand() % 24) - 12);

        /* draw each segment 5 lines thick */
        for (t = -2; t <= 2; t++)
        {
            GFC_Color c = (t == 0) ? boltCore : boltOuter;
            GFC_Vector2D sa = { a.x    + camera_get_offset().x + t, a.y    + camera_get_offset().y };
            GFC_Vector2D sm1= { mid1.x + camera_get_offset().x + t, mid1.y + camera_get_offset().y };
            GFC_Vector2D sm2= { mid2.x + camera_get_offset().x + t, mid2.y + camera_get_offset().y };
            GFC_Vector2D sb = { b.x    + camera_get_offset().x + t, b.y    + camera_get_offset().y };
            gf2d_draw_line(sa,  sm1, c);
            gf2d_draw_line(sm1, sm2, c);
            gf2d_draw_line(sm2, sb,  c);
        }
    }
}