#include <Arduino.h>

#if !defined(ARDUINO_ESP8266_GENERIC)
#error "This sketch must be compiled for ESP-01 (Generic ESP8266 Module)"
#endif

#if LED_BUILTIN != 1
#error "Led pin invalid for ESP-01"
#endif

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <LittleFS.h>

#include "main.hpp"
#include "config.hpp"
#include "software_spi.hpp"
#include "serial_cmd.hpp"
#include "dac.hpp"
#include "menu.hpp"
#include "printf.h"
#include "timer.hpp"

void _putchar(char character) { serial_write_char(character); }

// Pin settings
#define PIN_TERMINAL 2

// SPI settings
#define PIN_SCLK 2
#define PIN_MOSI 1
#define PIN_CS 0
#define SPI_FREQ 10000
#define SCLK_PERIOD_US (1E6 / (SPI_FREQ))

#define METER_RANGE_V 3.0  // Meter display range in volts. Cannot be higher than DAC_SUPPLY

static_assert(METER_RANGE_V <= DAC_SUPPLY);

SoftwareSPI spi;
Configuration config;

class Tween
{
public:
    Tween(int ms)
    {
        duration = ms;
        reset();
    }
    void reset() { ts = millis(); }

    // Returns false when tween duration is completed. t progresses from 0 to 1 but
    // is not guaranteed to have values 0 or 1 at the start and the end.
    bool update(float& t)
    {
        const uint32_t elapsed = millis() - ts;
        t = float(elapsed) / duration;
        return t <= 1.0;
    }

protected:
    uint32_t duration;
    uint32_t ts;
};

// Move meter back and forth to indicate full range of motion
void sweep_meter()
{
    const int steps = 100;
    const float totalTime = 1.5;  // seconds

    for (int s = 0; s <= steps; s++) {
        float t = float(s) / steps;
        MCP482x_setLevel(&spi, VOLTAGE_TO_LEVEL(METER_RANGE_V * t));
        delay(totalTime * 1000 / steps);
    }
    MCP482x_setLevel(&spi, VOLTAGE_TO_LEVEL(METER_RANGE_V));
    delay(2000);
    for (int s = 0; s <= steps; s++) {
        float t = float(s) / steps;
        MCP482x_setLevel(&spi, VOLTAGE_TO_LEVEL(METER_RANGE_V * (1 - t)));
        delay(totalTime * 1000 / steps);
    }
    MCP482x_setLevel(&spi, VOLTAGE_TO_LEVEL(0));
    delay(500);
}

void setup()
{
    // Initialize local filesystem support
    LittleFSConfig fsconfig;
    fsconfig.setAutoFormat(true);
    LittleFS.setConfig(fsconfig);
    LittleFS.begin();

    File f = LittleFS.open(CONFIG_FILENAME, "r");
    init_config(f, &config);
    f.close();

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    pinMode(PIN_TERMINAL, INPUT);

    // Wait 1 second if terminal button is pressed
    Timer2 startDelay(false, 1000);
    bool enterMenu = false;
    while (startDelay.update() && !enterMenu) {
        enterMenu = !digitalRead(PIN_TERMINAL);
    }

    // enterMenu = true;

    if (enterMenu) {
        // Indicate shell mode by blinking led rapidly
        for (int i = 0; i < 5; i++) {
            delay(100);
            digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        }

        menu(&LittleFS, &config);
        while (true)  // never exit
            ;
    } else {
        sw_spi_init(&spi, PIN_MOSI, PIN_SCLK, PIN_CS, SPI_FREQ);

#if 0
        while (true) sweep_meter();
#else
        // Start wifi initialization at background
        WiFi.mode(WIFI_STA);
        WiFi.begin(config.ssid, config.password);

        // Automatic sleep saves energy during long periods of inactivity
        WiFi.setSleepMode(WIFI_LIGHT_SLEEP, 3);  // Automatic Light Sleep, DTIM listen interval = 3

        sweep_meter();

        // Wait until Wi-Fi connected
        int step = 0;
        const int totalsteps = 15;
        while (WiFi.status() != WL_CONNECTED) {
            // Use the meter as a progress indicator ticker
            uint16_t level = VOLTAGE_TO_LEVEL(METER_RANGE_V * float(step) / totalsteps);
            MCP482x_setLevel(&spi, level);
            step = (step + 1) % (totalsteps + 1);

            delay(500);
        }

        MCP482x_setLevel(&spi, 0);
#endif
    }
}

void indicate_price(float price)
{
    // Set DAC output

    float r = price / config.scale;
    if (r < 0)
        r = 0;
    else if (r > 1.1)  // leave some space for calibration and scaling
        r = 1.1;

    uint16_t level = VOLTAGE_TO_LEVEL(METER_RANGE_V * r);
    MCP482x_setLevel(&spi, level);
}

bool fetch_and_process()
{
    bool success = false;
    WiFiClient client;
    HTTPClient http;
    // connect and issue request
    if (http.begin(client, config.url)) {
        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK) {
            float p = http.getString().toFloat();
            if (!isnan(p)) {
                indicate_price(p);
                success = true;
            }
        }
    }
    http.end();

    return success;
}

void longsleep(int ms)
{
    do {
        // sleep in 1s chunks
        int s = min(ms, 1000);
        delay(s);
        ms -= s;
    } while (ms > 0);
}

void loop()
{
    static int backoff = 2;  // seconds

    if (WiFi.status() != WL_CONNECTED) {
        return;
    }
    if (fetch_and_process()) {
        longsleep(config.interval * 1000);
        backoff = 2;
    } else {
        // exponential backoff of retries
        longsleep(backoff * 1000);
        backoff *= 2;
        if (backoff > config.interval) backoff = config.interval;
    }
}