#ifndef CONFIG_H
#define CONFIG_H

#define MOD Mod4Mask

#define COM(...) {.com = (const char *[]){__VA_ARGS__, 0}}

static struct key keys[] = {
    {MOD,      XK_q,   win_kill,   {0}},
    {MOD,      XK_c,   win_center, {0}},
    {MOD,      XK_f,   win_fs,     {0}},

    {Mod1Mask,           XK_Tab, win_next,   {0}},
    {Mod1Mask|ShiftMask, XK_Tab, win_prev,   {0}},

    {MOD, XK_d,      run, COM("dmenu_run")},
    {MOD, XK_w,      run, COM("bud", "/home/goldie/Pictures/Wallpapers")},
    {MOD, XK_p,      run, COM("scr")},
    {MOD, XK_Return, run, COM("st")},

    {0,   XF86XK_AudioLowerVolume,  run, COM("amixer", "sset", "Master", "5%-")},
    {0,   XF86XK_AudioRaiseVolume,  run, COM("amixer", "sset", "Master", "5%+")},
    {0,   XF86XK_AudioMute,         run, COM("amixer", "sset", "Master", "toggle")},
    {0,   XF86XK_MonBrightnessUp,   run, COM("bri", "10", "+")},
    {0,   XF86XK_MonBrightnessDown, run, COM("bri", "10", "-")},

    {MOD,           XK_1, ws_go,     {.i = 1}},
    {MOD|ShiftMask, XK_1, win_to_ws, {.i = 1}},
    {MOD,           XK_2, ws_go,     {.i = 2}},
    {MOD|ShiftMask, XK_2, win_to_ws, {.i = 2}},
    {MOD,           XK_3, ws_go,     {.i = 3}},
    {MOD|ShiftMask, XK_3, win_to_ws, {.i = 3}},
    {MOD,           XK_4, ws_go,     {.i = 4}},
    {MOD|ShiftMask, XK_4, win_to_ws, {.i = 4}},
    {MOD,           XK_5, ws_go,     {.i = 5}},
    {MOD|ShiftMask, XK_5, win_to_ws, {.i = 5}},
    {MOD,           XK_6, ws_go,     {.i = 6}},
    {MOD|ShiftMask, XK_6, win_to_ws, {.i = 6}},
};

#endif
