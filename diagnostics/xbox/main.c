/* SPDX-License-Identifier: Apache-2.0 */
/* Original diagnostic title inspired by OXOS's image/controller test techniques. */
#include <SDL.h>
#include <hal/debug.h>
#include <hal/video.h>
#include <pbkit/pbkit.h>
#include <windows.h>
#include <xboxkrnl/xboxkrnl.h>
#include "telemetry.h"

static SDL_GameController *controllers[4];
static struct diag_pad pads[4];
/* Low byte matches XID digital bits; high byte is A B X Y Black White LT RT. */
static const SDL_GameControllerButton buttons[14] = {
    SDL_CONTROLLER_BUTTON_DPAD_UP, SDL_CONTROLLER_BUTTON_DPAD_DOWN,
    SDL_CONTROLLER_BUTTON_DPAD_LEFT, SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
    SDL_CONTROLLER_BUTTON_START, SDL_CONTROLLER_BUTTON_BACK,
    SDL_CONTROLLER_BUTTON_LEFTSTICK, SDL_CONTROLLER_BUTTON_RIGHTSTICK,
    SDL_CONTROLLER_BUTTON_A, SDL_CONTROLLER_BUTTON_B, SDL_CONTROLLER_BUTTON_X,
    SDL_CONTROLLER_BUTTON_Y, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
    SDL_CONTROLLER_BUTTON_LEFTSHOULDER
};
static void sample_pads(void)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) { /* SDL refreshes hotplug state. */ }
    for (int i = 0; i < 4; i++) {
        if (controllers[i] && !SDL_GameControllerGetAttached(controllers[i])) {
            SDL_GameControllerClose(controllers[i]); controllers[i] = NULL;
        }
    }
    for (int n = 0; n < SDL_NumJoysticks(); n++) {
        SDL_GameController *pad = SDL_GameControllerOpen(n);
        if (!pad) continue;
        int slot = SDL_GameControllerGetPlayerIndex(pad) - 1;
        if (slot < 0 || slot >= 4 || controllers[slot]) SDL_GameControllerClose(pad);
        else controllers[slot] = pad;
    }
    SDL_GameControllerUpdate();
    for (int i = 0; i < 4; i++) {
        uint16_t mask = 0;
        pads[i].present = controllers[i] != NULL;
        if (controllers[i]) {
            for (int bit = 0; bit < 14; bit++)
                if (SDL_GameControllerGetButton(controllers[i], buttons[bit])) mask |= 1u << bit;
            if (SDL_GameControllerGetAxis(controllers[i], SDL_CONTROLLER_AXIS_TRIGGERLEFT) > 16000) mask |= 1u << 14;
            if (SDL_GameControllerGetAxis(controllers[i], SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > 16000) mask |= 1u << 15;
        }
        diag_sample(&pads[i], mask);
    }
}
int main(void)
{
    if (!XVideoSetMode(640, 480, 32, REFRESH_DEFAULT) || SDL_Init(SDL_INIT_GAMECONTROLLER) || pb_init()) {
        debugPrint("KVM diagnostic initialization failed\n");
        for (;;) Sleep(1000);
    }
    pb_show_front_screen();
    uint32_t frames = 0, started = GetTickCount();
    const uint32_t colors[] = {0xffff0000, 0xff00ff00, 0xff0000ff, 0xffffffff,
                               0xff000000, 0xff808080, 0xffffff00, 0xff00ffff};
    for (;;) {
        pb_wait_for_vbl(); sample_pads(); frames++;
        MM_STATISTICS memory = {0}; memory.Length = sizeof(memory);
        MmQueryStatistics(&memory);
        uint8_t bytes[DIAG_BYTES];
        uint32_t ms = GetTickCount() - started;
        diag_encode(bytes, frames, ms, memory.AvailablePages, pads);
        pb_target_back_buffer(); pb_reset();
        pb_fill(0, 0, 640, 480, 0xff202020);
        pb_erase_text_screen();
        pb_print("ESP-KVM DIAGNOSTIC XKD1 - 640x480\n");
        pb_print("Alive %lu s  Frames %lu  Free %lu pages\n", (unsigned long)(ms / 1000),
                 (unsigned long)frames, (unsigned long)memory.AvailablePages);
        pb_print("Held/last: hex XID bits + A B X Y Black White LT RT\n");
        for (int i = 0; i < 4; i++) pb_print("P%d %s held=%04x last=%04x press=%lu release=%lu\n", i + 1,
            pads[i].present ? "ON " : "OFF", pads[i].held, pads[i].last,
            (unsigned long)pads[i].presses, (unsigned long)pads[i].releases);
        pb_print("Input acknowledged by this title; no rumble or reset actions.\n");
        pb_fill(DIAG_X, DIAG_Y, DIAG_W, DIAG_H, 0xffff00ff);
        for (unsigned bit = 0; bit < DIAG_BYTES * 8; bit++) {
            int value = (bytes[bit / 8] >> (7 - bit % 8)) & 1;
            pb_fill(DIAG_X + DIAG_BORDER + (bit % DIAG_COLS) * DIAG_CELL,
                    DIAG_Y + DIAG_BORDER + (bit / DIAG_COLS) * DIAG_CELL,
                    DIAG_CELL, DIAG_CELL, value ? 0xffffffff : 0xff000000);
        }
        /* Fixed reference bars; the magenta marker border is unique. */
        for (int i = 0; i < 8; i++) pb_fill(64 + i * 64, 436, 64, 24, colors[i]);
        pb_draw_text_screen();
        while (pb_busy()) { }
        while (pb_finished()) { }
    }
}
