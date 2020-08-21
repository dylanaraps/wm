#include <xcb/xcb.h>
#include <X11/keysym.h>

#include "globals.h"

#define SOWM_MOD XCB_MOD_MASK_1
#define SOWM_NUM_DESKTOPS 6

struct key keys[] = {
    {SOWM_MOD, XK_q, NULL, {0}},
    {SOWM_MOD, XK_c, NULL, {0}},
    {SOWM_MOD, XK_f, NULL, {0}},

    {XCB_MOD_MASK_1, XK_Tab, NULL, {0}},

    /* {SOWM_MOD, XK_Return, NULL, {.cmd = {"st", 0}}}, */

    {SOWM_MOD,                    XK_1, NULL, {.i = 1}},
    {SOWM_MOD|XCB_MOD_MASK_SHIFT, XK_1, NULL, {.i = 1}},
    {SOWM_MOD,                    XK_2, NULL, {.i = 2}},
    {SOWM_MOD|XCB_MOD_MASK_SHIFT, XK_2, NULL, {.i = 2}},
    {SOWM_MOD,                    XK_3, NULL, {.i = 3}},
    {SOWM_MOD|XCB_MOD_MASK_SHIFT, XK_3, NULL, {.i = 3}},
    {SOWM_MOD,                    XK_4, NULL, {.i = 4}},
    {SOWM_MOD|XCB_MOD_MASK_SHIFT, XK_4, NULL, {.i = 4}},
    {SOWM_MOD,                    XK_5, NULL, {.i = 5}},
    {SOWM_MOD|XCB_MOD_MASK_SHIFT, XK_5, NULL, {.i = 5}},
    {SOWM_MOD,                    XK_6, NULL, {.i = 6}},
    {SOWM_MOD|XCB_MOD_MASK_SHIFT, XK_6, NULL, {.i = 6}},
};
