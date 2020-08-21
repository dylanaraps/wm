#include <xcb/xcb.h>

struct desktop {
    xcb_window_t *windows;
    int num;
};

union arg {
    const char **cmd;
    const uint32_t i;
};

struct key {
    unsigned int mod;
    xcb_keysym_t keysym;
    void (*func)(const union arg *, xcb_window_t);
    const union arg a;
};
