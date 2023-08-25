#include <nds/system.h>
#include <nds/arm9/video.h>
#include <nds/interrupts.h>

#include <math.h>

void clear_framebuffer()
{
    memset(VRAM_A, (u16)0, sizeof(u16) * SCREEN_WIDTH * SCREEN_HEIGHT);
}

void draw_pixel(int x, int y, u8 r, u8 g, u8 b)
{
    VRAM_A[y * SCREEN_WIDTH + x] = ARGB16(1, r >> 3, g >> 3, b >> 3);
}

void draw_circle(float x, float y, float radius)
{
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
    static const float constant = 1e4f;

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

void update(int point_mass_count, point_mass_t* point_masses, float timestep)
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

int main(void)
{
    videoSetMode(MODE_FB0);

    vramSetBankA(VRAM_A_LCD);

    const int point_mass_count = 3;
    point_mass_t point_masses[point_mass_count];
    
    point_masses[0].mass = 5.0;
    point_masses[0].position[0] = SCREEN_WIDTH / 2.0f - 50.0f;
    point_masses[0].position[1] = SCREEN_HEIGHT / 2.0f;
    point_masses[0].velocity[0] = 10.0f;
    point_masses[0].velocity[1] = 20.0f;
    point_masses[0].acceleration[0] = 0.0f;
    point_masses[0].acceleration[1] = 0.0f;

    point_masses[1].mass = 5.0;
    point_masses[1].position[0] = SCREEN_WIDTH / 2.0f + 50.0f;
    point_masses[1].position[1] = SCREEN_HEIGHT / 2.0f;
    point_masses[1].velocity[0] = -40.0f;
    point_masses[1].velocity[1] = -20.0f;
    point_masses[1].acceleration[0] = 0.0f;
    point_masses[1].acceleration[1] = 0.0f;

    point_masses[2].mass = 10.0;
    point_masses[2].position[0] = SCREEN_WIDTH / 2.0f;
    point_masses[2].position[1] = SCREEN_HEIGHT / 2.0f;
    point_masses[2].velocity[0] = 0.0f;
    point_masses[2].velocity[1] = 0.0f;
    point_masses[2].acceleration[0] = 0.0f;
    point_masses[2].acceleration[1] = 0.0f;

    float timestep = 1e-3f;

    while(1)
    {
        swiWaitForVBlank();

        clear_framebuffer();

        for (int point_mass_index = 0; point_mass_index < point_mass_count; ++point_mass_index)
        {
            point_mass_t* point_mass = point_masses + point_mass_index;
            draw_circle(point_mass->position[0], point_mass->position[1], point_mass->mass);
        }

        update(point_mass_count, point_masses, timestep);
    }

    return 0;
}