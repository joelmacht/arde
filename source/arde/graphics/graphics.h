#ifndef ARDE_GRAPHICS_GRAPHICS_H_INCLUDED
#define ARDE_GRAPHICS_GRAPHICS_H_INCLUDED

#include <arde/math/vector.h>

#include <nds/ndstypes.h>
#include <nds/arm9/video.h>

#include <math.h>
#include <string.h>

#define FRAMEBUFFER_SIZE SCREEN_WIDTH * SCREEN_HEIGHT

extern arde_transform_t world_to_observer;
extern arde_transform_t observer_to_sensor;

void arde_clear_framebuffer(u16* framebuffer);
void arde_draw_pixel(u16* framebuffer, int x, int y, u8 r, u8 g, u8 b);
void arde_draw_circle(u16* framebuffer, float x, float y, float radius);

#endif // ARDE_GRAPHICS_GRAPHICS_H_INCLUDED
