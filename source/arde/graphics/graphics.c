#include "graphics.h"

const arde_transform_t world_to_observer = {
    {{1.0f, 0.0f, 0.0f, 1.0f}},
    {{1.0f, 1.0f}},
    {{0.0f, 0.0f}}
};

const arde_transform_t observer_to_sensor = {
    {{1.0f, 0.0f, 0.0f, -1.0f}},
    {{SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f}},
    {{SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f}}
};


void arde_vector_add(const arde_vector_t* first, const arde_vector_t* second, arde_vector_t* result)
{
    result->data[0] = first->data[0] + second->data[0];
    result->data[1] = first->data[1] + second->data[1];
}

void arde_vector_multiply(const arde_vector_t* first, const arde_vector_t* second, arde_vector_t* result)
{
    result->data[0] = first->data[0] * second->data[0];
    result->data[1] = first->data[1] * second->data[1];
}

void arde_matrix_vector_multiply(const arde_matrix_t* matrix, const arde_vector_t* vector, arde_vector_t* result)
{
    result->data[0] = vector->data[0] * matrix->data[0] + vector->data[1] * matrix->data[1];
    result->data[1] = vector->data[0] * matrix->data[2] + vector->data[1] * matrix->data[3];
}

void arde_transform_position(const arde_transform_t* transform, const arde_vector_t* position, arde_vector_t* result)
{
    arde_matrix_vector_multiply(&transform->rotation, position, result);
    arde_vector_t scratchboard;
    arde_vector_multiply(&transform->scaling, result, &scratchboard);
    arde_vector_add(&transform->translation, &scratchboard, result);
}

void arde_transform_direction(const arde_transform_t* transform, const arde_vector_t* direction, arde_vector_t* result)
{
    arde_vector_t scratchboard;
    arde_matrix_vector_multiply(&transform->rotation, direction, &scratchboard);
    arde_vector_multiply(&transform->scaling, &scratchboard, result);
}

void arde_clear_framebuffer(u16* framebuffer)
{
    memset(framebuffer, (u16)0, FRAMEBUFFER_SIZE * sizeof(framebuffer[0]));
}

void arde_draw_pixel(u16* framebuffer, int x, int y, u8 r, u8 g, u8 b)
{
    if ((0 <= x) && (x < SCREEN_WIDTH) && (0 <= y) && (y < SCREEN_HEIGHT))
    {
        framebuffer[y * SCREEN_WIDTH + x] = ARGB16(1, r >> 3, g >> 3, b >> 3);
    }
}

void arde_draw_circle(u16* framebuffer, float x, float y, float radius)
{
    arde_vector_t position_world = {{x, y}};
    arde_vector_t position_observation;
    arde_vector_t position_sensor;
    arde_transform_position(&world_to_observer, &position_world, &position_observation);
    arde_transform_position(&observer_to_sensor, &position_observation, &position_sensor);
    x = position_sensor.data[0];
    y = position_sensor.data[1];
    
    position_world.data[0] = 0.0f;
    position_world.data[1] = radius;
    arde_transform_direction(&world_to_observer, &position_world, &position_observation);
    arde_transform_direction(&observer_to_sensor, &position_observation, &position_sensor);
    radius = position_sensor.data[1];

    radius = radius > 0.0f ? radius : -radius;

    float radius_squared = radius * radius;

    int min_i = (int)floorf(x - radius);
    int max_i = (int)ceilf(x + radius);
    int min_j = (int)floorf(y - radius);
    int max_j = (int)ceilf(y + radius);

    for (int j = min_j; j < max_j; ++j)
    {
        for (int i = min_i; i < max_i; ++i)
        {
            float pixel_center[2] = {i + 0.5f, j + 0.5f};
            float distance_squared = (pixel_center[0] - x) * (pixel_center[0] - x) + (pixel_center[1] - y) * (pixel_center[1] - y);
            if (distance_squared <= radius_squared)
            {
                arde_draw_pixel(framebuffer, i, j, 255, 255, 255);
            }
        }
    }
}
