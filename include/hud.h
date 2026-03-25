#ifndef __HUD_H__
#define __HUD_H__

#include "gfc_color.h"
#include "gfc_shape.h"
#include "gfc_string.h"
#include "gf2d_draw.h"
#include "gf2d_graphics.h"
#include "font.h"


#define HUD_MARGIN      12    
#define HUD_BAR_W       160   
#define HUD_BAR_H       20      

typedef struct
{
    float   timeSurvived;      
    Uint32  lastTick;         
} HUD;


/**
 * hud_init  –  Reset the HUD state. Call once after font_init().
 * @param hud  Pointer to your HUD instance.
 */
void hud_init(HUD *hud);

/**
 * hud_update  –  Advance the survival timer using real elapsed time.
 *                Call once per frame BEFORE hud_render.
 * @param hud  Pointer to your HUD instance.
 */
void hud_update(HUD *hud);

/**
 * hud_render  –  Draw the health bar and timer onto the screen.
 * @param hud        Pointer to your HUD instance.
 * @param health     Player's current health  (>= 0).
 * @param maxHealth  Player's maximum health  (> 0).
 */
void hud_render(HUD *hud, int health, int maxHealth);

#endif
