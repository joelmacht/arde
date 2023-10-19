#include <arde/graphics/graphics.h>
#include <arde/physics/gravity/gravity.h>
#include <arde/controller.h>

#include <nds/system.h>
#include <nds/interrupts.h>
#include <nds/input.h>
#include <nds/timers.h>
#include <nds/ndstypes.h>
#include <nds/arm9/console.h>
#include <nds/arm9/video.h>
#include <nds/arm9/input.h>

#include <stdio.h>
#include <stdlib.h>

static u16 FRAMEBUFFER[FRAMEBUFFER_SIZE];

int main(void)
{
    videoSetMode(MODE_FB0);

    vramSetBankA(VRAM_A_LCD);

    consoleDemoInit();

    // const int point_mass_count = 50;
    // arde_point_mass_t point_masses[point_mass_count];

    // for (int point_mass_index = 0; point_mass_index < point_mass_count; ++point_mass_index)
    // {
    //     arde_point_mass_t* point_mass = point_masses + point_mass_index;
    //     point_mass->mass = 1.0f;
    //     point_mass->position[0] = 2.0f * rand()/RAND_MAX - 1.0f;
    //     point_mass->position[1] = 2.0f * rand()/RAND_MAX - 1.0f;
    //     point_mass->velocity[0] = 0.0f;
    //     point_mass->velocity[1] = 0.0f;
    //     point_mass->acceleration[0] = 0.0f;
    //     point_mass->acceleration[1] = 0.0f;
    // }

    // circular orbit for two body problem

    const int point_mass_count = 2;
    arde_point_mass_t point_masses[point_mass_count];

    point_masses[0].mass = 1.0f;
    point_masses[0].position[0] = 0.5f;
    point_masses[0].position[1] = 0.0f;
    point_masses[0].velocity[0] = 0.0f;
    point_masses[0].velocity[1] = 0.0f;
    point_masses[0].acceleration[0] = 0.0f;
    point_masses[0].acceleration[1] = 0.0f;

    point_masses[1].mass = 5.0f;
    point_masses[1].position[0] = -0.5f;
    point_masses[1].position[1] = 0.0f;
    point_masses[1].velocity[0] = 0.0f;
    point_masses[1].velocity[1] = 0.0f;
    point_masses[1].acceleration[0] = 0.0f;
    point_masses[1].acceleration[1] = 0.0f;
    
    arde_point_mass_t center_of_mass;
    arde_point_mass_center_of_mass(point_mass_count, point_masses, &center_of_mass);

    // resets acceleration...
    // arde_point_mass_update_collection(point_mass_count, point_masses, timestep);
    arde_point_mass_update_acceleration(&point_masses[0], point_mass_count, point_masses);    
    arde_point_mass_update_acceleration(&point_masses[1], point_mass_count, point_masses);    

    float a = sqrtf(point_masses[0].acceleration[0] * point_masses[0].acceleration[0] + point_masses[0].acceleration[1] * point_masses[0].acceleration[1]);
    float r = sqrtf(point_masses[0].position[0] * point_masses[0].position[0] + point_masses[0].position[1] * point_masses[0].position[1]);
    float v = sqrtf(a * r);

    point_masses[0].velocity[0] = point_masses[0].acceleration[1] / a * v;
    point_masses[0].velocity[1] = -point_masses[0].acceleration[0] / a * v;
    
    point_masses[1].velocity[0] = point_masses[1].acceleration[1] / a * v;
    point_masses[1].velocity[1] = -point_masses[1].acceleration[0] / a * v;

    timerStart(0, ClockDivider_1, 0xff, NULL);
    timerStart(1, ClockDivider_1, 0xff, NULL);

    float zoom = 1.0f;
    float yaw = 0.0f;

    while(1)
    {
        timerElapsed(0);

        u16 cycles_per_tick = timerElapsed(1);

        float timestep = cycles_per_tick / (float)BUS_CLOCK;

        scanKeys();
        int keys_down_state = keysDown();
        int keys_held_state = keysHeld();
        if (KEY_X & keys_down_state || KEY_X & keys_held_state)
        {
            zoom += 0.1f;
        }
        else if (KEY_Y & keys_down_state || KEY_Y & keys_held_state)
        {
            zoom -= 0.1f;
        }
        zoom = zoom > 0.0f ? zoom : 0.0f;
        world_to_observer.scaling.data[0] = zoom;
        world_to_observer.scaling.data[1] = zoom;
        if (KEY_UP & keys_down_state || KEY_UP & keys_held_state)
        {
            world_to_observer.translation.data[1] -= 0.1f;
        }
        if (KEY_DOWN & keys_down_state || KEY_DOWN & keys_held_state)
        {
            world_to_observer.translation.data[1] += 0.1f;
        }
        if (KEY_LEFT & keys_down_state || KEY_LEFT & keys_held_state)
        {
            world_to_observer.translation.data[0] += 0.1f;
        }
        if (KEY_RIGHT & keys_down_state || KEY_RIGHT & keys_held_state)
        {
            world_to_observer.translation.data[0] -= 0.1f;
        }
        if (KEY_L & keys_down_state || KEY_L & keys_held_state)
        {
            yaw += 0.1f;
        }
        else if (KEY_R & keys_down_state || KEY_R & keys_held_state)
        {
            yaw -= 0.1f;
        }
        world_to_observer.rotation.data[0] = cosf(yaw);
        world_to_observer.rotation.data[1] = sinf(yaw);
        world_to_observer.rotation.data[2] = -sinf(yaw);
        world_to_observer.rotation.data[3] = cosf(yaw);

        arde_point_mass_update_collection(point_mass_count, point_masses, timestep);

        arde_point_mass_center_of_mass(point_mass_count, point_masses, &center_of_mass);

        arde_clear_framebuffer(FRAMEBUFFER);

        arde_point_mass_draw_collection(FRAMEBUFFER, point_mass_count, point_masses);

        arde_point_mass_draw(FRAMEBUFFER, &center_of_mass);

        arde_draw_line(FRAMEBUFFER, 0.0f, 0.0f, 1.0f, 0.0f, 255, 0, 0);
        arde_draw_line(FRAMEBUFFER, 0.0f, 0.0f, 0.0f, 1.0f, 0, 255, 0);

        u16 cycles_per_frame = timerElapsed(0);

        consoleClear();
        printf("FPS: %.3f\n", (float)BUS_CLOCK / cycles_per_frame);
        printf("dt: %1.3ef\n", timestep);

        swiWaitForVBlank();

        memcpy(VRAM_A, FRAMEBUFFER, FRAMEBUFFER_SIZE * sizeof(FRAMEBUFFER[0]));
    }

    return 0;
}