/**
 * @copyright Copyright (c) 2024  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2024-04-05
 * @note      Arduino Setting
 *            Tools ->
 *                  Board:"ESP32S3 Dev Module"
 *                  USB CDC On Boot:"Enable"
 *                  USB DFU On Boot:"Disable"
 *                  Flash Size : "16MB(128Mb)"
 *                  Flash Mode"QIO 80MHz
 *                  Partition Scheme:"16M Flash(3M APP/9.9MB FATFS)"
 *                  PSRAM:"OPI PSRAM"
 *                  Upload Mode:"UART0/Hardware CDC"
 *                  USB Mode:"Hardware CDC and JTAG"
 *
 */

#ifndef BOARD_HAS_PSRAM
#error "Please enable PSRAM, Arduino IDE -> tools -> PSRAM -> OPI !!!"
#endif

#include <Arduino.h>
#include "epd_driver.h"
#include "firasans.h"
#include "Button2.h"
#include "dolar.h"

#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Davitel_49941_2.4";
const char* password = "20362011877";
const char* apiUrl = "https://dolarapi.com/v1/dolares/blue";

Button2 btn1(BUTTON_1);

#if defined(CONFIG_IDF_TARGET_ESP32)
Button2 btn2(BUTTON_2);
Button2 btn3(BUTTON_3);
#endif

uint8_t *framebuffer;
int vref = 1100;
int cursor_x = 20;
int cursor_y = 60;
int state = 0;

Rect_t area1 = {
    .x = 10,
    .y = 20,
    .width = EPD_WIDTH - 20,
    .height =  EPD_HEIGHT / 2 + 80
};

const char *overview[] = {
    "   DOLAR HOY:"
};


void displayInfo(void)
{
    cursor_x = 20;
    cursor_y = 60;
    state %= 4;

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected to WiFi");
        HTTPClient http;
        http.begin(apiUrl);
        int httpResponseCode = http.GET();

        if (httpResponseCode > 0) {
            String payload = http.getString();
            Serial.println(payload);

            // Assuming the payload is a simple string for now, you might need to parse JSON depending on your API
            epd_clear_area(area1);
            write_string((GFXfont *)&FiraSans, payload.c_str(), &cursor_x, &cursor_y, NULL);
        } else {
            Serial.print("Error on HTTP request: ");
            Serial.println(httpResponseCode);
        }

        http.end();
    } else {
        Serial.println("WiFi not connected");
    }

    switch (state) {
    case 1:
        epd_clear_area(area1);
        write_string((GFXfont *)&FiraSans, (char *)overview[1], &cursor_x, &cursor_y, NULL);
        break;
    case 2:
        epd_clear_area(area1);
        write_string((GFXfont *)&FiraSans, (char *)overview[2], &cursor_x, &cursor_y, NULL);
        break;
    case 3:
        delay(1000);
        epd_clear_area(area1);
        write_string((GFXfont *)&FiraSans, "DeepSleep", &cursor_x, &cursor_y, NULL);
        epd_poweroff_all();
#if defined(CONFIG_IDF_TARGET_ESP32)
        esp_sleep_enable_ext1_wakeup(GPIO_SEL_39, ESP_EXT1_WAKEUP_ANY_LOW);
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
        esp_sleep_enable_ext1_wakeup(GPIO_SEL_21, ESP_EXT1_WAKEUP_ANY_LOW);
#endif
        esp_deep_sleep_start();
        break;
    case 4:
        break;
    default:
        break;
    }
    epd_poweroff();
}

void buttonPressed(Button2 &b)
{
    displayInfo();
    state++;
}


void setup()
{
    Serial.begin(115200);

    epd_init();

    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    framebuffer = (uint8_t *)ps_calloc(sizeof(uint8_t), EPD_WIDTH * EPD_HEIGHT / 2);
    if (!framebuffer) {
        Serial.println("alloc memory failed !!!");
        while (1);
    }
    memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);



    btn1.setPressedHandler(buttonPressed);
#if defined(CONFIG_IDF_TARGET_ESP32)
    btn2.setPressedHandler(buttonPressed);
    btn3.setPressedHandler(buttonPressed);
#endif

    epd_poweron();
    epd_clear();
    write_string((GFXfont *)&FiraSans, (char *)overview[0], &cursor_x, &cursor_y, framebuffer);

    //Draw Box
    epd_draw_rect(600, 450, 120, 60, 0, framebuffer);
    cursor_x = 615;
    cursor_y = 490;
    writeln((GFXfont *)&FiraSans, "Prev", &cursor_x, &cursor_y, framebuffer);

    epd_draw_rect(740, 450, 120, 60, 0, framebuffer);
    cursor_x = 755;
    cursor_y = 490;
    writeln((GFXfont *)&FiraSans, "Next", &cursor_x, &cursor_y, framebuffer);

    Rect_t area = {
        .x = 160,
        .y = 420,
        .width = dolar_width,
        .height =  dolar_height
    };
    epd_copy_to_framebuffer(area, (uint8_t *) dolar_data, framebuffer);

    epd_draw_rect(10, 20, EPD_WIDTH - 20, EPD_HEIGHT / 2 + 80, 0, framebuffer);
    epd_draw_grayscale_image(epd_full_screen(), framebuffer);
    epd_poweroff();
}


void loop()
{
    btn1.loop();

#if defined(CONFIG_IDF_TARGET_ESP32)
    btn2.loop();
    btn3.loop();
#endif

    delay(2);
}
