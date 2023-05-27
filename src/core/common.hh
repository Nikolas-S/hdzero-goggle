#ifndef __COMMON_HH__
#define __COMMON_HH__

#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <lvgl/lvgl.h>

#include "defines.h"

typedef enum {
    HW_REV_UNKNOWN = 0,
    HW_REV_1,
    HW_REV_2
} hw_revision_t;

void setHwRevision(hw_revision_t revision);
hw_revision_t getHwRevision();

#define DIAL_KEY_UP     1
#define DIAL_KEY_DOWN   2
#define DIAL_KEY_CLICK  3
#define DIAL_KEY_PRESS  4

typedef enum {
    RIGHT_KEY_CLICK = 5,
    RIGHT_KEY_LONG_PRESS = 6,
    RIGHT_KEY_DOUBLE_CLICK = 7
} right_button_input_t;

extern atomic_int g_key;
extern atomic_int g_init_done;
extern pthread_mutex_t lvgl_mutex;

uint8_t slow_key(left_dial_t key, uint8_t *state, uint8_t *cnt);

#endif //__COMMON_HH__
