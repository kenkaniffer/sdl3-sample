#include <SDL3/SDL_main.h>

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    if (!SDL_SetAppMetadata("Example splitscreen shooter game", "1.0", "com.example.woodeneye-008")) {
        return SDL_APP_FAILURE;
    }
    int i;
    for (i = 0; i < SDL_arraysize(extended_metadata); i++) {
        if (!SDL_SetAppMetadataProperty(extended_metadata[i].key, extended_metadata[i].value)) {
            return SDL_APP_FAILURE;
        }
    }

    AppState *as = (AppState*)SDL_calloc(1, sizeof(AppState));
    if (!as) {
        return SDL_APP_FAILURE;
    } else {
        *appstate = as;
    }

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        return SDL_APP_FAILURE;
    }
    if (!SDL_CreateWindowAndRenderer("examples/demo/woodeneye-008", 640, 480, 0, &as->window, &as->renderer)) {
        return SDL_APP_FAILURE;
    }

    as->player_count = 1;
    initPlayers(as->players, MAX_PLAYER_COUNT);
    initEdges(MAP_BOX_SCALE, as->edges, MAP_BOX_EDGES_LEN);
    debug_string[0] = 0;

    SDL_SetRenderVSync(as->renderer, false);
    SDL_SetWindowRelativeMouseMode(as->window, true);
    SDL_SetHintWithPriority(SDL_HINT_WINDOWS_RAW_KEYBOARD, "1", SDL_HINT_OVERRIDE);
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
   AppState *as = (AppState*)appstate;
    int player_count = as->player_count;
    int i;
    switch (event->type) {
        case SDL_EVENT_QUIT:
            return SDL_APP_SUCCESS;
            break;
        case SDL_EVENT_MOUSE_REMOVED:
            for (i = 0; i < player_count; i++) {
                if (as->players[i].mouse == event->mdevice.which) {
                    as->players[i].mouse = 0;
                }
            }
            break;
        case SDL_EVENT_KEYBOARD_REMOVED:
            for (i = 0; i < player_count; i++) {
                if (as->players[i].keyboard == event->kdevice.which) {
                    as->players[i].keyboard = 0;
                }
            }
            break;
        case SDL_EVENT_MOUSE_MOTION: {
            SDL_MouseID id = event->motion.which;
            int index = whoseMouse(id, as->players, as->player_count);
            if (index >= 0) {
                as->players[index].yaw -= ((int)event->motion.xrel) * 0x00080000;
                as->players[index].pitch = SDL_max(-0x40000000, SDL_min(0x40000000, as->players[index].pitch - ((int)event->motion.yrel) * 0x00080000));
            } else if (id) {
                for (i = 0; i < MAX_PLAYER_COUNT; i++) {
                    if (as->players[i].mouse == 0) {
                        as->players[i].mouse = id;
                        as->player_count = SDL_max(as->player_count, i + 1);
                        break;
                    }
                }
            }
            break;
        }
        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            SDL_MouseID id = event->button.which;
            int index = whoseMouse(id, as->players, as->player_count);
            if (index >= 0) {
                shoot(index, as->players, as->player_count);
            }
            break;
        }
        case SDL_EVENT_KEY_DOWN: {
            SDL_Keycode sym = event->key.key;
            SDL_KeyboardID id = event->key.which;
            int index = whoseKeyboard(id, as->players, as->player_count);
            if (index >= 0) {
                if (sym == SDLK_W) as->players[index].wasd |= 1;
                if (sym == SDLK_A) as->players[index].wasd |= 2;
                if (sym == SDLK_S) as->players[index].wasd |= 4;
                if (sym == SDLK_D) as->players[index].wasd |= 8;
                if (sym == SDLK_SPACE) as->players[index].wasd |= 16;
            } else if (id) {
                for (i = 0; i < MAX_PLAYER_COUNT; i++) {
                    if (as->players[i].keyboard == 0) {
                        as->players[i].keyboard = id;
                        as->player_count = SDL_max(as->player_count, i + 1);
                        break;
                    }
                }
            }
            break;
        }
        case SDL_EVENT_KEY_UP: {
            SDL_Keycode sym = event->key.key;
            SDL_KeyboardID id = event->key.which;
            if (sym == SDLK_ESCAPE) return SDL_APP_SUCCESS;
            int index = whoseKeyboard(id, as->players, as->player_count);
            if (index >= 0) {
                if (sym == SDLK_W) as->players[index].wasd &= 30;
                if (sym == SDLK_A) as->players[index].wasd &= 29;
                if (sym == SDLK_S) as->players[index].wasd &= 27;
                if (sym == SDLK_D) as->players[index].wasd &= 23;
                if (sym == SDLK_SPACE) as->players[index].wasd &= 15;
            }
            break;
        }
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    AppState *as = (AppState*)appstate;
    static Uint64 accu = 0;
    static Uint64 last = 0;
    static Uint64 past = 0;
    Uint64 now = SDL_GetTicksNS();
    Uint64 dt_ns = now - past;
    update(as->players, as->player_count, dt_ns);
    draw(as->renderer, (const float (*)[6])as->edges, as->players, as->player_count);
    if (now - last > 999999999) {
        last = now;
        SDL_snprintf(debug_string, sizeof(debug_string), "%" SDL_PRIu64 " fps", accu);
        accu = 0;
    }
    past = now;
    accu += 1;
    Uint64 elapsed = SDL_GetTicksNS() - now;
    if (elapsed < 999999) {
        SDL_DelayNS(999999 - elapsed);
    }
    return SDL_APP_CONTINUE;
}