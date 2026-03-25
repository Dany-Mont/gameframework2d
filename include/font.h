#ifndef __FONT_H__
#define __FONT_H__

#include "gfc_text.h"
#include "gfc_color.h"

typedef enum
{
    FS_small,
    FS_medium,
    FS_large,
    FS_MAX
} FontStyles;

/**
 * @brief initialize the font system
 * autocloses on exit
 */
void font_init();

/**
 * @brief draw some text to the screen
 * @param text the text to draw
 * @param style the font style to use
 * @param color the color of the text
 * @param position where on the screen to draw the text
 */
void font_draw_text(const char *text, FontStyles style, GFC_Color color, GFC_Vector2D position);

/**
 * @brief peridoically clean up old font cache entries
 */
void font_cleanup();
#endif
