#include <xcb/xcb.h>

struct desktop {
    xcb_window_t *windows;
    int num;
};

struct arg {
    const char **cmd;
    const uint32_t i;
};

struct key {
    unsigned int mod;
    xcb_keysym_t keysym;
    void (*func)(const struct arg, xcb_window_t);
    const struct arg a;
};
