#include "nds/arm9/console.h"
#include "nds/system.h"

#include <stdio.h>

int main(void)
{
    powerOn(POWER_ALL_2D);

    videoSetMode(MODE_0_2D);

    setBackdropColor(1);

    // PrintConsole console;
    // consoleInit(&console, 0, BgType_Text8bpp, BgSize_B16_256x256, 0, 0, false, true);

    // consoleDebugInit(DebugDevice_CONSOLE);
    // consoleDemoInit();

    while(true)
    {
        // printf("Hello World\n");
    }

    return 0;
}