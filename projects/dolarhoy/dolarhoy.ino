#include <Arduino.h>
#include "epd_driver.h"
#include "firasans.h"
#include "Button2.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char *ssid = "Davitel_49941_2.4";
const char *password = "20362011877";
const char *apiUrl = "https://dolarapi.com/v1/dolares/blue";

Button2 btn1(BUTTON_1);

uint8_t *framebuffer;
int cursor_x = 20;
int cursor_y = 60;

Rect_t displayArea = {
    .x = 10,
    .y = 20,
    .width = EPD_WIDTH / 2,
    .height = EPD_HEIGHT / 2
};

Rect_t screen = {
    .x = 0,
    .y = 0,
    .width = EPD_WIDTH,
    .height = EPD_HEIGHT
};

// Struct to hold the parsed data
struct DolarInfo {
    int compra;
    int venta;
};

// Function to get and parse the Dolar information from the API
DolarInfo getDolarInfo() {
    DolarInfo info = {0, 0}; // Default values in case of failure

    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(apiUrl);
        int httpResponseCode = http.GET();

        if (httpResponseCode > 0) {
            String payload = http.getString();
            Serial.println(payload);

            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, payload);

            if (!error) {
                info.compra = doc["compra"];
                info.venta = doc["venta"];
            } else {
                Serial.println("JSON Parsing failed");
            }
        } else {
            Serial.print("Error on HTTP request: ");
            Serial.println(httpResponseCode);
        }

        http.end();
    } else {
        Serial.println("WiFi not connected");
    }

    return info;
}


void displayDolarInfo(const DolarInfo &info) {
    cursor_x = 20;
    cursor_y = 60;

    epd_clear_area(displayArea);

    String line_compra = "Compra: $" + String(info.compra);
    String line_venta = "Venta: $" + String(info.venta);

    cursor_x = 20 + 20;
    cursor_y = 60 + 60;

    write_string((GFXfont *)&FiraSans, line_compra.c_str(), &cursor_x, &cursor_y, NULL);

    cursor_x = 20 + 20 + 20;
    cursor_y = 60 + 60 + 60;

    write_string((GFXfont *)&FiraSans, line_venta.c_str(), &cursor_x, &cursor_y, NULL);

    epd_poweroff();
}


void buttonPressed(Button2 &b){
    DolarInfo info = getDolarInfo();
    displayDolarInfo(info);
}


void setup() {
    Serial.begin(115200);
    epd_init();

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    btn1.setPressedHandler(buttonPressed);

    epd_poweron();
    epd_clear_area(screen);
    DolarInfo info = getDolarInfo();
    displayDolarInfo(info);
}

void loop() {
    btn1.loop();
    delay(2);
}
