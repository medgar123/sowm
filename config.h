#define MOD Mod4Mask

#define COM(...) {.com = (const char *[]){__VA_ARGS__, 0}}
#define MM(name) {0, XF86XK_##name, run, COM(#name)}

static struct key keys[] = {
    {MOD,           XK_q,      win_kill,   {0}},
    {MOD,           XK_c,      win_center, {0}},
    {MOD,           XK_f,      win_fs,     {0}},
    {MOD,           XK_h,      win_hide,   {0}},

    {0,        XK_KP_Add,      ws_toggle,   {0}},

    {ShiftMask|MOD, XK_q,      run,        COM("matricide")},

    {MOD,           XK_grave,  ws_toggle, {0}},
    {MOD|ShiftMask, XK_grave,  win_hide,  {0}},

    {MOD, XK_p,      run, COM("Search")},
    {MOD, XK_Return, run, COM("st")},
    {MOD, XK_space,  run, COM("slock")},

    MM(HomePage),
    MM(Mail),
    MM(Search),
    MM(Calculator),
    MM(Tools),
    MM(AudioPrev),
    MM(AudioPlay),
    MM(AudioNext),
    MM(AudioMute),
    MM(AudioLowerVolume),
    MM(AudioRaiseVolume),
    MM(Sleep),
    MM(PowerOff),
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
