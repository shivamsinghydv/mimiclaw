#pragma once
#include <stdbool.h>

void mimi_display_init(void);

void mimi_display_status(const char *msg);

void mimi_display_reply(const char *text);

void mimi_display_wifi(const char *ssid, bool connected);


void mimi_display_thinking(void);

void mimi_display_clear(void);