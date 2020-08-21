#include <xcb/xcb.h>
#include <X11/keysym.h>

#define SOWM_MOD XCB_MOD_MASK_1
#define SOWM_NUM_DESKTOPS 6

const char* term[] = {"st", 0};

struct key keys[] = {
    {SOWM_MOD, XK_q, action_kill,       {0}},
    {SOWM_MOD, XK_c, action_center,     {0}},
    {SOWM_MOD, XK_f, action_fullscreen, {0}},

    {XCB_MOD_MASK_1, XK_Tab, NULL, {0}},

    {SOWM_MOD, XK_Return, action_execute, {.cmd = term}},

    {SOWM_MOD,                    XK_1, action_workspace,      {.i = 1}},
    {SOWM_MOD|XCB_MOD_MASK_SHIFT, XK_1, action_workspace_send, {.i = 1}},
    {SOWM_MOD,                    XK_2, action_workspace,      {.i = 2}},
    {SOWM_MOD|XCB_MOD_MASK_SHIFT, XK_2, action_workspace_send, {.i = 2}},
    {SOWM_MOD,                    XK_3, action_workspace,      {.i = 3}},
    {SOWM_MOD|XCB_MOD_MASK_SHIFT, XK_3, action_workspace_send, {.i = 3}},
    {SOWM_MOD,                    XK_4, action_workspace,      {.i = 4}},
    {SOWM_MOD|XCB_MOD_MASK_SHIFT, XK_4, action_workspace_send, {.i = 4}},
    {SOWM_MOD,                    XK_5, action_workspace,      {.i = 5}},
    {SOWM_MOD|XCB_MOD_MASK_SHIFT, XK_5, action_workspace_send, {.i = 5}},
    {SOWM_MOD,                    XK_6, action_workspace,      {.i = 6}},
    {SOWM_MOD|XCB_MOD_MASK_SHIFT, XK_6, action_workspace_send, {.i = 6}},
};
