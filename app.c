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
} vector_t;

typedef struct {
    float data[4];
} matrix_t;

typedef struct {
    matrix_t rotation;
    vector_t scaling;
    vector_t translation;
} transform_t;

void vector_add(const vector_t* first, const vector_t* second, vector_t* result)
{
    result->data[0] = first->data[0] + second->data[0];
    result->data[1] = first->data[1] + second->data[1];
}

void vector_multiply(const vector_t* first, const vector_t* second, vector_t* result)
{
    result->data[0] = first->data[0] * second->data[0];
    result->data[1] = first->data[1] * second->data[1];
}

void matrix_vector_multiply(const matrix_t* matrix, const vector_t* vector, vector_t* result)
{
    result->data[0] = vector->data[0] * matrix->data[0] + vector->data[1] * matrix->data[1];
    result->data[1] = vector->data[0] * matrix->data[2] + vector->data[1] * matrix->data[3];
}

void transform_position(const transform_t* transform, const vector_t* position, vector_t* result)
{
    matrix_vector_multiply(&transform->rotation, position, result);
    vector_t scratchboard;
    vector_multiply(&transform->scaling, result, &scratchboard);
    vector_add(&transform->translation, &scratchboard, result);
}

void transform_direction(const transform_t* transform, const vector_t* direction, vector_t* result)
{
    vector_t scratchboard;
    matrix_vector_multiply(&transform->rotation, direction, &scratchboard);
    vector_multiply(&transform->scaling, &scratchboard, result);
}

void clear_framebuffer()
{
    memset(FRAMEBUFFER, (u16)0, FRAMEBUFFER_SIZE * sizeof(FRAMEBUFFER[0]));
}

const transform_t world_to_observer = {
    {{1.0f, 0.0f, 0.0f, 1.0f}},
    {{1.0f, 1.0f}},
    {{0.0f, 0.0f}}
};

const transform_t observer_to_sensor = {
    {{1.0f, 0.0f, 0.0f, -1.0f}},
    {{SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f}},
    {{SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f}}
};

void draw_pixel(int x, int y, u8 r, u8 g, u8 b)
{
    if ((0 <= x) && (x < SCREEN_WIDTH) && (0 <= y) && (y < SCREEN_HEIGHT))
    {
        FRAMEBUFFER[y * SCREEN_WIDTH + x] = ARGB16(1, r >> 3, g >> 3, b >> 3);
    }
}

void draw_circle(float x, float y, float radius)
{
    vector_t position_world = {{x, y}};
    vector_t position_observation;
    vector_t position_sensor;
    transform_position(&world_to_observer, &position_world, &position_observation);
    transform_position(&observer_to_sensor, &position_observation, &position_sensor);
    x = position_sensor.data[0];
    y = position_sensor.data[1];
    
    position_world.data[0] = 0.0f;
    position_world.data[1] = radius;
    transform_direction(&world_to_observer, &position_world, &position_observation);
    transform_direction(&observer_to_sensor, &position_observation, &position_sensor);
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
                draw_pixel(i, j, 255, 255, 255);
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
} point_mass_t;

void update_acceleration(point_mass_t* point_mass, int point_mass_count, point_mass_t* point_masses)
{
    static const float constant = 1e0f;

    float force[2];
    force[0] = 0.0f;
    force[1] = 0.0f;

    for (int point_mass_index = 0; point_mass_index < point_mass_count; ++point_mass_index)
    {
        point_mass_t* other_point_mass = point_masses + point_mass_index;
        
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

void point_mass_update_list(int point_mass_count, point_mass_t* point_masses, float timestep)
{
    for (int point_mass_index = 0; point_mass_index < point_mass_count; ++point_mass_index)
    {
        update_acceleration(point_masses + point_mass_index, point_mass_count, point_masses);
    }

    for (int point_mass_index = 0; point_mass_index < point_mass_count; ++point_mass_index)
    {
        point_mass_t* point_mass = point_masses + point_mass_index;
        point_mass->velocity[0] += timestep * point_mass->acceleration[0];
        point_mass->velocity[1] += timestep * point_mass->acceleration[1];
        point_mass->position[0] += timestep * point_mass->velocity[0];
        point_mass->position[1] += timestep * point_mass->velocity[1];
    }
}

void point_mass_draw_list(int point_mass_count, point_mass_t* point_masses)
{
    for (int point_mass_index = 0; point_mass_index < point_mass_count; ++point_mass_index)
    {
        point_mass_t* point_mass = point_masses + point_mass_index;
        draw_circle(point_mass->position[0], point_mass->position[1], 0.01f);
    }
}

int main(void)
{
    videoSetMode(MODE_FB0);

    vramSetBankA(VRAM_A_LCD);

    consoleDemoInit();

    const int point_mass_count = 50;
    point_mass_t point_masses[point_mass_count];

    for (int point_mass_index = 0; point_mass_index < point_mass_count; ++point_mass_index)
    {
        point_mass_t* point_mass = point_masses + point_mass_index;
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

        point_mass_update_list(point_mass_count, point_masses, timestep);

        clear_framebuffer();

        point_mass_draw_list(point_mass_count, point_masses);

        u32 per_frame_ticks = cpuEndTiming();

        consoleClear();
        printf("FPS: %.3f\n", (float)BUS_CLOCK / per_frame_ticks);

        swiWaitForVBlank();

        memcpy(VRAM_A, FRAMEBUFFER, FRAMEBUFFER_SIZE * sizeof(FRAMEBUFFER[0]));
    }

    return 0;
}