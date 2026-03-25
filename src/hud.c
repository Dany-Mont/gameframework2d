#include "hud.h"
#include "simple_logger.h"
#include "gfc_string.h"

void hud_init(HUD *hud)
{
    if (!hud)
    {
        slog("hud_init: NULL hud pointer");
        return;
    }
    hud->timeSurvived = 0;
    hud->lastTick     = 0;
    hud->lastTick = SDL_GetTicks();
}

void hud_update(HUD *hud)
{
    Uint32 now;
    float  deltaTime;

    if (!hud) return;

    now            = SDL_GetTicks();
    deltaTime      = (now - hud->lastTick) / 1000.0f;  /* ms → seconds     */
    hud->lastTick  = now;
    hud->timeSurvived += deltaTime;
}

void hud_render(HUD *hud, int health, int maxHealth)
{
    GFC_Rect   rect;
    GFC_String *buf;
    float       ratio;

    int x    = HUD_MARGIN;
    int y    = HUD_MARGIN;
    int barX = x + 40;  /* offset right of "HP" label                      */
    int barY   = y + 10; 

    /* ── Colors ──────────────────────────────────────────────────────── */
    GFC_Color colOverlay = gfc_color(0.00f, 0.00f, 0.00f, 0.47f);
    GFC_Color colBarBg   = GFC_COLOR_GREY;
    GFC_Color colBarFg   = gfc_color(0.86f, 0.20f, 0.20f, 1.00f);
    GFC_Color colBarLow  = GFC_COLOR_ORANGE;                /* < 25% HP     */
    GFC_Color colBorder  = gfc_color(0.78f, 0.78f, 0.78f, 0.70f);
    GFC_Color colText    = GFC_COLOR_WHITE;

    if (!hud) return;

    /* ── Clamp health ─────────────────────────────────────────────────── */
    if (health    < 0)         health    = 0;
    if (maxHealth < 1)         maxHealth = 1;
    if (health    > maxHealth) health    = maxHealth;
    ratio = (float)health / (float)maxHealth;

    /* ── Background panel ─────────────────────────────────────────────── */
    rect = gfc_rect(x - 6, y - 4,
                    barX + HUD_BAR_W + 70,
                    HUD_BAR_H * 2 + 28);
    gf2d_draw_rect_filled(rect, colOverlay);

    /* ── Row 1: Health ────────────────────────────────────────────────── */

    /* "HP" label */
    font_draw_text("HP", FS_small, colText, gfc_vector2d(x, y));

    /* Bar background */
    rect = gfc_rect(barX, barY, HUD_BAR_W, HUD_BAR_H);
    gf2d_draw_rect_filled(rect, colBarBg);
    gf2d_draw_rect(rect, colBorder);

    /* Bar fill – turns orange below 25% */
    rect = gfc_rect(barX + 1, barY + 1,
                    (int)((HUD_BAR_W - 2) * ratio),
                    HUD_BAR_H - 2);
    gf2d_draw_rect_filled(rect, (ratio < 0.25f) ? colBarLow : colBarFg);

    /* "current/max" beside bar */
    buf = gfc_stringf("%d/%d", health, maxHealth);
    font_draw_text(gfc_string_text(buf), FS_small, colText,
                   gfc_vector2d(barX + HUD_BAR_W + 6, y));
    gfc_string_free(buf);

    /* ── Row 2: Timer ─────────────────────────────────────────────────── */
    buf = gfc_stringf("Time  %ds", (int)hud->timeSurvived);
    font_draw_text(gfc_string_text(buf), FS_small, colText,
                   gfc_vector2d(x, y + HUD_BAR_H + 8));
    gfc_string_free(buf);
}