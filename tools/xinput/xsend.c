/* xsend: scripted X11 keyboard/mouse input for testing the windowed build under Xvfb.
 * Reads commands from stdin, one per line:
 *   key NAME down|up|tap     (X keysym name: w, Return, Escape, Control_L, ...)
 *   button N down|up|tap     (1 left, 2 middle, 3 right, 4/5 wheel)
 *   move DX DY               (relative pointer motion)
 *   wait SECONDS
 *   shot FILE.png            (runs `import -window root FILE.png`)
 * Build: cc -O2 xsend.c -o xsend -lX11 -l:libXtst.so.6  (no XTest headers needed). */
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int XTestFakeKeyEvent(Display*, unsigned int, Bool, unsigned long);
int XTestFakeButtonEvent(Display*, unsigned int, Bool, unsigned long);
int XTestFakeRelativeMotionEvent(Display*, int, int, unsigned long);

int main(void) {
    Display* d = XOpenDisplay(NULL);
    if (!d) { fprintf(stderr, "xsend: no display\n"); return 1; }
    char line[256];
    while (fgets(line, sizeof line, stdin)) {
        char cmd[32] = {0}, a[128] = {0}, b[32] = {0};
        if (sscanf(line, "%31s %127s %31s", cmd, a, b) < 1) continue;
        if (!strcmp(cmd, "wait")) { usleep((useconds_t)(atof(a) * 1e6)); continue; }
        if (!strcmp(cmd, "shot")) { char c[256]; snprintf(c, sizeof c, "import -window root %s", a); if (system(c)) {} continue; }
        if (!strcmp(cmd, "move")) { XTestFakeRelativeMotionEvent(d, atoi(a), atoi(b), 0); XFlush(d); continue; }
        int down = !strcmp(b, "down"), tap = !strcmp(b, "tap");
        if (!strcmp(cmd, "key")) {
            KeyCode kc = XKeysymToKeycode(d, XStringToKeysym(a));
            if (!kc) { fprintf(stderr, "xsend: unknown key %s\n", a); continue; }
            if (tap) { XTestFakeKeyEvent(d, kc, True, 0); XFlush(d); usleep(80000); XTestFakeKeyEvent(d, kc, False, 0); }
            else XTestFakeKeyEvent(d, kc, down, 0);
        } else if (!strcmp(cmd, "button")) {
            unsigned n = (unsigned)atoi(a);
            if (tap) { XTestFakeButtonEvent(d, n, True, 0); XFlush(d); usleep(80000); XTestFakeButtonEvent(d, n, False, 0); }
            else XTestFakeButtonEvent(d, n, down, 0);
        }
        XFlush(d);
    }
    XCloseDisplay(d);
    return 0;
}
