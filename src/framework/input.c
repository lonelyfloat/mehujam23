#include "input.h"
#include "dev_imgui.h"
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <assert.h>

#define MAX_GAMEPAD_CAPACITY 16

InputSystem* InitInputSystem(Arena* arena, size_t capacity, size_t maxSources, InputTypeFlags inputTypes) {
    InputSystem* result = ArenaAllocStruct(arena, InputSystem);
    // Initialize Inputworld global structure
    result->state.mouseButtons = 0;
    result->state.mousePosition = (Vector2){};
    result->state.keyboardScancodes = NULL;
    result->state.flags = inputTypes;
    result->state.defaultFlags = inputTypes;
    result->state.gamepadCount = 0;
    if(inputTypes & INPUT_TYPE_GAMEPAD) { 
        result->state.gamepads = ArenaAllocArray(arena, SDL_Gamepad*, MAX_GAMEPAD_CAPACITY);
    }
    result->capacity = capacity;
    result->controls = ArenaAllocArray(arena, Input, capacity);
    result->count = 0;
    for(int i = 0; i < result->capacity; ++i) {
        result->controls[i].capacity = maxSources;
        result->controls[i].count = 0;
        result->controls[i].sources = ArenaAllocArray(arena, InputSource,maxSources);
        result->controls[i].output = (InputResult){ 0 };
    }

    return result;
}

InputID AddInput(InputSystem* registry) { 
    InputID new = (InputID){ registry->count };
    // NOTE: I don't care about resizing the table here ATM b/c the amount of game input bindings won't change at runtime
    registry->count++;
    return new;
}

int AddInputSource(InputSystem* registry, InputID input, InputSource source) {
    registry->controls[input.idx].sources[registry->controls[input.idx].count] = source;
    int idx = registry->controls[input.idx].count;
    registry->controls[input.idx].count++;
    if(registry->controls[input.idx].count == registry->controls[input.idx].capacity) {
        fprintf(stderr, "ERROR: Input source count exceeds capacity!\n");
    }
    return idx;
}

void RemoveSource(InputSystem* registry, InputID input, int sourceIdx) {
    registry->controls[input.idx].count--;
    if(registry->controls[input.idx].count < 0) {
        fprintf(stderr, "ERROR: Input source count < 0!\n");
    }
    registry->controls[input.idx].sources[sourceIdx] = registry->controls[input.idx].sources[registry->controls[input.idx].count];
}



static void CheckInputContinuous(InputSystem* registry, Input* control) {
    InputResult result = {control->output.justPressed, control->output.justReleased, 0.0};
    for(int s = 0; s < control->count; ++s) {
        InputSource source = control->sources[s];
        switch(source.type) {
            case INPUT_SOURCE_MOUSE_BUTTON: {
                if(registry->state.flags & INPUT_TYPE_MOUSE) {
                    if(result.continuous == 0) {
                        result.continuous = SDL_BUTTON_MASK(source.data.mouseButton.button) & registry->state.mouseButtons ? 1 : 0;
                    }
                }
                break;
            }
            case INPUT_SOURCE_MOUSE_WHEEL: {
                if(registry->state.flags & INPUT_TYPE_MOUSE) {
                    if(result.continuous == 0) {
                        if(source.data.mouseWheel.isHorizontal) {
                            result.continuous = registry->state.mouseWheel.x;
                        } else {
                            result.continuous = registry->state.mouseWheel.y;
                        }
                    }
                }
                break;
            }
            case INPUT_SOURCE_KEYBOARD_SCANCODE: {
                if(registry->state.flags & INPUT_TYPE_KEYBOARD) {
                    if(result.continuous == 0) {
                        result.continuous = registry->state.keyboardScancodes[source.data.scancodeKey.scancode] ? 1 : 0;
                    }
                }
                break;
            }
            case INPUT_SOURCE_KEYBOARD_KEYCODE: {
                if(registry->state.flags & INPUT_TYPE_KEYBOARD) {
                    if(result.continuous == 0) {
                        result.continuous = registry->state.keyboardScancodes[SDL_GetScancodeFromKey(source.data.scancodeKey.scancode,NULL)] ? 1 : 0;
                    }
                }
                break;
            }
            case INPUT_SOURCE_GAMEPAD_BUTTON: {
                if(registry->state.flags & INPUT_TYPE_GAMEPAD) {
                    if(result.continuous == 0) {
                        result.continuous = SDL_GetGamepadButton(registry->state.gamepads[source.data.gamepadButton.gamepadIndex], source.data.gamepadButton.button);
                    }
                }
                break;
            }
            case INPUT_SOURCE_GAMEPAD_AXIS: {
                if(registry->state.flags & INPUT_TYPE_GAMEPAD) {
                    short axis = SDL_GetGamepadAxis(registry->state.gamepads[source.data.gamepadButton.gamepadIndex], source.data.gamepadAxis.axis);
                    float normalized = (float)axis/SDL_JOYSTICK_AXIS_MAX; // value from [-1..1]
                    if(fabs(normalized) < source.data.gamepadAxis.deadzone) {
                        normalized = 0;
                    }
                    if(normalized < 0 && source.data.gamepadAxis.negative) {
                        result.continuous = -normalized;
                    }
                    if(normalized > 0 && !source.data.gamepadAxis.negative) {
                        result.continuous = normalized;
                    }
                }
                break;
            }
            case INPUT_SOURCE_TYPE_COUNT:
                break;
        }
    }
    if(result.continuous > 0 && control->output.continuous < 1) {
        control->output.justPressed = true;
    } else {
        control->output.justPressed = false;
    }
    if(result.continuous < 1 && control->output.continuous > 0) {
        control->output.justReleased = true;
    } else {
        control->output.justReleased = false;
    }

    control->output.continuous = result.continuous;
}

static void PollInputRegistry(InputSystem* registry, SDL_Event* event) {
    // Do Gamepad hot-plugging
    if(event->type == SDL_EVENT_GAMEPAD_ADDED) {
        if(registry->state.gamepadCount > MAX_GAMEPAD_CAPACITY) {
                SDL_Log("Gamepad count exceeds capacity!");
        }
        if(registry->state.gamepads[registry->state.gamepadCount] == NULL) {
            registry->state.gamepads[registry->state.gamepadCount] = SDL_OpenGamepad(event->gdevice.which); 
            if(!registry->state.gamepads[registry->state.gamepadCount]) {
                SDL_Log("Failed to open gamepad ID %u: %s", (unsigned int) event->gdevice.which, SDL_GetError());
            }
            registry->state.gamepadCount++;
        }
        
    }
    if(event->type == SDL_EVENT_GAMEPAD_REMOVED) {
        for(int i = 0; i < registry->state.gamepadCount; ++i) {
            if (registry->state.gamepads[i] && (SDL_GetGamepadID(registry->state.gamepads[i]) == event->gdevice.which)) {
                SDL_CloseGamepad(registry->state.gamepads[i]);
                registry->state.gamepads[i] = registry->state.gamepads[registry->state.gamepadCount-1];
                registry->state.gamepads[registry->state.gamepadCount-1] = NULL;
                registry->state.gamepadCount--;
            }
        }
    }

    // Check mouse wheel event
    if(event->type == SDL_EVENT_MOUSE_WHEEL) {
        registry->state.mouseWheel.x = event->wheel.x;
        registry->state.mouseWheel.y = event->wheel.y;
    }
}

static void ResetInputState(InputSystem* registry) {
    registry->state.mouseWheel.x = 0;
    registry->state.mouseWheel.y = 0;
    for(int i = 0; i < registry->count; ++i) {
        Input* c = &registry->controls[i];
        c->output.justPressed = false;
        c->output.justReleased = false;
    }
}

static void UpdateContinuousInput(InputSystem* registry) {
    if(registry->state.flags & INPUT_TYPE_KEYBOARD) {
        registry->state.keyboardScancodes = SDL_GetKeyboardState(NULL);
        registry->state.keyboardModState = SDL_GetModState();
    }
    if(registry->state.flags & INPUT_TYPE_MOUSE) {
        registry->state.mouseButtons = SDL_GetMouseState(&registry->state.mousePosition.x, &registry->state.mousePosition.y);
    }
    for(int i = 0; i < registry->count; ++i) {
        Input* c = &registry->controls[i];
        CheckInputContinuous(registry, c);
    }
}


static void MaskImGuiCapturedInput(InputSystem* registry) {
#ifdef DEBUG_MODE
    ImGuiIO* io = ImGui_GetIO();
    if(io->WantCaptureMouse) {
        registry->state.flags = registry->state.flags & (~INPUT_TYPE_MOUSE);
    } else {
        registry->state.flags |= (registry->state.defaultFlags & INPUT_TYPE_MOUSE);
    }
    if(io->WantCaptureKeyboard) {
        registry->state.flags = registry->state.flags & (~INPUT_TYPE_KEYBOARD);
    } else {
        registry->state.flags |= (registry->state.defaultFlags & INPUT_TYPE_KEYBOARD);
    }
#endif
}

bool PollInputSystem(InputSystem* registry) {
    SDL_Event e;
    bool quitGame = false;
    ResetInputState(registry);
    MaskImGuiCapturedInput(registry);
    while(SDL_PollEvent(&e)) {
        cImGui_ImplSDL3_ProcessEvent(&e);
        PollInputRegistry(registry, &e);
        if(e.type == SDL_EVENT_QUIT) {
            quitGame = true;
            break;
        }
    }
    
    UpdateContinuousInput(registry);
    return quitGame;
}

InputResult GetInput(InputSystem* registry, InputID input) {
    return registry->controls[input.idx].output;
}
