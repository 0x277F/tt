#ifndef TT_TT_H
#define TT_TT_H

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define _NET_WM_STATE_REMOVE 0
#define _NET_WM_STATE_ADD 1

// Program arguments
#define TT_DEBUG_CWIN 1
#define TT_USE_CWIN 2
#define TT_SPEC_WIN 4
#define TT_ACT_UMAX 8
#define TT_ACT_MAX 16
#define TT_ACT_TMAX 32

#define BITSET_C(bitset, flag) (bitset == 0 ? 0 : ((bitset & flag) == flag))
#define BITSET_NC(bitset, flag) (bitset == 0 ? 1 : ((bitset & flag) != flag))

extern Display *display;
extern unsigned long TT_FLAGS;
extern Window sel_window;

void tt_init();
char *get_property (Display *disp, Window win, Atom xa_prop_type, char *prop_name, unsigned long *size);
void dump_atoms(char *atoms, unsigned long atoms_len);
bool atom_present(char *atoms, unsigned long atoms_len, Atom atom);
Window active_window(Display *display);
bool is_maximized(Display *display, Window window);
bool is_maximized_vert(Display *display, Window window);
bool is_maximized_horz(Display *display, Window window);
bool set_maximized_horz(Display *display, Window window, bool maximized);
bool set_maximized_vert(Display *display, Window window, bool maximized);
void set_maximized(Display *display, Window window, bool maximized);
void toggle_maximize(Display *display, Window window);
void sync_maximize(Display *display, Window window, Atom a_wm_state, Atom a_max_vert, Atom a_max_horz, bool adding);
bool is_decorated(Display *display, Window window);
void set_decorated(Display *display, Window window, bool decorated);

#endif //TT_TT_H
