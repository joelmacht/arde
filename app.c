#include <nds/system.h>
#include <nds/arm9/video.h>
#include <nds/interrupts.h>

int main(void)
{
    videoSetMode(MODE_FB0);

    vramSetBankA(VRAM_A_LCD);

    for (int y = 0; y < SCREEN_HEIGHT; ++y)
    {
        for (int x = 0; x < SCREEN_WIDTH; ++x)
        {
            VRAM_A[y * SCREEN_WIDTH + x] = ARGB16(1, 15, 0, 0);
        }
    }

    while(1)
    {
        swiWaitForVBlank();
    }

    return 0;
}