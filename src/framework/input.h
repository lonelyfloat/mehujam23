#ifndef H_INPUT_BINDINGS
#define H_INPUT_BINDINGS

#include <SDL3/SDL.h>
#include <SDL3/SDL_mouse.h>

#include "types.h"
#include "arena.h"

// System for rebindable inputs w/ multiple control sources

typedef enum InputSourceType {
    INPUT_SOURCE_MOUSE_BUTTON,
    INPUT_SOURCE_MOUSE_WHEEL,
    INPUT_SOURCE_KEYBOARD_SCANCODE,
    INPUT_SOURCE_KEYBOARD_KEYCODE,
    INPUT_SOURCE_GAMEPAD_BUTTON,
    INPUT_SOURCE_GAMEPAD_AXIS,
    INPUT_SOURCE_TYPE_COUNT
} InputSourceType;

// <BUTTON/AXIS> = <STATE OF THING> 
//  Physical R Key = Down
//  Gamepad Axis 1 = Left

// Big ol' tagged union for different input types. I know it's ugly AF, but it works
typedef struct InputSource {
    InputSourceType type;
    union {
        struct {
            SDL_MouseButtonFlags button;
        } mouseButton;
        struct {
            bool isHorizontal;
        } mouseWheel;
        struct {
            SDL_Scancode scancode;
        } scancodeKey;
        struct {
            SDL_Keycode keycode;
            SDL_Keymod mod;
        } keycodeKey;
        struct {
            uint8_t gamepadIndex;
            SDL_GamepadButton button;
        } gamepadButton;
        struct {
            uint8_t gamepadIndex; // Index into GlobalInputState gamepad stack
            SDL_GamepadAxis axis;
            float deadzone;
            bool negative; // negative or positive part of the axis
        } gamepadAxis;
    } data;
} InputSource;

typedef struct InputID {
    uint32_t idx;
} InputID;

typedef struct InputResult {
    bool justPressed;
    bool justReleased;
    float continuous;
} InputResult;

// stack of input sources that all converge to the same output value
typedef struct Input {
    InputSource* sources;
    size_t count;
    size_t capacity;
    InputResult output;
} Input;

typedef enum InputTypeFlags {
    INPUT_TYPE_KEYBOARD = 1,
    INPUT_TYPE_MOUSE    = 1 << 1,
    INPUT_TYPE_GAMEPAD  = 1 << 2,
} InputTypeFlags;

typedef struct InputWorld { // State of *all* the input
    InputTypeFlags flags;
    InputTypeFlags defaultFlags;
    // Keyboard
    const bool* keyboardScancodes;
    SDL_Keymod keyboardModState;
    // Mouse
    Vector2 mousePosition;
    Vector2 mouseWheel;
    uint8_t mouseButtons; // mask with 5 values (see SDL_mouse.h) 
    // Stack of pointers to SDL_Gamepad objects (should work reasonably ok for simple use cases)
    SDL_Gamepad** gamepads; // Max gamepad capacity = 32
    int gamepadCount;
} InputWorld;


// Table of input IDs to lists of input bindings, also stores input data itself
typedef struct InputSystem {
    Arena* arena;
    InputWorld state;
    Input* controls;
    size_t count;
    size_t capacity;
} InputSystem;

InputSystem* InitInputSystem(Arena* arena, size_t capacity, size_t maxSources, InputTypeFlags inputTypes);
InputID AddInput(InputSystem* registry);
int AddInputSource(InputSystem* registry, InputID id, InputSource source);
void RemoveInputSource(InputSystem* registry, InputID id, int sourceIdx);
bool PollInputSystem(InputSystem* registry); // Returns true if SDL_QUIT event was captured
InputResult GetInput(InputSystem* registry, InputID input);

static inline InputSource MouseButton(SDL_MouseButtonFlags button) {
    return (InputSource) {
        .type = INPUT_SOURCE_MOUSE_BUTTON,
        .data.mouseButton = {
            .button = button
        }
    };
}

static inline InputSource MouseWheel(bool isHorizontal) {
    return (InputSource) {
        .type = INPUT_SOURCE_MOUSE_WHEEL,
        .data.mouseWheel = {
            .isHorizontal = isHorizontal
        }
    };
}

static inline InputSource ScancodeKey(SDL_Scancode scancode) {
    return (InputSource) {
        .type = INPUT_SOURCE_KEYBOARD_SCANCODE,
        .data.scancodeKey = {
            .scancode = scancode
        }
    };
}

static inline InputSource KeycodeKey(SDL_Keycode keycode, SDL_Keymod mod) {
    return (InputSource) {
        .type = INPUT_SOURCE_KEYBOARD_KEYCODE,
        .data.keycodeKey = {
            .keycode = keycode,
            .mod = mod
        }
    };
}

static inline InputSource GamepadButton(SDL_GamepadButton button, uint8_t gamepadIndex) {
    return (InputSource) {
        .type = INPUT_SOURCE_GAMEPAD_BUTTON,
        .data.gamepadButton = {
            .gamepadIndex = gamepadIndex,
            .button = button
        }
    };
}

static inline InputSource GamepadAxis(SDL_GamepadAxis axis, bool negative, float deadzone, uint8_t gamepadIndex) {
    return (InputSource) {
        .type = INPUT_SOURCE_GAMEPAD_AXIS,
        .data.gamepadAxis = {
            .gamepadIndex = gamepadIndex,
            .axis = axis,
            .deadzone = deadzone,
            .negative = negative
        }
    };
}

#endif
