#include <stdio.h>

// On Linux, when including this file and using kbhit, Carriage Return (\r) is necessary on every usage of \n in until kbhit
// returns non-zero because of raw mode terminal, the terminal doesn't automatically converts \n to \r\n
#include "simulate_rendering.h" // Cross-platform sleep, getch, and kbhit

#include "screens/screen1.h"
#include "screens/screen2.h"
#include "screens/arraylist_screens.h"

int main(void) {
    // General allocator
    struct Allocator alloc = allocator_get_default();

#ifdef __linux__
    set_conio_terminal_mode();
#endif

    // Screens
    // Create an arraylist of generic base screen pointers
    struct arraylist_dyn_screens screens = dyn_screens_init(alloc, screen_base_ptr_dtor);

    // "register" (insert) base screens into the arraylist
    struct screen1 *s1 = screen1_init_ptr(1, &alloc);
    *dyn_screens_emplace_back(&screens) = &s1->base;

    struct screen2 *s2 = screen2_init_ptr(2, &alloc);
    *dyn_screens_emplace_back(&screens) = &s2->base;

    // "rendering" loop
    while(!kbhit()) {
        printf("Press any key to exit the loop.\r\n");

        // Cycle through screens
        for (size_t i = 0; i < dyn_screens_size((&screens)); ++i) {
            // Get screen from screens dynamic array
            // struct screen_base *screen = *dyn_screens_at(&screens, i);

            // Or access the buffer directly (no bounds checking)
            struct screen_base *screen = screens.data[i];

            // Function pointer call dispatches to the correct implementation of the concrete screen type at index i 
            screen->screen_render(screen);

            // May call it directly as well
            // screens.data[i]->screen_render(screens.data[i]); // Or calling the at function

            // If this was a real implementation, one would get the current screen the user is looking at (maybe by id
            // or render directly from user struct) and only render that screen, keeping the screens sorted with
            // arraylist qsort would make it faster to access by id (I think), so there wouldn't be a loop through
            // screens size, but just get user->current_screen.base.screen_render or something like that
            // Right now, it is rendering all screens one at a time.
            printf("\r\n");
        }

        sleep(1);
    }

    (void)getch(); /* consume the character */

    // Cast screens concrete type to screen_base ** to call deinit
    // s1->base.screen_deinit((struct screen_base **)&s1, &alloc);
    // s2->base.screen_deinit((struct screen_base **)&s2, &alloc);

    // Call the destructor
    // Runtime destructor function screen_base_ptr_dtor from arraylist_screen.c dispatches to the
    // correct implementation of deinit function
    dyn_screens_deinit(&screens);

    // There could be a screens manager to move all this screen managing code to its own file maybe
    return 0;
}
