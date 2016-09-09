#include <X11/Xlib.h>
#include "tt.h"

Display *display;
unsigned long TT_FLAGS;
Window sel_window;

void tt_init() {
    if (!(display = XOpenDisplay(NULL))) {
        fputs("Error opening display.\n", stderr);
        exit(5);
    }
    TT_FLAGS = 0;
    sel_window = 0;
}

char *get_property (Display *disp, Window win, Atom xa_prop_type, char *prop_name, unsigned long *size) {
    Atom xa_prop_name;
    Atom xa_ret_type;
    int ret_format;
    unsigned long ret_nitems;
    unsigned long ret_bytes_after;
    unsigned long tmp_size;
    unsigned char *ret_prop;
    char *ret;

    xa_prop_name = XInternAtom(disp, prop_name, False);

    /* MAX_PROPERTY_VALUE_LEN / 4 explanation (XGetWindowProperty manpage):
     *
     * long_length = Specifies the length in 32-bit multiples of the
     *               data to be retrieved.
     */
    if (XGetWindowProperty(disp, win, xa_prop_name, 0, 4906 / 4, False,
                           xa_prop_type, &xa_ret_type, &ret_format,
                           &ret_nitems, &ret_bytes_after, &ret_prop) != Success) {
        printf("Cannot get %s property.\n", prop_name);
        return NULL;
    }

    if (xa_ret_type != xa_prop_type) {
        printf("Invalid type of %s property, should be %lu.\n", prop_name, xa_ret_type);
        XFree(ret_prop);
        return NULL;
    }

    /* null terminate the result to make string handling easier */
    tmp_size = (ret_format / (32 / sizeof(long))) * ret_nitems;
    ret = malloc(tmp_size + 1);
    if (!ret) {
        fprintf(stderr, "Problem allocating memory.\n");
        exit(2);
    }
    memcpy(ret, ret_prop, tmp_size);
    ret[tmp_size] = '\0';

    if (size) {
        *size = tmp_size;
    }

    XFree(ret_prop);
    return ret;
}

bool atom_present(char *atoms, unsigned long atoms_len, Atom atom) {
    for (unsigned long i = 0; i < atoms_len; ++i) {
        if (((unsigned long *)atoms)[i] == atom) {
            return true;
        }
    }
    return false;
}

void dump_atoms(char *atoms, unsigned long atoms_len) {
    for (unsigned long i = 0; i < atoms_len; ++i) {
        printf("Atom %lu: %lu\n", i, ((unsigned long *)atoms)[i]);
    }
}

Window active_window(Display *display) {
    if (!display) {
        return 0;
    }
    char *prop;
    unsigned long size;
    Window ret = (Window) 0;
    prop = get_property(display, DefaultRootWindow(display), XA_WINDOW, "_NET_ACTIVE_WINDOW", &size);
    if (prop) {
        ret = *((Window*)prop);
        free(prop);
    }
    return ret;
}

bool is_maximized(Display *display, Window window) {
    if (!display) {
        return false;
    }
    unsigned long size;
    char *prop;
    prop = get_property(display, window, XA_ATOM, "_NET_WM_STATE", &size);
    if (prop) {
        Atom a_max_vert = XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_VERT", false);
        Atom a_max_horz = XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_HORZ", false);
        bool is_max_vert = false, is_max_horz = false;
        for (unsigned long i = 0; i < size; i++) {
            Atom atom = ((Atom*)prop)[i];
            if (atom == a_max_vert) {
                is_max_vert = true;
            } else if (atom == a_max_horz) {
                is_max_horz = true;
            }
        }
        return is_max_vert && is_max_horz;
    }
    return false;
}

bool is_maximized_vert(Display *display, Window window) {
    unsigned long size;
    char *prop = get_property(display, window, XA_ATOM, "_NET_WM_STATE", &size);
    if (prop) {
        Atom atom = XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_VERT", false);
        return atom_present(prop, size, atom);
    }
    return false;
}

bool is_maximized_horz(Display *display, Window window) {
    unsigned long size;
    char *prop = get_property(display, window, XA_ATOM, "_NET_WM_STATE", &size);
    if (prop) {
        Atom atom = XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_HORZ", false);
        return atom_present(prop, size, atom);
    }
    return false;
}

bool set_maximized_horz(Display *display, Window window, bool maximized) {
    if (!display) {
        return false;
    }

    Atom a_wm_state = XInternAtom(display, "_NET_WM_STATE", false);
    Atom a_max_horz = XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_HORZ", false);
    sync_maximize(display, window, a_wm_state, 0, a_max_horz, maximized);
    return true;
}

bool set_maximized_vert(Display *display, Window window, bool maximized) {
    if (!display) {
        return false;
    }

    Atom a_wm_state = XInternAtom(display, "_NET_WM_STATE", false);
    Atom a_max_vert = XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_VERT", false);
    sync_maximize(display, window, a_wm_state, a_max_vert, 0, maximized);
    return true;
}

void set_maximized(Display *display, Window window, bool maximized) {
    if (!display) {
        return;
    }

    Atom a_wm_state = XInternAtom(display, "_NET_WM_STATE", true);
    Atom a_max_vert = XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_VERT", true);
    Atom a_max_horz = XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_HORZ", true);

    unsigned long size;
    char *prop;
    prop = get_property(display, window, XA_ATOM, "_NET_WM_STATE", &size);
    if (prop) {
        puts("test\n");
        if (maximized) {
            Atom atoms[2] = {a_max_vert, a_max_horz};
            XChangeProperty(display, window, a_wm_state, XA_ATOM, 32, PropModePrepend, (unsigned char *) atoms, 2);
        } else {
            // I can't explain it. If you can, please send help.
            Atom *atoms = (Atom *) prop;
            for (unsigned long i = 0; i < size; i++) {
                if (atoms[i] == a_max_horz || atoms[i] == a_max_vert) {
                    if (i != size - 1) {
                        memmove(&atoms[i], &atoms[i+1], sizeof(Atom) * (size - i - 1));
                    }
                    size--;
                    i--;
                }
            }
            XChangeProperty(display, window, a_wm_state, XA_ATOM, 32, PropModeReplace, (unsigned char *) atoms, (int) size);
        }
    }

    sync_maximize(display, window, a_wm_state, a_max_vert, a_max_horz, maximized);
}

void sync_maximize(Display *display, Window window, Atom a_wm_state, Atom a_max_vert, Atom a_max_horz, bool adding) {
    XEvent event;
    memset(&event, 0, sizeof(event));
    event.type = ClientMessage;
    event.xclient.window = window;
    event.xclient.message_type = a_wm_state;
    event.xclient.format = 32;
    event.xclient.data.l[0] = adding;
    event.xclient.data.l[1] = a_max_horz;
    event.xclient.data.l[2] = a_max_vert;
    XSendEvent(display, DefaultRootWindow(display), false, SubstructureNotifyMask, &event);
}

void toggle_maximize(Display *display, Window window) {
    bool max_vert = is_maximized_vert(display, window);
    bool max_horz = is_maximized_horz(display, window);
    set_maximized(display, window, false);

    if (!max_horz || !max_vert) {
        set_maximized(display, window, true);
    }
}

bool is_decorated(Display *display, Window window) {
    if (!display) {
        return false;
    }
    Atom a_motif_wm_hints = XInternAtom(display, "_MOTIF_WM_HINTS", false);
    unsigned long size;
    char *prop;
    prop = get_property(display, window, 0, "_MOTIF_WM_HINTS", &size);
    for (unsigned long i = 0; i < size; ++i) {
        Atom atom = ((Atom*)prop)[i];
        printf("atom: %lu\n", atom);
    }
    return true;
}

void set_decorated(Display *display, Window window, bool decorated) {

}
