#include <nds/system.h>
#include <nds/arm9/video.h>
#include <nds/interrupts.h>

#include <math.h>

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

int main(void)
{
    videoSetMode(MODE_FB0);

    vramSetBankA(VRAM_A_LCD);

    draw_circle(SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f, 10.0f);

    while(1)
    {
        swiWaitForVBlank();
    }

    return 0;
}