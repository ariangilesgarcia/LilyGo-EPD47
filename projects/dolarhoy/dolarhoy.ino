#include <Arduino.h>
#include "epd_driver.h"
#include "firasans.h"
#include "Button2.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "Davitel_49941_2.4";
const char* password = "20362011877";
const char* apiUrl = "https://dolarapi.com/v1/dolares/blue";

Button2 btn1(BUTTON_1);

uint8_t *framebuffer;
int cursor_x = 20;
int cursor_y = 60;

Rect_t displayArea = {
    .x = 10,
    .y = 20,
    .width = EPD_WIDTH - 20,
    .height =  EPD_HEIGHT / 2 + 80
};

void displayDolarInfo()
{
    cursor_x = 20;
    cursor_y = 60;

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
                int compra = doc["compra"];
                int venta = doc["venta"];

                epd_clear_area(displayArea);

                String info = "Compra: " + String(compra) + "\nVenta: " + String(venta);
                write_string((GFXfont *)&FiraSans, info.c_str(), &cursor_x, &cursor_y, NULL);
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

    epd_poweroff();
}

void buttonPressed(Button2 &b)
{
    displayDolarInfo();
}

void setup()
{
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
    epd_clear();
    displayDolarInfo(); // Display information on startup
}

void loop()
{
    btn1.loop();
    delay(2);
}
