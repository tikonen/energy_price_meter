#pragma once

#include <FS.h>

#define CONFIG_FILENAME "/config.txt"

#define DEFAULT_INTERVAL_S (10 * 60)  // Default poll interval. 10 minutes
#define DEFAULT_SCALE 150.0           // scales number received from the URL to the voltage

struct Configuration {
    char ssid[32 + 1];
    char password[32 + 1];
    char url[256 + 1];
    int interval;  // seconds
    float scale;
};

#define SET_CONF_FIELD(pconfig, field, str)                             \
    do {                                                                \
        strncpy(pconfig->field, str, sizeof(Configuration::field) - 1); \
        pconfig->field[sizeof(Configuration::field) - 1] = 0;           \
    } while (0)

void default_config(Configuration* conf);
bool read_config(File& f, Configuration* conf);
void write_config(File& f, Configuration* conf);
void init_config(File& f, Configuration* conf);
