/****************************************
 *
 * Copyright (c) 2025 Fahim Faisal
 *
 ****************************************/

#include <stddef.h>
#include <stdint.h>

#define TRUE 1
#define FALSE 0

extern uint8_t *data[4];
extern int width;
extern int height;
extern int file_ended;
extern int media_open;

int get_input_media_handlers(char *filename, int scr_width, int scr_height);
int decode_next_frame();
void close_input_media();
