#include <xcb/xcb.h>

struct window {
    xcb_window_t id;
    xcb_get_geometry_reply_t *geom;
    int is_fs;
};

struct desktop {
    struct window *windows;
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
