/*
 * SeriesDash360 OpenXeChain SDK-free compatibility runtime.
 *
 * This target intentionally uses only APIs currently exposed by the
 * open xecorelib import libraries. It is a runnable bootstrap used to
 * validate source -> Xbox PE -> XEX2 without Microsoft's XDK.
 *
 * The full Series-style Direct3D runtime remains in platform/xbox360
 * while its rendering/network/UI calls are progressively moved to
 * open equivalents.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <xecore/xam.h>
#include <xecore/xboxkrnl.h>

typedef struct SD_GAMEPAD {
    uint16_t buttons;
    uint8_t left_trigger;
    uint8_t right_trigger;
    int16_t thumb_lx;
    int16_t thumb_ly;
    int16_t thumb_rx;
    int16_t thumb_ry;
} SD_GAMEPAD;

typedef struct SD_INPUT_STATE {
    uint32_t packet_number;
    SD_GAMEPAD gamepad;
} SD_INPUT_STATE;

/* xam.xex exports present in OpenXeChain xam.def. */
extern uint32_t XamInputGetState(uint32_t user_index,
                                 uint32_t flags,
                                 SD_INPUT_STATE *state);
extern void XNotifyQueueUI(uint32_t type,
                           uint32_t user_index,
                           uint64_t areas_or_priority,
                           const uint16_t *text,
                           void *context);

#define BTN_BACK  0x0020
#define BTN_START 0x0010
#define BTN_A     0x1000
#define BTN_B     0x2000

static void ascii_to_utf16(const char *src, uint16_t *dst, uint32_t cap) {
    uint32_t i = 0;
    if (!cap) return;
    while (src[i] && i + 1 < cap) {
        dst[i] = (uint8_t)src[i];
        ++i;
    }
    dst[i] = 0;
}

static void notify(const char *text) {
    uint16_t wide[160];
    ascii_to_utf16(text, wide, 160);
    XNotifyQueueUI(14, 0, 2, wide, 0);
}

static void delay_ms(uint32_t ms) {
    int64_t interval = -((int64_t)ms * 10000);
    KeDelayExecutionThread(0, 0, &interval);
}

int main(void) {
    uint8_t smc[16];
    uint8_t reply[16];
    char status[160];
    SD_INPUT_STATE state;
    uint16_t old_buttons = 0;

    memset(smc, 0, sizeof(smc));
    memset(reply, 0, sizeof(reply));
    smc[0] = 0x07;
    HalSendSMCMessage(smc, reply);

    snprintf(status, sizeof(status),
             "SeriesDash360 SDK-free runtime loaded - CPU %uC GPU %uC",
             (unsigned)reply[1], (unsigned)reply[2]);
    notify(status);

    for (;;) {
        uint16_t buttons = 0;
        memset(&state, 0, sizeof(state));

        if (XamInputGetState(0, 1, &state) == 0) {
            buttons = state.gamepad.buttons;
        }

        if ((buttons & BTN_A) && !(old_buttons & BTN_A)) {
            notify("SeriesDash360: OpenXeChain input works");
        }

        if ((buttons & BTN_START) && !(old_buttons & BTN_START)) {
            memset(reply, 0, sizeof(reply));
            HalSendSMCMessage(smc, reply);
            snprintf(status, sizeof(status),
                     "CPU %uC  GPU %uC  EDRAM %uC  BOARD %uC",
                     (unsigned)reply[1], (unsigned)reply[2],
                     (unsigned)reply[3], (unsigned)reply[4]);
            notify(status);
        }

        if ((buttons & BTN_BACK) && !(old_buttons & BTN_BACK)) {
            notify("SeriesDash360: returning to host");
            delay_ms(500);
            XamLoaderTerminateTitle();
            break;
        }

        old_buttons = buttons;
        delay_ms(16);
    }

    return 0;
}
