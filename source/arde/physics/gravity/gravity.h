#ifndef ARDE_PHYSICS_GRAVITY_GRAVITY_H_INCLUDED
#define ARDE_PHYSICS_GRAVITY_GRAVITY_H_INCLUDED

#include <nds/ndstypes.h>

typedef struct 
{
    float mass;
    float position[2];
    float velocity[2];
    float acceleration[2];
} arde_point_mass_t;

void arde_update_acceleration(arde_point_mass_t* point_mass, int point_mass_count, arde_point_mass_t* point_masses);

void arde_point_mass_update_collection(int point_mass_count, arde_point_mass_t* point_masses, float timestep);

void arde_point_mass_draw_collection(u16* framebuffer, int point_mass_count, arde_point_mass_t* point_masses);

#endif // ARDE_PHYSICS_GRAVITY_GRAVITY_H_INCLUDED
