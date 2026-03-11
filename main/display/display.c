#include "mimi_display.h"
#include "ssd1306.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <string.h>
#include <stdio.h>
#include <display.h>
static const char *TAG = "mimi_display";

#define DISPLAY_SDA      8
#define DISPLAY_SCL      9
#define DISPLAY_WIDTH  128
#define DISPLAY_HEIGHT  64  
#define DISPLAY_ADDR  0x3C 

static SSD1306_t s_dev;
static bool      s_ready = false;
static SemaphoreHandle_t s_lock;

static void put_row(int row, const char *text) {
    char buf[17] = {0};
    strncpy(buf, text, 16);
    ssd1306_display_text(&s_dev, row, buf, (int)strlen(buf), false);
}


static void put_wrapped(const char *text, int start_row, int max_rows) {
    if (!text) return;
    char line[17] = {0};
    int  col = 0, row = start_row;
    for (const char *p = text; *p && row < start_row + max_rows; p++) {
        if (*p == '\n' || col == 16) {
            line[col] = '\0';
            put_row(row++, line);
            memset(line, 0, sizeof(line));
            col = 0;
            if (*p == '\n') continue;
        }
        line[col++] = *p;
    }
    if (col > 0 && row < start_row + max_rows) {
        line[col] = '\0';
        put_row(row, line);
    }
}



void mimi_display_init(void) {
    s_lock = xSemaphoreCreateMutex();

    ESP_LOGI(TAG, "init SDA=%d SCL=%d addr=0x%02X", DISPLAY_SDA, DISPLAY_SCL, DISPLAY_ADDR);

    i2c_master_init(&s_dev, DISPLAY_SDA, DISPLAY_SCL, -1);
    ssd1306_init(&s_dev, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    ssd1306_clear_screen(&s_dev, false);
    ssd1306_contrast(&s_dev, 0xFF);

    s_ready = true;

    put_row(0, "  MimiClaw  ");
    put_row(1, " Starting...");
    ESP_LOGI(TAG, "SSD1306 ready");
}

void mimi_display_clear(void) {
    if (!s_ready) return;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    ssd1306_clear_screen(&s_dev, false);
    xSemaphoreGive(s_lock);
}

void mimi_display_status(const char *msg) {
    if (!s_ready || !msg) return;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    ssd1306_clear_screen(&s_dev, false);
    put_row(0, "   Status");
    put_wrapped(msg, 1, 3);
    xSemaphoreGive(s_lock);
}

void mimi_display_thinking(void) {
    if (!s_ready) return;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    ssd1306_clear_screen(&s_dev, false);
    put_row(0, "  MimiClaw");
    put_row(2, "  Thinking..");
    xSemaphoreGive(s_lock);
}

void mimi_display_reply(const char *text) {
    if (!s_ready || !text) return;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    ssd1306_clear_screen(&s_dev, false);
    put_row(0, "   Reply");
    put_wrapped(text, 1, 7);  
    xSemaphoreGive(s_lock);
}

void mimi_display_wifi(const char *ssid, bool connected) {
    if (!s_ready) return;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    ssd1306_clear_screen(&s_dev, false);
    put_row(0, connected ? "WiFi  OK" : "WiFi  ...");
    if (ssid) put_row(1, ssid);
    xSemaphoreGive(s_lock);
}