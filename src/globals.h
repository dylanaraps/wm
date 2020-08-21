#include <xcb/xcb.h>

extern xcb_connection_t *dpy;
extern xcb_screen_t *scr;

struct desktop {
    xcb_window_t **windows;
    int num;
};

union arg {
    const char **cmd;
    const uint32_t i;
};

struct key {
    unsigned int mod;
    xcb_keysym_t keysym;
    void (*func)(const union arg *);
    const union arg a;
};

extern int current_desktop;
extern struct desktop **desktops;
