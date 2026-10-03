# TODO: 
 - Put simple color palette in `draw.h`
 - Texture resource cache
 - Redo file/folder structuring
 - Look into service locators for 'global' utilities (e.g. InputRegistry, AudioEngine)

# MAYBE:
 - More robust FX processing on AudioBus
    - Pre-designed FX (volume, low pass, delay/verb, etc.) that can be easily applied to a bus
 - Add a second "backend" (SDLGPU)


# DONE:
 - Created 'GameCalls' system to replace old hot reloading system, allowed for game to preserve state between reloads
    - (MAYBE) Change ReloadCalls system into some sort of "Application" struct (might have some pseudo-global state in there?)
    - *At least* make ReloadCalls less solely bound to hot reloading since it is used *every time* the update method is called.
 - Audio System (BIG)
    - Named internal audio channels (e.g. 'SFX', 'Music') <- This is not how it works exactly
        - Capabilities for "adaptive soundtracking" <- You can dynamically change track volume and theoretically run each "bus" through arbitrary DSP effects
    - Built in multi sample SFX support (psuedo-random sound selection from a bank?) <- works
 - Abstraction around fixed-timestep functions
 - Procedural animation:
    - 2D 2-bone IK
    - FABRIK
    - Angle limiting on verlet physics
 - Separating Axis Theorem utilities for 2D collisions
 - Resource system
    - Refcounted resource handles
    - Use ResourceCache in audio system to allow sounds to be dynamically unloaded
