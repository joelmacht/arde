#include <arde/graphics/graphics.h>
#include <arde/physics/gravity/gravity.h>

#include <nds/system.h>
#include <nds/arm9/video.h>
#include <nds/interrupts.h>
#include <nds/timers.h>
#include <nds/arm9/console.h>

#include <stdio.h>
#include <stdlib.h>

static u16 FRAMEBUFFER[FRAMEBUFFER_SIZE];

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

        arde_clear_framebuffer(FRAMEBUFFER);

        arde_point_mass_draw_list(FRAMEBUFFER, point_mass_count, point_masses);

        u32 per_frame_ticks = cpuEndTiming();

        consoleClear();
        printf("FPS: %.3f\n", (float)BUS_CLOCK / per_frame_ticks);

        swiWaitForVBlank();

        memcpy(VRAM_A, FRAMEBUFFER, FRAMEBUFFER_SIZE * sizeof(FRAMEBUFFER[0]));
    }

    return 0;
}