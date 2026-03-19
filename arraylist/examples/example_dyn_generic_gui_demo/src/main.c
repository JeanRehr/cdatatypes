#include <stdio.h>
#include "simulate_rendering.h" // Cross-platform sleep, getch, and kbhit

#include "screens/screen1.h"
#include "screens/screen2.h"

int main(void) {
    // General allocator
    struct Allocator alloc = allocator_get_default();

#ifdef __linux__
    set_conio_terminal_mode();    
#endif

    // Screens
    struct screen1 *s1 = screen1_init_ptr(1, &alloc);
    struct screen2 *s2 = screen2_init_ptr(2, &alloc);

    // Carriage Return (\r) is necessary on every usage of \n because of raw mode terminal
    // The terminal doesn't automatically converts \n to \r\n

    // "rendering" loop
    while(!kbhit()) {
        printf("Press any key to exit the loop.\r\n");

        s1->base.screen_render(&s1->base);
        printf("\r\n");
        s2->base.screen_render(&s2->base);

        printf("\r\n");
        sleep(1);
    }

    (void)getch(); /* consume the character */

    // Cast screens concrete type to screen_base ** to call deinit
    s1->base.screen_deinit((struct screen_base **)&s1, &alloc);
    s2->base.screen_deinit((struct screen_base **)&s2, &alloc);
    return 0;
}
