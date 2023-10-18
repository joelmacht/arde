#include "gravity.h"

#include <arde/graphics/graphics.h>

static const float constant = 1e2f;

float arde_gravitational_potential_gradient(float mass, float distance)
{
    return constant * mass / (distance * distance);
}

float arde_radial_harmonic_potential_gradient(float mass, float distance)
{
    return constant * mass * distance;
}

void arde_point_mass_update_acceleration(arde_point_mass_t* point_mass, int point_mass_count, arde_point_mass_t* point_masses)
{
    // required to happen before
    // point_mass->acceleration[0] = 0.0f;
    // point_mass->acceleration[1] = 0.0f;

    for (arde_point_mass_t* other_point_mass = point_mass + 1; other_point_mass <= point_masses + point_mass_count - 1; ++other_point_mass)
    {
        float radial_direction[2] = {
            point_mass->position[0] - other_point_mass->position[0],
            point_mass->position[1] - other_point_mass->position[1],
        };
        float distance = sqrtf(radial_direction[0] * radial_direction[0] + radial_direction[1] * radial_direction[1]);
        radial_direction[0] /= distance;
        radial_direction[1] /= distance;
        float acceleration_magnitude = -arde_radial_harmonic_potential_gradient(other_point_mass->mass, distance);
        point_mass->acceleration[0] += acceleration_magnitude * radial_direction[0];
        point_mass->acceleration[1] += acceleration_magnitude * radial_direction[1];
        other_point_mass->acceleration[0] -= acceleration_magnitude * radial_direction[0];
        other_point_mass->acceleration[1] -= acceleration_magnitude * radial_direction[1];
    }
}

void arde_point_mass_update_collection(int point_mass_count, arde_point_mass_t* point_masses, float timestep)
{
    for (int point_mass_index = 0; point_mass_index < point_mass_count; ++point_mass_index)
    {
        arde_point_mass_t* point_mass = point_masses + point_mass_index;

        point_mass->position[0] += point_mass->velocity[0] * timestep + 0.5f * point_mass->acceleration[0] * timestep * timestep;
        point_mass->position[1] += point_mass->velocity[1] * timestep + 0.5f * point_mass->acceleration[1] * timestep * timestep;
        
        // first half
        point_mass->velocity[0] += 0.5f * point_mass->acceleration[0] * timestep;
        point_mass->velocity[1] += 0.5f * point_mass->acceleration[1] * timestep;
        
        // reset
        point_mass->acceleration[0] = 0.0f;
        point_mass->acceleration[1] = 0.0f;
    }

    for (int point_mass_index = 0; point_mass_index < point_mass_count; ++point_mass_index)
    {
        arde_point_mass_t* point_mass = point_masses + point_mass_index;

        // should be independent of velocity
        arde_point_mass_update_acceleration(point_mass, point_mass_count, point_masses);
        
        // second half
        point_mass->velocity[0] += 0.5f * point_mass->acceleration[0] * timestep;
        point_mass->velocity[1] += 0.5f * point_mass->acceleration[1] * timestep;
    }
}

void arde_point_mass_draw(u16* framebuffer, const arde_point_mass_t* point_mass)
{
    arde_draw_circle(framebuffer, point_mass->position[0], point_mass->position[1], 0.01f);
}

void arde_point_mass_draw_collection(u16* framebuffer, int point_mass_count, const arde_point_mass_t* point_masses)
{
    for (int point_mass_index = 0; point_mass_index < point_mass_count; ++point_mass_index)
    {
        arde_point_mass_draw(framebuffer, &point_masses[point_mass_index]);
    }
}

void arde_point_mass_center_of_mass(const int point_mass_count, const arde_point_mass_t* point_masses, arde_point_mass_t * center_of_mass)
{
    center_of_mass->mass = 0.0f;
    center_of_mass->position[0] = 0.0f;
    center_of_mass->position[1] = 0.0f;
    center_of_mass->velocity[0] = 0.0f;
    center_of_mass->velocity[1] = 0.0f;
    center_of_mass->acceleration[0] = 0.0f;
    center_of_mass->acceleration[1] = 0.0f;

    for (int point_mass_index = 0; point_mass_index < point_mass_count; ++point_mass_index)
    {
        const arde_point_mass_t * point_mass = &point_masses[point_mass_index];
        center_of_mass->mass += point_mass->mass;
        center_of_mass->position[0] += point_mass->mass * point_mass->position[0];
        center_of_mass->position[1] += point_mass->mass * point_mass->position[1];
    }
    center_of_mass->position[0] /= center_of_mass->mass;
    center_of_mass->position[1] /= center_of_mass->mass;
}
