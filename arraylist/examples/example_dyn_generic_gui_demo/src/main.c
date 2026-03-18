#include <stdio.h>

/* ------------------------------------------- */
/* All of this is to simulate a rendering loop */
/* ------------------------------------------- */
// cross-platform sleep
#ifdef _WIN32
    #include <windows.h> // For Sleep
#else
    #include <unistd.h> // For sleep
#endif

// Try to simulate kbhit from Windows
// Taken from https://stackoverflow.com/questions/448944/c-non-blocking-keyboard-input
#ifdef __linux__
    #include <stdlib.h>
    #include <string.h>
    #include <unistd.h>
    #include <sys/select.h>
    #include <termios.h>

    struct termios orig_termios;

    void reset_terminal_mode(void) {
        tcsetattr(0, TCSANOW, &orig_termios);
    }

    void set_conio_terminal_mode(void) {
        struct termios new_termios;

        /* take two copies - one for now, one for later */
        tcgetattr(0, &orig_termios);
        memcpy(&new_termios, &orig_termios, sizeof(new_termios));

        /* register cleanup handler, and set the new terminal mode */
        atexit(reset_terminal_mode);
        cfmakeraw(&new_termios);
        tcsetattr(0, TCSANOW, &new_termios);
    }

    int kbhit(void) {
        struct timeval tv = { 0L, 0L };
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(0, &fds);
        return select(1, &fds, NULL, NULL, &tv) > 0;
    }

    int getch(void) {
        int r;
        unsigned char c;
        if ((r = read(0, &c, sizeof(c))) < 0) {
            return r;
        } else {
            return c;
        }
    }
#elif _WIN32
    #include <conio.h>
#endif

// Just to sleep a little to simulate a rendering on screen
void sleep_secs(size_t secs) {
#ifdef _WIN32
    Sleep(secs * 1000);
#else
    sleep(secs);
#endif
}
/* ------------------------------------------- */

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
