#ifndef ARDE_GRAPHICS_GRAPHICS_H_INCLUDED
#define ARDE_GRAPHICS_GRAPHICS_H_INCLUDED

#include <nds/ndstypes.h>
#include <nds/arm9/video.h>

#include <math.h>
#include <string.h>

#define FRAMEBUFFER_SIZE SCREEN_WIDTH * SCREEN_HEIGHT

typedef struct {
    float data[2];
} arde_vector_t;

typedef struct {
    float data[4];
} arde_matrix_t;

typedef struct {
    arde_matrix_t rotation;
    arde_vector_t scaling;
    arde_vector_t translation;
} arde_transform_t;

extern const arde_transform_t world_to_observer;
extern const arde_transform_t observer_to_sensor;

void arde_vector_add(const arde_vector_t* first, const arde_vector_t* second, arde_vector_t* result);
void arde_vector_multiply(const arde_vector_t* first, const arde_vector_t* second, arde_vector_t* result);

void arde_matrix_vector_multiply(const arde_matrix_t* matrix, const arde_vector_t* vector, arde_vector_t* result);

void arde_transform_position(const arde_transform_t* transform, const arde_vector_t* position, arde_vector_t* result);
void arde_transform_direction(const arde_transform_t* transform, const arde_vector_t* direction, arde_vector_t* result);

void arde_clear_framebuffer(u16* framebuffer);
void arde_draw_pixel(u16* framebuffer, int x, int y, u8 r, u8 g, u8 b);
void arde_draw_circle(u16* framebuffer, float x, float y, float radius);

#endif // ARDE_GRAPHICS_GRAPHICS_H_INCLUDED+
