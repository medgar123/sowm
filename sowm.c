// sowm - An itsy bitsy floating window manager.

#include <X11/Xlib.h>
#include <X11/XF86keysym.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

typedef union {
    const char** com;
} Arg;

struct key {
    unsigned int mod;
    KeySym keysym;
    void (*function)(const Arg arg);
    const Arg arg;
};

struct button {
    unsigned int mod;
    unsigned int button;
    void (*function)(const Arg arg);
    const Arg arg;
};

void run(const Arg arg);
void win_center(const Arg arg);
void win_focus(const Arg arg);
void win_fs(const Arg arg);
void win_hide(const Arg arg);
void win_kill(const Arg arg);
void win_lower(const Arg arg);
void win_move(const Arg arg);
void win_raise(const Arg arg);
void win_resize(const Arg arg);
void ws_toggle(const Arg arg);

#include "config.h"

#define MAX(a, b)  ((a) > (b) ? (a) : (b))

#define win_size(W, gx, gy, gw, gh) \
    XGetGeometry(d, W, &(Window){0}, gx, gy, gw, gh, \
                 &(unsigned int){0}, &(unsigned int){0})

static int          wx, wy;
static unsigned int sw, sh, ww, wh, clean_mask;

static Display      *d;
static XButtonEvent mouse;
enum { MOVING = 1, SIZING = 2 } drag;
static Window       root, cur;

static void win_each(void(*op)(Window)) {
    Window *children;
    unsigned int i, n;

    if (!XQueryTree(d, root, &(Window){0}, &(Window){0}, &children, &n))
        return;
    for (i = 0; i < n; i++)
        op(children[i]);
    XFree(children);
}

void win_focus(const Arg arg) {
    if (!cur) return;
    XSetInputFocus(d, cur, RevertToParent, CurrentTime);
}

void win_move(const Arg arg) {
    win_size(mouse.subwindow, &wx, &wy, &ww, &wh);
    drag = MOVING;
}

void win_resize(const Arg arg) {
    win_size(mouse.subwindow, &wx, &wy, &ww, &wh);
    drag = SIZING;
}

void win_center(const Arg arg) {
    if (!cur) return;

    win_size(cur, &(int){0}, &(int){0}, &ww, &wh);
    XMoveWindow(d, cur, (sw - ww) / 2, (sh - wh) / 2);
}

void win_fs(const Arg arg) {
    if (!cur) return;

    win_size(cur, &(int){0}, &(int){0}, &ww, &wh);
    if (ww!=sw || wh!=sh) {
        XMoveResizeWindow(d, cur, 0, 0, sw, sh);
    } else {
        XMoveResizeWindow(d, cur, sw/4, sh/4, sw/2, sh/2);
    }
}

void win_hide(const Arg arg) {
    if (!cur) return;

    XUnmapWindow(d, cur);
    cur = 0;
    XSetInputFocus(d, root, RevertToParent, CurrentTime);
}

void win_kill(const Arg arg) {
    if (cur) XKillClient(d, cur);
}

void win_lower(const Arg arg) {
    if (!cur) return;

    XLowerWindow(d, cur);
}

void win_raise(const Arg arg) {
    if (!cur) return;

    XRaiseWindow(d, cur);
}

static void toggle_mapped(Window w) {
    XWindowAttributes wa;
    if (!XGetWindowAttributes(d, w, &wa))
        return;
    switch (wa.map_state) {
    case IsUnmapped: XMapWindow(d, w); break;
    case IsViewable: XUnmapWindow(d, w); break;
    }
}

void ws_toggle(const Arg arg) {
    win_each(toggle_mapped);
}

void run(const Arg arg) {
    if (fork()) return;
    if (d) close(ConnectionNumber(d));

    setsid();
    execvp((char*)arg.com[0], (char**)arg.com);
    exit(111);
}

// Taken from DWM. Many thanks. https://git.suckless.org/dwm
static unsigned int numlockmask(void) {
    unsigned int nlm = 0;
    XModifierKeymap *modmap = XGetModifierMapping(d);
    KeyCode code = XKeysymToKeycode(d, XK_Num_Lock);

    for (unsigned int i = 0; i < 8; i++)
        for (int m = modmap->max_keypermod, k = 0; k < m; k++)
            if (modmap->modifiermap[i * m + k] == code)
                nlm = 1<<i;
    XFreeModifiermap(modmap);

    XUngrabKey(d, AnyKey, AnyModifier, root);

    clean_mask = ~(nlm|LockMask) &
        (ShiftMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask);
    return nlm;
}

static void input_grab(void) {
    unsigned int nlm = numlockmask();
    unsigned int modifiers[] = {0, LockMask, nlm, nlm|LockMask};
    KeyCode code;

    for (size_t i = 0; i < sizeof(keys)/sizeof(*keys); i++)
        if ((code = XKeysymToKeycode(d, keys[i].keysym)))
            for (size_t j = 0; j < sizeof(modifiers)/sizeof(*modifiers); j++)
                XGrabKey(d, code, keys[i].mod | modifiers[j], root,
                        True, GrabModeAsync, GrabModeAsync);

    for (size_t i = 0; i < sizeof(buttons)/sizeof(*buttons); i++)
        for (size_t j = 0; j < sizeof(modifiers)/sizeof(*modifiers); j++)
            XGrabButton(d, buttons[i].button, buttons[i].mod | modifiers[j], root, True,
                ButtonPressMask|ButtonReleaseMask|PointerMotionMask,
                GrabModeAsync, GrabModeAsync, 0, 0);
}

static void button_press(XEvent *e) {
    if (!e->xbutton.subwindow) return;
    unsigned mod = clean_mask & e->xbutton.state;

    mouse = e->xbutton;
    drag = 0;
    for (unsigned int i = 0; i < sizeof(buttons)/sizeof(*buttons); ++i)
        if (buttons[i].button == e->xbutton.button &&
            buttons[i].mod == mod)
            buttons[i].function(buttons[i].arg);
}

static void button_release(XEvent *e) {
    mouse.subwindow = 0;
}

static void configure_request(XEvent *e) {
    XConfigureRequestEvent *ev = &e->xconfigurerequest;

    XConfigureWindow(d, ev->window, ev->value_mask, &(XWindowChanges) {
        .x          = ev->x,
        .y          = ev->y,
        .width      = ev->width,
        .height     = ev->height,
        .sibling    = ev->above,
        .stack_mode = ev->detail
    });
}

static void key_press(XEvent *e) {
    KeySym keysym = XkbKeycodeToKeysym(d, e->xkey.keycode, 0, 0);
    unsigned mod = clean_mask & e->xkey.state;

    for (unsigned int i=0; i < sizeof(keys)/sizeof(*keys); ++i)
        if (keys[i].keysym == keysym &&
            keys[i].mod == mod)
            keys[i].function(keys[i].arg);
}

static void adopt(Window w) {
    XSelectInput(d, w, StructureNotifyMask|EnterWindowMask);
}

static void map_request(XEvent *e) {
    cur = e->xmaprequest.window;

    adopt(cur);
    win_size(cur, &wx, &wy, &ww, &wh);
    if (wx + wy == 0) win_center((Arg){0});

    XMapWindow(d, cur);
    win_focus((Arg){0});
}

static void mapping_notify(XEvent *e) {
    XMappingEvent *ev = &e->xmapping;

    if (ev->request == MappingKeyboard || ev->request == MappingModifier) {
        XRefreshKeyboardMapping(ev);
        input_grab();
    }
}

static void notify_enter(XEvent *e) {
    while(XCheckTypedEvent(d, EnterNotify, e));

    cur = e->xcrossing.window;
    win_focus((Arg){0});
}

static void notify_motion(XEvent *e) {
    if (!mouse.subwindow || !drag) return;

    while(XCheckTypedEvent(d, MotionNotify, e));

    int xd = e->xbutton.x_root - mouse.x_root;
    int yd = e->xbutton.y_root - mouse.y_root;

    XMoveResizeWindow(d, mouse.subwindow,
        wx + (drag == MOVING ? xd : 0),
        wy + (drag == MOVING ? yd : 0),
        MAX(1, ww + (drag == SIZING ? xd : 0)),
        MAX(1, wh + (drag == SIZING ? yd : 0)));
}

static void (*events[LASTEvent])(XEvent *e) = {
    [ButtonPress]      = button_press,
    [ButtonRelease]    = button_release,
    [ConfigureRequest] = configure_request,
    [KeyPress]         = key_press,
    [MapRequest]       = map_request,
    [MappingNotify]    = mapping_notify,
    [EnterNotify]      = notify_enter,
    [MotionNotify]     = notify_motion
};

static int xerror() { return 0; }

int main(void) {
    XEvent ev;

    if (!(d = XOpenDisplay(0))) exit(1);

    signal(SIGCHLD, SIG_IGN);
    XSetErrorHandler(xerror);

    int s = DefaultScreen(d);
    root  = RootWindow(d, s);
    sw    = XDisplayWidth(d, s);
    sh    = XDisplayHeight(d, s);

    XSelectInput(d,  root, SubstructureRedirectMask);
    XDefineCursor(d, root, XCreateFontCursor(d, 68));
    win_each(adopt);
    input_grab();

    while (1 && !XNextEvent(d, &ev)) // 1 && will forever be here.
        if (events[ev.type]) events[ev.type](&ev);
}
