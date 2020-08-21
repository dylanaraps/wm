#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include <X11/keysym.h>

#include "types.h"
#include "vec.h"

#define LEN(x)    (sizeof(x) / sizeof(*x))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

static int id_to_window(xcb_window_t);

static void action_center(const struct arg, xcb_window_t);
static void action_execute(const struct arg a, xcb_window_t w);
static void action_fullscreen(const struct arg, xcb_window_t);
static void action_kill(const struct arg, xcb_window_t);
static void action_workspace(const struct arg, xcb_window_t);
static void action_workspace_send(const struct arg, xcb_window_t);

static void event_button_press(xcb_generic_event_t *ev);
static void event_button_release(xcb_generic_event_t *ev);
static void event_key_press(xcb_generic_event_t *ev);
static void event_notify_create(xcb_generic_event_t *ev);
static void event_notify_destroy(xcb_generic_event_t *ev);
static void event_notify_enter(xcb_generic_event_t *ev);
static void event_notify_motion(xcb_generic_event_t *ev);

static void init_wm(void);
static void init_mouse(void);
static void init_keys(void);
static void init_desktops(void);

#include "config.h"

xcb_connection_t *dpy;
xcb_screen_t *scr;

static int cur_ws = 0;
static struct desktop **desktops;
static xcb_window_t motion_win;

void (*events[XCB_NO_OPERATION])(xcb_generic_event_t *) = {
    [XCB_BUTTON_PRESS]   = event_button_press,
    [XCB_BUTTON_RELEASE] = event_button_release,
    [XCB_KEY_PRESS]      = event_key_press,
    [XCB_CREATE_NOTIFY]  = event_notify_create,
    [XCB_DESTROY_NOTIFY] = event_notify_destroy,
    [XCB_ENTER_NOTIFY]   = event_notify_enter,
    [XCB_MOTION_NOTIFY]  = event_notify_motion
};

static int id_to_window(xcb_window_t w) {
    if (!w || w == scr->root) {
        return -1;
    }

    for (unsigned int i = 0; i < vec_size(desktops[cur_ws]->windows); ++i) {
        if (w == desktops[cur_ws]->windows[i].id) {
            return i;
        }
    }

    return -1;
}

static void action_center(const struct arg a, xcb_window_t w) {
    struct window win;
    uint32_t values[2];
    int wid;

    (void)(a);

    wid = id_to_window(w);

    if (wid == -1) {
        return;
    }

    win = desktops[cur_ws]->windows[wid];

    values[0] = (scr->width_in_pixels - win.geom->width) / 2;
    values[1] = (scr->height_in_pixels - win.geom->height) / 2;

    xcb_configure_window(dpy, w,
        XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, values);
}

static void action_execute(const struct arg a, xcb_window_t w) {
    (void)(w);

    if (fork()) {
        return;
    }

    setsid();
    execvp((char *)a.cmd[0], (char **)a.cmd);
}

static void action_fullscreen(const struct arg a, xcb_window_t w) {
    struct window *win;
    uint32_t values[4];
    int wid;

    (void)(a);

    if (w == scr->root) {
        return;
    }

    wid = id_to_window(w);

    if (wid == -1) {
        return;
    }

    win = &desktops[cur_ws]->windows[wid];

    if ((win->is_fs = win->is_fs ? 0 : 1)) {
        values[0] = win->geom->x;
        values[1] = win->geom->y;
        values[2] = win->geom->width;
        values[3] = win->geom->height;

    } else {
        values[0] = 0;
        values[1] = 0;
        values[2] = scr->width_in_pixels;
        values[3] = scr->height_in_pixels;

        win->geom = xcb_get_geometry_reply(dpy, xcb_get_geometry(dpy, w), NULL);
    }

    xcb_configure_window(dpy, w,
        XCB_CONFIG_WINDOW_X     | XCB_CONFIG_WINDOW_Y |
        XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
        values);
}

static void action_kill(const struct arg a, xcb_window_t w) {
    (void)(a);

    xcb_destroy_window(dpy, w);
}

static void action_workspace(const struct arg a, xcb_window_t w) {
    (void)(a);
    (void)(w);
}

static void action_workspace_send(const struct arg a, xcb_window_t w) {
    (void)(a);
    (void)(w);
}

void event_button_press(xcb_generic_event_t *ev) {
    xcb_button_press_event_t *e = (xcb_button_press_event_t *)ev;
    xcb_get_geometry_reply_t *geom;
    uint32_t value;

    if (!e->child) {
        return;
    }

    motion_win = e->child;
    value = XCB_STACK_MODE_ABOVE;

    xcb_configure_window(dpy, e->child,
        XCB_CONFIG_WINDOW_STACK_MODE, &value);

    geom = xcb_get_geometry_reply(dpy,
        xcb_get_geometry(dpy, e->child), NULL);

    xcb_warp_pointer(dpy, XCB_NONE, e->child, 0, 0, 0, 0,
        e->detail != 1 ? geom->width : 1,
        e->detail != 1 ? geom->height : 1);

    xcb_grab_pointer(dpy, 0, scr->root, XCB_EVENT_MASK_BUTTON_RELEASE |
        XCB_EVENT_MASK_BUTTON_MOTION | XCB_EVENT_MASK_POINTER_MOTION_HINT,
        XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, scr->root, XCB_NONE,
        XCB_CURRENT_TIME);
}

void event_button_release(xcb_generic_event_t *ev) {
    int wid;

    (void)(ev);

    xcb_ungrab_pointer(dpy, XCB_CURRENT_TIME);

    wid = id_to_window(motion_win);

    desktops[cur_ws]->windows[wid].geom =
        xcb_get_geometry_reply(dpy,
            xcb_get_geometry(dpy, motion_win), NULL);

    motion_win = 0;
}

void event_key_press(xcb_generic_event_t *ev) {
    xcb_key_press_event_t *e = (xcb_key_press_event_t *)ev;
	xcb_key_symbols_t *keysyms;
    xcb_keysym_t keysym;

    keysyms = xcb_key_symbols_alloc(dpy);

    if (!keysyms) {
        return;
    }

    keysym = xcb_key_symbols_get_keysym(keysyms, e->detail, 0);

    /* todo mask crap */
    for (unsigned int i = 0; i < LEN(keys); i++) {
        if (keysym == keys[i].keysym && keys[i].func) {
            keys[i].func(keys[i].a, e->child);
            break;
        }
    }

    xcb_key_symbols_free(keysyms);
}

void event_notify_create(xcb_generic_event_t *ev) {
    xcb_create_notify_event_t *e = (xcb_create_notify_event_t *)ev;
    uint32_t value;

    struct window new = {
        .id    = e->window,
        .is_fs = 1,
        .geom  = xcb_get_geometry_reply(dpy,
                     xcb_get_geometry(dpy, e->window), NULL),
    };

    value = XCB_EVENT_MASK_ENTER_WINDOW |
            XCB_EVENT_MASK_FOCUS_CHANGE |
            XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;

    xcb_change_window_attributes(dpy, e->window, XCB_CW_EVENT_MASK, &value);
    xcb_map_window(dpy, e->window);

    xcb_set_input_focus(dpy, XCB_INPUT_FOCUS_POINTER_ROOT,
        e->window, XCB_CURRENT_TIME);

    vec_push_back(desktops[cur_ws]->windows, new);
    action_center((struct arg){0}, e->window);
}

void event_notify_destroy(xcb_generic_event_t *ev) {
    xcb_destroy_notify_event_t *e = (xcb_destroy_notify_event_t *)ev;

    for (size_t i = 0; i < vec_size(desktops[cur_ws]->windows); ++i) {
        if (e->window == desktops[cur_ws]->windows[i].id) {
            vec_erase(desktops[cur_ws]->windows, i);
        }
    }
}

void event_notify_enter(xcb_generic_event_t *ev) {
    xcb_enter_notify_event_t *e = (xcb_enter_notify_event_t *)ev;

    if (!e->event || e->event == scr->root) {
        return;
    }

    xcb_set_input_focus(dpy, XCB_INPUT_FOCUS_POINTER_ROOT,
        e->event, XCB_CURRENT_TIME);
}

void event_notify_motion(xcb_generic_event_t *ev) {
    xcb_motion_notify_event_t *e = (xcb_motion_notify_event_t *)ev;
    xcb_query_pointer_reply_t *ptr;
    xcb_get_geometry_reply_t *geom;
    uint32_t values[2];

    if (!motion_win || motion_win == scr->root) {
        return;
    }

    ptr = xcb_query_pointer_reply(dpy, xcb_query_pointer(dpy, scr->root), 0);
    geom = xcb_get_geometry_reply(dpy, xcb_get_geometry(dpy, motion_win), NULL);

    /* move */
    if (e->state & XCB_BUTTON_MASK_1) {
        values[0] = (ptr->root_x + geom->width > scr->width_in_pixels)?
            (scr->width_in_pixels - geom->width):ptr->root_x;

        values[1] = (ptr->root_y + geom->height > scr->height_in_pixels)?
            (scr->height_in_pixels - geom->height):ptr->root_y;

        xcb_configure_window(dpy, motion_win,
            XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, values);

    /* resize */
    } else if (e->state & XCB_BUTTON_MASK_3) {
        values[0] = MAX(10, ptr->root_x - geom->x);
        values[1] = MAX(10, ptr->root_y - geom->y);

        xcb_configure_window(dpy, motion_win,
            XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
    }
}

static void init_wm() {
    uint32_t values;

    dpy = xcb_connect(NULL, NULL);

    if (xcb_connection_has_error(dpy)) {
        printf("error: Failed to start sowm\n");
        exit(1);
    }

    scr = xcb_setup_roots_iterator(xcb_get_setup(dpy)).data;

    if (!scr) {
        printf("error: Failed to assign screen number\n");
        xcb_disconnect(dpy);
        exit(1);
    }

    values = XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;

    xcb_change_window_attributes_checked(dpy, scr->root,
        XCB_CW_EVENT_MASK, &values);

    xcb_flush(dpy);
}

static void init_mouse() {
    xcb_grab_key(dpy, 1, scr->root, SOWM_MOD, XCB_NO_SYMBOL,
        XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);

    xcb_grab_button(dpy, 0, scr->root, XCB_EVENT_MASK_BUTTON_PRESS |
        XCB_EVENT_MASK_BUTTON_RELEASE, XCB_GRAB_MODE_ASYNC,
        XCB_GRAB_MODE_ASYNC, scr->root, XCB_NONE, 1, SOWM_MOD);

    xcb_grab_button(dpy, 0, scr->root, XCB_EVENT_MASK_BUTTON_PRESS |
        XCB_EVENT_MASK_BUTTON_RELEASE, XCB_GRAB_MODE_ASYNC,
        XCB_GRAB_MODE_ASYNC, scr->root, XCB_NONE, 3, SOWM_MOD);

    xcb_flush(dpy);
}

static void init_keys() {
    xcb_key_symbols_t *keysyms;
    xcb_keycode_t *keycode;

    xcb_ungrab_key(dpy, XCB_GRAB_ANY, scr->root, XCB_MOD_MASK_ANY);

    keysyms = xcb_key_symbols_alloc(dpy);

    if (!keysyms) {
        printf("error: Failed to grab keyboard\n");
        xcb_disconnect(dpy);
        exit(1);
    }

    for (unsigned int i = 0; i < LEN(keys); i++) {
        keycode = xcb_key_symbols_get_keycode(keysyms, keys[i].keysym);

        /* todo handle numlock etc fuck me */
        for (int j = 0; keycode[j] != XCB_NO_SYMBOL; j++) {
            xcb_grab_key(dpy, 1, scr->root, keys[i].mod, keycode[j],
                XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
        }

        free(keycode);
    }

    xcb_key_symbols_free(keysyms);
}

static void init_desktops() {
    struct desktop new = {0};

    for (int i = 0; i < SOWM_NUM_DESKTOPS; i++) {
        new.num = i;
        vec_push_back(desktops, &new);
    }
}

int main(int argc, char **argv) {
    xcb_generic_event_t *ev;
    unsigned int ev_type;

    (void) argc;
    (void) argv;

    signal(SIGCHLD, SIG_IGN);

    init_wm();
    init_mouse();
    init_keys();
    init_desktops();

    while ((ev = xcb_wait_for_event(dpy))) {
        ev_type = ev->response_type & ~0x80;

        if (events[ev_type]) {
            events[ev_type](ev);
            xcb_flush(dpy);
        }

        free(ev);
    }

    /* todo atexit + destroy func */
    for (int i = 0; i < SOWM_NUM_DESKTOPS; i++) {
        vec_free(desktops[i]->windows);
    }
    vec_free(desktops);

    return 0;
}
