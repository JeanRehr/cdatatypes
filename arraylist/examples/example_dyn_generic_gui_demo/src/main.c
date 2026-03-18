#include <stdio.h>
#include "simulate_rendering.h" // Cross-platform sleep, getch, and kbhit

int main(void) {
#ifdef __linux__
    set_conio_terminal_mode();    
#endif

    // Carriage Return (\r) is necessary on every usage of \n because of raw mode terminal
    // The terminal doesn't automatically converts \n to \r\n

    while(!kbhit()) {
        printf("Press any key to exit the loop.\r\n");
        sleep(1);
    }

    (void)getch(); /* consume the character */

    printf("%s\r\n", __FILE__);
    return 0;
}
