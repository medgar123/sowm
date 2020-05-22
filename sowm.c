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
    const int i;
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
void win_fs(const Arg arg);
void win_kill(const Arg arg);
void win_move(const Arg arg);
void win_resize(const Arg arg);
void win_prev(const Arg arg);
void win_next(const Arg arg);
void win_lower(const Arg arg);
void win_raise(const Arg arg);
void win_to_ws(const Arg arg);
void ws_go(const Arg arg);

#include "config.h"

#define win        (client *t=0, *c=list; c && t!=list->prev; t=c, c=c->next)
#define ws_save(W) ws_list[W] = list
#define ws_sel(W)  list = ws_list[ws = W]
#define MAX(a, b)  ((a) > (b) ? (a) : (b))

#define win_size(W, gx, gy, gw, gh) \
    XGetGeometry(d, W, &(Window){0}, gx, gy, gw, gh, \
                 &(unsigned int){0}, &(unsigned int){0})

typedef struct client {
    struct client *next, *prev;
    int f, wx, wy;
    unsigned int ww, wh;
    Window w;
} client;

static client       *list = {0}, *ws_list[10] = {0}, *cur;
static int          ws = 1, sw, sh, wx, wy;
static unsigned int ww, wh, clean_mask;

static Display      *d;
static XButtonEvent mouse;
enum { MOVING = 1, SIZING = 2 } drag;
static Window       root;

static void win_add(Window w) {
    client *c;

    if (!(c = (client *) calloc(1, sizeof(client))))
        exit(1);

    c->w = w;

    if (list) {
        list->prev->next = c;
        c->prev          = list->prev;
        list->prev       = c;
        c->next          = list;

    } else {
        list = c;
        list->prev = list->next = list;
    }

    ws_save(ws);
}

static void win_del(Window w) {
    client *x = 0;

    for win if (c->w == w) x = c;

    if (!list || !x)  return;
    if (x->prev == x) list = 0;
    if (list == x)    list = x->next;
    if (x->next)      x->next->prev = x->prev;
    if (x->prev)      x->prev->next = x->next;

    free(x);
    ws_save(ws);
}

static void win_focus(client *c) {
    cur = c;
    XSetInputFocus(d, cur->w, RevertToParent, CurrentTime);
}

void win_move(const Arg arg) {
    win_size(mouse.subwindow, &wx, &wy, &ww, &wh);
    drag = MOVING;
}

void win_resize(const Arg arg) {
    win_size(mouse.subwindow, &wx, &wy, &ww, &wh);
    drag = SIZING;
}

void win_kill(const Arg arg) {
    if (cur) XKillClient(d, cur->w);
}

void win_center(const Arg arg) {
    if (!cur) return;

    win_size(cur->w, &(int){0}, &(int){0}, &ww, &wh);
    XMoveWindow(d, cur->w, (sw - ww) / 2, (sh - wh) / 2);
}

void win_lower(const Arg arg) {
    if (!cur) return;

    XLowerWindow(d, cur->w);
}

void win_raise(const Arg arg) {
    if (!cur) return;

    XRaiseWindow(d, cur->w);
}

void win_fs(const Arg arg) {
    if (!cur) return;

    if ((cur->f = cur->f ? 0 : 1)) {
        win_size(cur->w, &cur->wx, &cur->wy, &cur->ww, &cur->wh);
        XMoveResizeWindow(d, cur->w, 0, 0, sw, sh);

    } else {
        XMoveResizeWindow(d, cur->w, cur->wx, cur->wy, cur->ww, cur->wh);
    }
}

void win_to_ws(const Arg arg) {
    int tmp = ws;

    if (arg.i == tmp) return;

    ws_sel(arg.i);
    win_add(cur->w);
    ws_save(arg.i);

    ws_sel(tmp);
    win_del(cur->w);
    XUnmapWindow(d, cur->w);
    ws_save(tmp);

    if (list) win_focus(list);
}

void win_prev(const Arg arg) {
    if (!cur) return;

    XRaiseWindow(d, cur->prev->w);
    win_focus(cur->prev);
}

void win_next(const Arg arg) {
    if (!cur) return;

    XRaiseWindow(d, cur->next->w);
    win_focus(cur->next);
}

void ws_go(const Arg arg) {
    int tmp = ws;

    if (arg.i == ws) return;

    ws_save(ws);
    ws_sel(arg.i);

    for win XMapWindow(d, c->w);

    ws_sel(tmp);

    for win XUnmapWindow(d, c->w);

    ws_sel(arg.i);

    if (list) win_focus(list); else cur = 0;
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

static void map_request(XEvent *e) {
    Window w = e->xmaprequest.window;

    XSelectInput(d, w, StructureNotifyMask|EnterWindowMask);
    win_size(w, &wx, &wy, &ww, &wh);
    win_add(w);
    cur = list->prev;

    if (wx + wy == 0) win_center((Arg){0});

    XMapWindow(d, w);
    win_focus(list->prev);
}

static void mapping_notify(XEvent *e) {
    XMappingEvent *ev = &e->xmapping;

    if (ev->request == MappingKeyboard || ev->request == MappingModifier) {
        XRefreshKeyboardMapping(ev);
        input_grab();
    }
}

static void notify_destroy(XEvent *e) {
    win_del(e->xdestroywindow.window);

    if (list) win_focus(list->prev);
}

static void notify_enter(XEvent *e) {
    while(XCheckTypedEvent(d, EnterNotify, e));

    for win if (c->w == e->xcrossing.window) win_focus(c);
}

static void notify_motion(XEvent *e) {
    if (!mouse.subwindow || !drag || cur->f) return;

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
    [DestroyNotify]    = notify_destroy,
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
    input_grab();

    while (1 && !XNextEvent(d, &ev)) // 1 && will forever be here.
        if (events[ev.type]) events[ev.type](&ev);
}
