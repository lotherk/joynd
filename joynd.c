
/* joynd.c
 *
 * Copyright (C) 2018
 *	Konrad Lother <k@hiddenbox.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <signal.h>

#include <SDL.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>

#include "cmdline.h"

struct key_series {
        char *key;
        struct key_series *next;
        struct key_series *prev;
};

struct key_map {
        struct key_series *key_series;
        int min;
        int max;
};

static struct gengetopt_args_info args;
static SDL_Joystick *joystick;
static Display *display;
static XEvent event;
static int run;

static struct key_map **button_map;
static size_t button_map_s;
static struct key_map **axis_map;
static size_t axis_map_s;

static void do_atexit();
static int init_x11();
static int init_SDL();
static int open_joystick(unsigned int num);
static int create_key_maps(struct key_map **m, char **values, size_t size);
static int add_key_to_series(struct key_map *m, char *key);
static int run_button_key_series(int num);
static int release_button_key_series(int num);
static int poll_joystick();
static void list_joystick();

static void signal_handler(int sig);

int main(int argc, char **argv)
{
        int r, i;
        pid_t pid;

        button_map = NULL;
        axis_map = NULL;
        display = NULL;
        joystick = NULL;

        atexit(do_atexit);
        signal(SIGINT, signal_handler);

        if (cmdline_parser(argc, argv, &args) != 0)
                exit(EXIT_FAILURE);



        if (init_SDL() != 0) {
                perror("init_SDL");
                exit(EXIT_FAILURE);
        }

        if (args.list_flag) {
                list_joystick();
                exit(EXIT_SUCCESS);
        }

        if (init_x11() != 0) {
                perror("init_x11");
                exit(EXIT_FAILURE);
        }

        if (open_joystick(args.input_arg) != 0) {
                perror("open_joystick");
                exit(EXIT_FAILURE);
        }

        if (create_key_maps(button_map, args.map_button_arg, args.map_button_given) != 0) {
                perror("create_key_maps");
                exit(EXIT_FAILURE);
        }

        if (args.daemon_flag) {
                daemon(0, 0);
        }

        if((r = poll_joystick()) == 0) {
                perror("poll_joystick");
                exit(EXIT_FAILURE);
        }

        return 0;
}

static int init_x11()
{
        display = XOpenDisplay(NULL);
        if (display == NULL) {
                perror("XOpenDisplay");
                return 1;
        }

        return 0;
}

static int init_SDL()
{
        if (SDL_Init(SDL_INIT_JOYSTICK) < 0) {
                fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
                fflush(stderr);
                errno = -1;
                return 1;
        }

        SDL_JoystickEventState(SDL_ENABLE);

        return 0;
}
static int open_joystick(unsigned int num)
{
        int r, i;

        joystick = SDL_JoystickOpen(num);
        if (joystick == NULL) {
                fprintf(stderr, "SDL_JoystickOpen: %s\n", SDL_GetError());
                fflush(stderr);
                errno = -1;
                return 1;
        }

        r = SDL_JoystickNumButtons(joystick);
        button_map = malloc(sizeof(struct key_map *) * r);
        if (*button_map == NULL)
                return 1;
        button_map_s = r;

        for (i = 0; i < button_map_s; i++) {
                button_map[i] = malloc(sizeof(struct key_map));
                if (button_map[i] == NULL)
                        return 1;

                button_map[i]->key_series = malloc(sizeof(struct key_series));
                if (button_map[i]->key_series == NULL)
                        return 1;

                button_map[i]->key_series->next = NULL;
                button_map[i]->key_series->prev = NULL;
                button_map[i]->key_series->key = NULL;

                button_map[i]->min = 0;
                button_map[i]->max = 1;
        }

        // TODO: implement axis

        return 0;
}
static void list_joystick()
{
        int i;
        char *name, guid_str[256];
        SDL_JoystickGUID guid;

        if (SDL_NumJoysticks() == 0) {
                printf("no joysticks found.\n");
                return;
        }

        for (i = 0; i < SDL_NumJoysticks(); i++) {
                joystick = SDL_JoystickOpen(i);
                if (joystick == NULL) {
                        fprintf(stderr, "error opening joystick %i: %s\n", i, SDL_GetError());
                        exit(EXIT_FAILURE);
                }

                name = (char *) SDL_JoystickName(joystick);
                guid = SDL_JoystickGetGUID(joystick);
                SDL_JoystickGetGUIDString(guid, guid_str, sizeof(guid_str));

                printf("%d: %s (%s)\n", i, name, guid_str);

                SDL_JoystickClose(joystick);
                joystick = NULL;
        }

}
static int create_key_maps(struct key_map **m, char **values, size_t size)
{
        int i, r;

        int l_num, l_min, l_max;
        char *l_val, *r_val, *r_key, *r_next;
        char *delim_lr, *delim_mk;
        char *tmp;

        l_num = -1;
        l_min = -1;
        l_max = -1;

        delim_lr = "=";
        delim_mk = "+";

        for (i = 0; i < size; i++) {
                tmp = values[i];
                l_val = strtok_r(tmp, delim_lr, &r_val);

                if (l_val == NULL || r_val == NULL)
                        return (errno = EINVAL);

                r = sscanf(l_val, "%i:%i:%i", &l_num, &l_min, &l_max);
                if (r < 1)
                        return (errno = EINVAL);

                r_next = r_val;
                do {
                        r_key = strtok_r(r_next, delim_mk, &r_next);
                        if (add_key_to_series(m[l_num], r_key) != 0)
                                return errno;

                } while (r_next != NULL);
        }

        return 0;
}

static int add_key_to_series(struct key_map *m, char *key)
{
        struct key_series *current, *last;

        current = m->key_series;
        last = NULL;
        if (current->key == NULL) {
                current->key = strdup(key);
                return 0;
        }
        do {

                last = current;
        } while ((current = current->next) != NULL);

        current = malloc(sizeof(struct key_series));
        if (current == NULL)
                return 1;
        last->next = current;
        current->prev = last;
        current->next = NULL;

        current->key = strdup(key);
        if (current->key == NULL)
                return 1;
        return 0;
}
static int run_button_key_series(int num) {
        struct key_map *map;
        struct key_series *current;
        unsigned int keycode;
        KeySym keysym;

        map = button_map[num];

        current = map->key_series;

        if (current == NULL)
                return 1;
        if (args.debug_flag)
                fprintf(stdout, "button: %d, key press: ", num);
        do {
                if (current->key != NULL) {
                        keysym = XStringToKeysym(current->key);
                        if (args.debug_flag) {
                                fprintf(stdout, "%s", current->key);
                                if (current->next != NULL && current->next->key != NULL)
                                        fprintf(stdout, "+");
                        } else {
                                keycode = XKeysymToKeycode(display, keysym);
                                XTestFakeKeyEvent(display, keycode, True, 0);
                                XFlush(display);
                        }
                }
        } while ((current = current->next) != NULL);
        if (args.debug_flag) {
                fprintf(stdout, "\n");
                fflush(stdout);
        }
        return 0;
}
static int release_button_key_series(int num) {
        struct key_map *map;
        struct key_series *current, *last;
        unsigned int keycode;
        KeySym keysym;

        map = button_map[num];

        current = map->key_series;

        if (current == NULL)
                return 1;
        if (current->key == NULL)
                return 0;

        do
                last = current;
        while ((current = current->next) != NULL);

        current = last;

        do {
                if (current->key != NULL) {
                        keysym = XStringToKeysym(current->key);
                        keycode = XKeysymToKeycode(display, keysym);
                        XTestFakeKeyEvent(display, keycode, False, 0);
                        XFlush(display);
                }
        } while ((current = current->prev) != NULL);
        return 0;
}
static int poll_joystick()
{
        SDL_Event event;
        struct timespec tim, tim2;

        run = 1;
        tim.tv_sec = 0;
        tim.tv_nsec = 5000000L;

        while (run) {
                while (SDL_PollEvent(&event)) {
                        switch(event.type) {
                        case SDL_JOYBUTTONDOWN:
                                run_button_key_series(event.jbutton.button);
                                break;
                        case SDL_JOYBUTTONUP:
                                release_button_key_series(event.jbutton.button);
                                break;
                        }
                }

                nanosleep(&tim, &tim2);
        }
        return 0;
}
static void signal_handler(int sig) {
        if (sig != SIGINT)
                return;

        exit(EXIT_FAILURE);
}
static void do_atexit()
{
        int i;
        if (joystick != NULL)
                SDL_JoystickClose(joystick);

        if (display != NULL)
                XCloseDisplay(display);

        if (button_map != NULL) {
                for (i = 0; i < button_map_s; i++) {
                        free(button_map[i]);
                }
                free(button_map);
        }
        SDL_Quit();
}
