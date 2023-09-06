#include "gravity.h"

#include <arde/graphics/graphics.h>

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

void arde_point_mass_draw_list(u16* framebuffer, int point_mass_count, arde_point_mass_t* point_masses)
{
    for (int point_mass_index = 0; point_mass_index < point_mass_count; ++point_mass_index)
    {
        arde_point_mass_t* point_mass = point_masses + point_mass_index;
        arde_draw_circle(framebuffer, point_mass->position[0], point_mass->position[1], 0.01f);
    }
}
