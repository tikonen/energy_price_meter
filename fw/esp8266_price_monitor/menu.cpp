#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <FS.h>

#include "main.hpp"
#include "menu.hpp"
#include "serial_cmd.hpp"
#include "printf.h"

static bool modified = false;
static Configuration* config;
static FS* filesys;

typedef void (*MenuHandler)();

typedef struct {
    const char* name;
    MenuHandler handler;
} MenuItem;

void info_handler()
{
    serial_println("Version: " APP_VERSION);
    serial_println("== Config ==");
    serial_printfln("SSID: %s", config->ssid);
    serial_printfln("Password: %s", config->password);
    serial_printfln("URL: %s", config->url);
    serial_printfln("Poll interval (s): %d", config->interval);
    serial_printfln("Scale: %.2f", config->scale);
}

const char* get_user_input()
{
    const char* line = NULL;
    do {
        line = serial_read_line();
    } while (line == NULL);
    return line;
}

void config_handler()
{
    const char* in;

    serial_printf("Enter SSID[%s]>", config->ssid);
    in = get_user_input();
    if (strlen(in)) {
        SET_CONF_FIELD(config, ssid, in);
        modified = true;
    }
    serial_printfln(SER_EOL "SSID: %s", config->ssid);

    serial_printf("Enter password[%s]>", config->password);
    in = get_user_input();
    if (strlen(in)) {
        SET_CONF_FIELD(config, password, in);
        modified = true;
    }
    serial_printfln(SER_EOL "Password: %s", config->password);

    serial_printf("Enter poll interval(s)[%ds]>", config->interval);
    in = get_user_input();
    if (strlen(in)) {
        config->interval = atoi(in);
        if (config->interval <= 0) config->interval = DEFAULT_INTERVAL_S;
        modified = true;
    }
    serial_printfln(SER_EOL "Poll interval: %ds", config->interval);


    serial_printf("Enter scale [%.2f]>", config->scale);
    in = get_user_input();
    if (strlen(in)) {
        config->scale = atof(in);
        if (config->scale == 0) config->scale = 1;
        modified = true;
    }
    serial_printfln(SER_EOL "Scale: %.2f", config->scale);

    serial_printf("Enter URL[%s]>", config->url);
    in = get_user_input();

    if (strlen(in) > 8 && strncmp(in, "http://", 7) == 0) {
        SET_CONF_FIELD(config, url, in);
        modified = true;
    } else {
        serial_println(SER_EOL "Invalid URL");
    }

    serial_printfln(SER_EOL "URL: %s", config->url);
    serial_println("Config set");
}

void config_file_dump_handler()
{
    File f = filesys->open(CONFIG_FILENAME, "r");
    if (!f) {
        serial_printfln("Config file %s not found", CONFIG_FILENAME);
        return;
    }
    serial_printfln("Config file %s", CONFIG_FILENAME);
    serial_println(">>>>");
    while (f.available()) {
        serial_write_char(f.read());
    }
    serial_println("<<<<");
}

void config_file_save_handler()
{
    serial_printfln("Writing config file %s", CONFIG_FILENAME);
    File f = filesys->open(CONFIG_FILENAME, "w+");
    if (f) {
        write_config(f, config);
        modified = false;
        serial_println("Done");
    } else {
        serial_println("Cannot open file for writing.");
    }
}

void load_config_handler()
{
    default_config(config);
    serial_printfln("Loading config file %s", CONFIG_FILENAME);
    File f = filesys->open(CONFIG_FILENAME, "r");
    if (f) {
        read_config(f, config);
        modified = false;
        serial_println("Done");
    } else {
        serial_printfln("Config file %s not found", CONFIG_FILENAME);
    }
}

void config_file_delete_handler()
{
    if (filesys->exists(CONFIG_FILENAME)) {
        serial_printfln("Deleting config file %s", CONFIG_FILENAME);
        filesys->remove(CONFIG_FILENAME);
        serial_println("Done");
    } else {
        serial_printfln("Config file %s not found", CONFIG_FILENAME);
    }
}

const char* encryptionTypeToStr(int encryption)
{
    switch (encryption) {
        case ENC_TYPE_WEP: return "WEP"; break;
        case ENC_TYPE_TKIP: return "WPA"; break;
        case ENC_TYPE_CCMP: return "WPA2"; break;
        case ENC_TYPE_NONE: return "None"; break;
        case ENC_TYPE_AUTO: return "Auto"; break;
    }
    return "Unknown";
}


void wifi_scan_handler()
{
    // scan for nearby networks:
    int numSsid = WiFi.scanNetworks();
    if (numSsid == -1) {
        serial_println("Couldn't get a wifi connection");
    }

    // print the list of networks seen:
    serial_printfln("Number of available networks: %d", numSsid);

    // print the network number and name for each network found:
    for (int thisNet = 0; thisNet < numSsid; thisNet++) {
        long rssi = WiFi.RSSI(thisNet);
        const char* encType = encryptionTypeToStr(WiFi.encryptionType(thisNet));
        serial_printfln("%s Signal: %d dBm Encryption: %s", WiFi.SSID(thisNet).c_str(), rssi, encType);
    }
}


void wifi_connect_handler()
{
    serial_printfln("Connecting to %s", config->ssid);
    if (WiFi.status() != WL_CONNECTED) {
        WiFi.mode(WIFI_STA);
        WiFi.begin(config->ssid, config->password);
        serial_print("Connecting");
        int retries = 30;
        while (WiFi.status() != WL_CONNECTED && retries-- > 0) {
            delay(1000);
            serial_print(".");
        }
        serial_print(SER_EOL);
    }
    if (WiFi.status() == WL_CONNECTED) {
        serial_print("Connected, IP address: ");
        Serial.println(WiFi.localIP());
    } else {
        serial_println("Connection failed.");
        WiFi.disconnect();
    }
}

void url_test_handler()
{
    if (WiFi.status() != WL_CONNECTED) {
        serial_println("WiFi not connected.");
        return;
    }
    WiFiClient client;
    HTTPClient http;
    serial_printfln("Connecting %s", config->url);
    // connect and issue request
    if (http.begin(client, config->url)) {
        serial_println("Sending request.");

        int httpCode = http.GET();
        if (httpCode > 0) {
            serial_printfln("HTTP response %d", httpCode);

            while (client.available()) {
                serial_write_char(client.read());
            }
            serial_println("");

            http.end();
            return;
        }
        http.end();
    }
    serial_println("Request failed.");
}

void reset_handler() { ESP.restart(); }

MenuItem menuItems[] = {                            //
    {"Info", info_handler},                         //
    {"Configure", config_handler},                  //
    {"Reload config", load_config_handler},         //
    {"Save config", config_file_save_handler},      //
    {"Dump config", config_file_dump_handler},      //
    {"Delete config", config_file_delete_handler},  //
    {"Scan WiFi", wifi_scan_handler},               //
    {"Connect WiFi", wifi_connect_handler},         //
    {"Test URL", url_test_handler},                 //
    {"Restart", reset_handler}};

constexpr int menuItemCount = sizeof(menuItems) / sizeof(menuItems[0]);

const char* wifiStatusToStr(int status)
{
    switch (status) {
        case WL_IDLE_STATUS: return "WL_IDLE_STATUS";
        case WL_NO_SSID_AVAIL: return "WL_NO_SSID_AVAIL";
        case WL_SCAN_COMPLETED: return "WL_SCAN_COMPLETED";
        case WL_CONNECTED: return "WL_CONNECTED";
        case WL_CONNECT_FAILED: return "WL_CONNECT_FAILED";
        case WL_CONNECTION_LOST: return "WL_CONNECTION_LOST";
        case WL_DISCONNECTED: return "WL_DISCONNECTED";
    };

    return "Unknown";
}

void print_menu()
{
    serial_println(APP_NAME " " APP_VERSION);
    serial_println("== Options ==");
    for (int i = 0; i < menuItemCount; i++) {
        serial_printfln("%d. %s", i + 1, menuItems[i].name);
    }
    int status = WiFi.status();
    serial_printfln("Wifi status: %s (%d)", wifiStatusToStr(status), status);
    if (modified) serial_println("* Unsaved changes");
    serial_println("");
}

void menu(FS* pfs, Configuration* pconf)
{
    filesys = pfs;
    config = pconf;
    Serial.begin(115200);
    print_menu();
    serial_println(">");

    while (true) {
        const char* cmd = serial_read_line();
        if (!cmd) continue;
        if (strlen(cmd) == 0) {
            print_menu();
        } else {
            int select = atoi(cmd);
            if (select > 0 && select <= menuItemCount) {
                serial_printfln("** %s **", menuItems[select - 1].name);
                menuItems[select - 1].handler();
            } else {
                serial_println("Invalid option.");
            }
        }
        serial_println(">");

        // serial_echo_loop();  // DEBUG
    }
}
