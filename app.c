#include <nds/system.h>
#include <nds/arm9/video.h>
#include <nds/interrupts.h>
#include <nds/timers.h>
#include <nds/arm9/console.h>

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define FRAMEBUFFER_SIZE SCREEN_WIDTH * SCREEN_HEIGHT
static u16 FRAMEBUFFER[FRAMEBUFFER_SIZE];

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

void arde_clear_framebuffer()
{
    memset(FRAMEBUFFER, (u16)0, FRAMEBUFFER_SIZE * sizeof(FRAMEBUFFER[0]));
}

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

void arde_draw_pixel(int x, int y, u8 r, u8 g, u8 b)
{
    if ((0 <= x) && (x < SCREEN_WIDTH) && (0 <= y) && (y < SCREEN_HEIGHT))
    {
        FRAMEBUFFER[y * SCREEN_WIDTH + x] = ARGB16(1, r >> 3, g >> 3, b >> 3);
    }
}

void arde_draw_circle(float x, float y, float radius)
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
                arde_draw_pixel(i, j, 255, 255, 255);
            }
        }
    }
}

typedef struct 
{
    float mass;
    float position[2];
    float velocity[2];
    float acceleration[2];
} arde_point_mass_t;

void arde_update_acceleration(arde_point_mass_t* point_mass, int point_mass_count, arde_point_mass_t* point_masses)
{
    static const float constant = 1e0f;

    float force[2];
    force[0] = 0.0f;
    force[1] = 0.0f;

    for (int point_mass_index = 0; point_mass_index < point_mass_count; ++point_mass_index)
    {
        arde_point_mass_t* other_point_mass = point_masses + point_mass_index;
        
        if (point_mass != other_point_mass)
        {
            float direction[2] = {
                other_point_mass->position[0] - point_mass->position[0],
                other_point_mass->position[1] - point_mass->position[1],
            };
            float distance = sqrtf(direction[0] * direction[0] + direction[1] * direction[1]);
            float strength = constant * point_mass->mass * other_point_mass->mass / (distance * distance);
            force[0] += strength * direction[0] / distance;
            force[1] += strength * direction[1] / distance;
        }
    }

    point_mass->acceleration[0] = force[0] / point_mass->mass;
    point_mass->acceleration[1] = force[1] / point_mass->mass;
}

void arde_point_mass_update_list(int point_mass_count, arde_point_mass_t* point_masses, float timestep)
{
    for (int point_mass_index = 0; point_mass_index < point_mass_count; ++point_mass_index)
    {
        arde_update_acceleration(point_masses + point_mass_index, point_mass_count, point_masses);
    }

    for (int point_mass_index = 0; point_mass_index < point_mass_count; ++point_mass_index)
    {
        arde_point_mass_t* point_mass = point_masses + point_mass_index;
        point_mass->velocity[0] += timestep * point_mass->acceleration[0];
        point_mass->velocity[1] += timestep * point_mass->acceleration[1];
        point_mass->position[0] += timestep * point_mass->velocity[0];
        point_mass->position[1] += timestep * point_mass->velocity[1];
    }
}

void arde_point_mass_draw_list(int point_mass_count, arde_point_mass_t* point_masses)
{
    for (int point_mass_index = 0; point_mass_index < point_mass_count; ++point_mass_index)
    {
        arde_point_mass_t* point_mass = point_masses + point_mass_index;
        arde_draw_circle(point_mass->position[0], point_mass->position[1], 0.01f);
    }
}

int main(void)
{
    videoSetMode(MODE_FB0);

    vramSetBankA(VRAM_A_LCD);

    consoleDemoInit();

    const int point_mass_count = 50;
    arde_point_mass_t point_masses[point_mass_count];

    for (int point_mass_index = 0; point_mass_index < point_mass_count; ++point_mass_index)
    {
        arde_point_mass_t* point_mass = point_masses + point_mass_index;
        point_mass->mass = 1.0f;
        point_mass->position[0] = 2.0f * rand()/RAND_MAX - 1.0f;
        point_mass->position[1] = 2.0f * rand()/RAND_MAX - 1.0f;
        point_mass->velocity[0] = 0.0f;
        point_mass->velocity[1] = 0.0f;
        point_mass->acceleration[0] = 0.0f;
        point_mass->acceleration[1] = 0.0f;
    }
    
    const float timestep = 1e-3f;

    while(1)
    {
        cpuStartTiming(0);

        arde_point_mass_update_list(point_mass_count, point_masses, timestep);

        arde_clear_framebuffer();

        arde_point_mass_draw_list(point_mass_count, point_masses);

        u32 per_frame_ticks = cpuEndTiming();

        consoleClear();
        printf("FPS: %.3f\n", (float)BUS_CLOCK / per_frame_ticks);

        swiWaitForVBlank();

        memcpy(VRAM_A, FRAMEBUFFER, FRAMEBUFFER_SIZE * sizeof(FRAMEBUFFER[0]));
    }

    return 0;
}