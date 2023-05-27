#ifndef _INPUT_DEVICE_H
#define _INPUT_DEVICE_H

#include <stdbool.h>
#include <core/common.hh>

void input_device_init();
void tune_channel_timer();
void exit_tune_channel();
void rbtn_click(right_button_input_t click_type);

#endif
