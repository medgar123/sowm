#define MOD Mod4Mask

#define COM(...) {.com = (const char *[]){__VA_ARGS__, 0}}

static struct key keys[] = {
    {MOD,      XK_q,   win_kill,   {0}},
    {MOD,      XK_c,   win_center, {0}},
    {MOD,      XK_f,   win_fs,     {0}},
    {MOD,      XK_h,   win_hide,   {0}},

    {Mod1Mask,           XK_Tab, win_lower,   {0}},

    {MOD, XK_d,      run, COM("dmenu_run")},
    {MOD, XK_w,      run, COM("bud", "/home/goldie/Pictures/Wallpapers")},
    {MOD, XK_p,      run, COM("scr")},
    {MOD, XK_Return, run, COM("st")},

    {0,   XF86XK_AudioLowerVolume,  run, COM("amixer", "sset", "Master", "5%-")},
    {0,   XF86XK_AudioRaiseVolume,  run, COM("amixer", "sset", "Master", "5%+")},
    {0,   XF86XK_AudioMute,         run, COM("amixer", "sset", "Master", "toggle")},
    {0,   XF86XK_MonBrightnessUp,   run, COM("bri", "10", "+")},
    {0,   XF86XK_MonBrightnessDown, run, COM("bri", "10", "-")},

    {MOD,           XK_grave, ws_toggle, {0}},
    {MOD|ShiftMask, XK_grave, win_hide,  {0}},
};

static struct button buttons[] = {
    {MOD,           Button1, win_raise, {0}},
    {MOD,           Button1, win_move, {0}},

    {MOD,           Button3, win_raise, {0}},
    {MOD,           Button3, win_resize, {0}},

    {MOD|ShiftMask, Button1, win_raise, {0}},
    {MOD|ShiftMask, Button1, win_center, {0}},

    {MOD|ShiftMask, Button3, win_raise, {0}},
    {MOD|ShiftMask, Button3, win_fs, {0}},

    {MOD,           Button2, win_lower, {0}},
    {MOD|ShiftMask, Button2, win_kill, {0}},
};
