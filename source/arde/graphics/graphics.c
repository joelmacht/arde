#include "graphics.h"

#include <arde/math/vector.h>

arde_transform_t world_to_observer = {
    {{1.0f, 0.0f, 0.0f, 1.0f}},
    {{1.0f, 1.0f}},
    {{0.0f, 0.0f}}
};

arde_transform_t observer_to_sensor = {
    {{1.0f, 0.0f, 0.0f, -1.0f}},
    {{SCREEN_HEIGHT / 2.0f /* height < width -> aspect*/, SCREEN_HEIGHT / 2.0f}},
    {{SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f}}
};

void _project_position(const arde_vector_t * position, arde_vector_t * projected_position)
{
    arde_vector_t spm;
    arde_transform_position(&world_to_observer, position, &spm);
    arde_transform_position(&observer_to_sensor, &spm, projected_position);
}

arde_vector_t project_position(arde_vector_t position)
{
    arde_vector_t projected_position;
    _project_position(&position, &projected_position);
    return projected_position;
}

void _project_direction(const arde_vector_t * direction, arde_vector_t * projected_direction)
{
    arde_vector_t spm;
    arde_transform_direction(&world_to_observer, direction, &spm);
    arde_transform_direction(&observer_to_sensor, &spm, projected_direction);
}

arde_vector_t project_direction(arde_vector_t direction)
{
    arde_vector_t projected_direction;
    _project_direction(&direction, &projected_direction);
    return projected_direction;
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


void _arde_draw_line(u16* framebuffer, float x0, float y0, float x1, float y1, u8 r, u8 g, u8 b)
{
    float sx = x0 <= x1 ? 1.0f : -1.0f;
    float sy = y0 <= y1 ? 1.0f : -1.0f;
    float dx = sx * (x1 - x0);
    float dy = sy * (y1 - y0);

    float x = x0;
    float y = y0;

    if (dx >= dy)
    {
        const float m = dy / dx;
        while (sx * (x - x0) < dx)
        {
            arde_draw_pixel(framebuffer, x, y, r, g, b);
            x += sx * 1.0f;
            y += sy * m;
        }
    }
    else
    {
        const float m = dx / dy;
        while (sy * (y - y0) < dy)
        {
            arde_draw_pixel(framebuffer, x, y, r, g, b);
            y += sy * 1.0f;
            x += sx * m;
        }
    }
}

void arde_draw_line(u16* framebuffer, float x0, float y0, float x1, float y1, u8 r, u8 g, u8 b)
{
    arde_vector_t start = project_position((arde_vector_t){{x0, y0}});
    x0 = start.data[0];
    y0 = start.data[1];

    arde_vector_t end = project_position((arde_vector_t){{x1, y1}});
    x1 = end.data[0];
    y1 = end.data[1];

    _arde_draw_line(framebuffer, x0, y0, x1, y1, r, g, b);
}

void arde_draw_line_screen_space(u16* framebuffer, float x0, float y0, float x1, float y1, u8 r, u8 g, u8 b)
{
    arde_vector_t _start = {{x0, y0}};
    arde_vector_t start;
    arde_transform_position(&observer_to_sensor, &_start, &start);

    arde_vector_t _end = {{x1, y1}};
    arde_vector_t end;
    arde_transform_position(&observer_to_sensor, &_end, &end);

   _arde_draw_line(framebuffer, start.data[0], start.data[1], end.data[0], end.data[1], r, g, b);
}

void arde_draw_circle(u16* framebuffer, float x, float y, float radius)
{
    arde_vector_t center = project_position((arde_vector_t){{x, y}});
    x = center.data[0];
    y = center.data[1];

    arde_vector_t extend = project_direction((arde_vector_t){{radius, 0.0f}});
    radius = sqrtf(extend.data[0] * extend.data[0] + extend.data[1] * extend.data[1]);

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
