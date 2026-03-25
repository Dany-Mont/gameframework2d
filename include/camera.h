#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "gfc_vector.h"
#include "gfc_shape.h"


/**
 * @brief get the current position of the camera
 */
GFC_Vector2D camera_get_position();
/**
 * @brief get the offset to apply to world coordinates to convert them to screen coordinates
 * @return the offset 
 */
GFC_Vector2D camera_get_offset();

/**
 * @brief set the position of the camera
 * @param position the new position of the camera
 */
void camera_set_position(GFC_Vector2D position);

/**
 * @brief set the size of the camera view
 * @param size the new size of the camera view
 */
void camera_set_size(GFC_Vector2D size);

/**
 * @brief apply the camera bounds to the current camera position
 */
void camera_apply_bounds();

void camera_set_bounds(GFC_Rect bounds);

void camera_enable_binding(Bool bindCamera);

void camera_center_on(GFC_Vector2D target);

#endif
