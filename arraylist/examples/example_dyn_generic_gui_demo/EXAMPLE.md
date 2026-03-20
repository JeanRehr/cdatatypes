# Generic GUI Demo Example

This example demonstrates a use-case of the ARRAYLIST_DYN (runtime destructor) version for building flexible, polymorphic systems in C.

It showcases a simple GUI framework (I am assuming immediate-mode) where different screen types and UI components can be managed in a type-safe, memory-safe manner.

I have tried to do some patterns from OOP in C to demonstrate the usage of the ARRAYLIST_DYN version.

It has:
- Polymorphism via function pointers (virtual functions)
- Inheritance through struct composition (base as first member)
- Dynamic dispatch for rendering and cleanup
- Automatic memory management with custom destructors

## Runtime Destructor Benefits

This example highlights why ARRAYLIST_DYN is essential for polymorphic collections:
- Store different screen types (screen1, screen2, etc.) in a single arraylist
- Each screen type has different sizes and cleanup requirements
- Each component's destructor is determined at runtime, not compile-time
- Nested cleanup: screens contain arraylists of components, all cleaned up automatically

There is an arraylist of polymorphic screens, and in each screen there is an arraylist of polymorphic components.

Base Types:
- screen_base - Base interface for all screens
- component_base - Base interface for all UI components

Derived Types:
- screen1, screen2 - Concrete screen implementations
- textbox, button - Concrete UI components

Arraylists:
- arraylist_dyn_screens - Manages screen lifetime
- arraylist_dyn_components - Manages UI component lifetime

I have implemented:

1. Virtual Functions via Function Pointers

Each base type contains function pointers that derived types override:
```c
// All common behavior that a component could have
struct component_base {
    void (*component_render)(struct component_base *self);
    void (*component_deinit)(struct component_base **self_ptr, struct Allocator *alloc);
    const char *component_name;
    size_t allocated_size;
};
```

2. Two-Level Polymorphic Cleanup

When you call dyn_screens_deinit():
1. The arraylist calls screen_base_ptr_dtor for each screen
2. Each screen's screen_deinit is called (polymorphic - screen1 vs screen2)
3. Each screen calls dyn_components_deinit() on its components
4. The components arraylist calls component_base_ptr_dtor for each component
5. Each component's component_deinit is called (polymorphic - textbox vs button)

This results in a complete automatic cleanup of complex nested structures.

3. Mixing Different Types Safely
```c
// Create screens arraylist
struct arraylist_dyn_screens screens = dyn_screens_init(alloc, screen_base_ptr_dtor);

// Add different screen types
struct screen1 *s1 = screen1_init_ptr(1, &alloc);
*dyn_screens_emplace_back(&screens) = &s1->base;

struct screen2 *s2 = screen2_init_ptr(2, &alloc);
*dyn_screens_emplace_back(&screens) = &s2->base;

// Render all screens polymorphically
for (size_t i = 0; i < dyn_screens_size(&screens); ++i) {
    struct screen_base *screen = screens.data[i];
    screen->screen_render(screen); // Calls correct implementation
}

// Clean up everything automatically
dyn_screens_deinit(&screens);// Frees all screens and their components
```

## Compile-Time vs Runtime Destructors

ARRAYLIST (compile-time) would fail here because:
- All components must be cleaned up the same way
- Can't mix screen1 with screen2 with different cleanup logic and potentially different sizes
- Destructor is baked into the implementation at compile time

ARRAYLIST_DYN (runtime) is able to do it because:
- Each component can have its own cleanup logic
- Destructor is stored as a function pointer, determined at runtime
- Enables polymorphic OOP patterns

This pattern is usually done on:
- GUI frameworks - Mix windows, dialogs, panels, widgets
- Game engines - Mix entities, sprites, particles with different behaviors
- Plugin systems - Load different module types with different cleanup
- Event systems - Handle different event types polymorphically
- Resource management - Mix textures, meshes, sounds with automatic cleanup

Of course Data Oriented design would almost always be better performance-wise

## Building and Running

Follow the standard build instructions in the main README, then:

`$ ./build/bin/example_dyn_generic_gui_demo`

## To extend this example

To add a new component type (like listview):
1. Create listview.h with `struct listview { struct component_base base; /* fields */ };`
2. Implement `listview_init_ptr()` that sets `base.component_render` and `base.component_deinit`
3. Add to any screen with `*dyn_components_emplace_back(&screen->components) = &listview->base;`
4. It will automatically render and cleanup polymorphically.

To add a new screen type:
1. Create screen3.h with `struct screen3 { struct screen_base base; /* fields */ };`
2. Implement `screen3_init_ptr()` that sets the function pointers
3. Add to the screens arraylist
4. It will automatically integrate with the render loop.

To add new base functionality/behavior:
1. Add a function pointer to the component or screen base struct.
2. Implement a stub for it in the default constructor of base.
3. Implement it as private (static) in each component and screen.
4. Call it through base pointers through the arraylists.

With this one may add generic behavior that is common on every component or screen, like TAB pressing goes to focus on the next component.

I have made this to demonstrate the usefulness of the ARRAYLIST_DYN type, while also demonstrating that:

1. Function pointers enable polymorphism in C, like virtual functions in C++
2. Runtime destructors enable heterogeneous collections that allows to mix types safely
3. Composition over inheritance in C is always true, as the base as the first member enables safe casting
4. Automatic cleanup is possible in C With proper destructor patterns
5. Type safety through conventions, done manually but effective when done right

This is how one may do generic programming in C
