#include "gravity.h"

#include <arde/graphics/graphics.h>

static const float constant = 1e0f;

float arde_gravitational_potential_gradient(float mass, float distance)
{
    return constant * mass / (distance * distance);
}

float arde_radial_harmonic_potential_gradient(float mass, float distance)
{
    return constant * mass * distance;
}

void arde_update_acceleration(arde_point_mass_t* point_mass, int point_mass_count, arde_point_mass_t* point_masses)
{
    float force[2];
    force[0] = 0.0f;
    force[1] = 0.0f;

    for (int point_mass_index = 0; point_mass_index < point_mass_count; ++point_mass_index)
    {
        arde_point_mass_t* other_point_mass = point_masses + point_mass_index;
        
        if (point_mass != other_point_mass)
        {
            float radial_direction[2] = {
                point_mass->position[0] - other_point_mass->position[0],
                point_mass->position[1] - other_point_mass->position[1],
            };
            float distance = sqrtf(radial_direction[0] * radial_direction[0] + radial_direction[1] * radial_direction[1]);
            radial_direction[0] /= distance;
            radial_direction[1] /= distance;
            // float force_magnitude = point_mass->mass * -arde_gravitational_potential_gradient(other_point_mass->mass, distance);
            float force_magnitude = point_mass->mass * -arde_radial_harmonic_potential_gradient(other_point_mass->mass, distance);
            force[0] += force_magnitude * radial_direction[0];
            force[1] += force_magnitude * radial_direction[1];
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
