#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <console/console.h>
#include <input/input.h>
#include <usb/usbmain.h>
#include <xenon_soc/xenon_power.h>
#include <xenos/xenos.h>

#define STICK_THRESHOLD 16000
#define ITEM_COUNT 3

static const char *items[ITEM_COUNT] = {
    "Home",
    "Library",
    "System"
};

static void draw_menu(int selected)
{
    int i;

    console_clrscr();
    printf("SeriesDash360 - LibXenon bootstrap\n");
    printf("===================================\n\n");
    printf("Bare-metal Xbox 360 build for XeLL Reloaded.\n");
    printf("This is the first LibXenon runtime port, not the XEX build.\n\n");

    for (i = 0; i < ITEM_COUNT; ++i)
        printf("%c %s\n", i == selected ? '>' : ' ', items[i]);

    printf("\nLeft stick: move   A: open   B: back   Y/Guide: exit\n");
}

static void draw_page(int selected)
{
    console_clrscr();

    if (selected == 0) {
        printf("SeriesDash360 / Home\n");
        printf("====================\n\n");
        printf("LibXenon runtime is alive.\n");
        printf("Next port stage: Series-style renderer and tile navigation.\n");
    } else if (selected == 1) {
        printf("SeriesDash360 / Library\n");
        printf("=======================\n\n");
        printf("Library scanner has not been ported to LibXenon yet.\n");
        printf("The existing XDK/XEX implementation remains separate.\n");
    } else {
        printf("SeriesDash360 / System\n");
        printf("======================\n\n");
        printf("Runtime: LibXenon / bare metal\n");
        printf("Target: Xbox 360 via XeLL Reloaded\n");
        printf("Build: SeriesDash360.elf32\n");
    }

    printf("\nPress B to return. Y/Guide exits.\n");
}

int main(void)
{
    struct controller_data_s pad;
    int selected = 0;
    bool in_page = false;

    xenos_init(VIDEO_MODE_AUTO);
    console_init();
    xenon_make_it_faster(XENON_SPEED_FULL);

    usb_init();
    usb_do_poll();

    draw_menu(selected);

    while (true) {
        usb_do_poll();

        if (!get_controller_data(&pad, 0))
            continue;

        if (pad.logo || pad.y)
            exit(0);

        if (!in_page) {
            if (pad.s1_y > STICK_THRESHOLD) {
                selected = (selected + ITEM_COUNT - 1) % ITEM_COUNT;
                draw_menu(selected);
                do {
                    usb_do_poll();
                    get_controller_data(&pad, 0);
                } while (pad.s1_y > STICK_THRESHOLD);
            } else if (pad.s1_y < -STICK_THRESHOLD) {
                selected = (selected + 1) % ITEM_COUNT;
                draw_menu(selected);
                do {
                    usb_do_poll();
                    get_controller_data(&pad, 0);
                } while (pad.s1_y < -STICK_THRESHOLD);
            } else if (pad.a) {
                in_page = true;
                draw_page(selected);
                do {
                    usb_do_poll();
                    get_controller_data(&pad, 0);
                } while (pad.a);
            }
        } else if (pad.b) {
            in_page = false;
            draw_menu(selected);
            do {
                usb_do_poll();
                get_controller_data(&pad, 0);
            } while (pad.b);
        }
    }

    return 0;
}
