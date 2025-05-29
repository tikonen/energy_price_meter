#include <FS.h>
#include <string.h>

#include "config.hpp"

#define KEY_SSID "SSID"
#define KEY_PASSWORD "PASSWORD"
#define KEY_URL "URL"
#define KEY_INTERVAL "INT"
#define KEY_SCALE "SCALE"

void default_config(Configuration* conf)
{
    strcpy(conf->ssid, "MyWifi");
    strcpy(conf->password, "foobar");
    strcpy(conf->url, "http://somesite.me/price/spot");
    conf->interval = DEFAULT_INTERVAL_S;
    conf->scale = DEFAULT_SCALE;
};

// Reads newline separated lines. Does not return empty lines.
bool read_line(Stream& ins, char* ptr, int len)
{
    int idx = 0;

    while (ins.available()) {
        char c = ins.peek();
        if (!isspace(c)) {
            break;
        }
        ins.read();  // consume whitespace
    }
    while (ins.available() && --len > 0) {
        char c = ins.read();
        if (c == '\r' || c == '\n') break;
        ptr[idx++] = c;
    }
    ptr[idx] = '\0';

    // None-empty line found
    return idx != 0;
}

bool read_config(File& f, Configuration* conf)
{
    if (!f) return false;

    char buffer[256];
    while (read_line(f, buffer, sizeof(buffer))) {
        char* p = strstr(buffer, "=");
        if (!p) continue;
        *p = '\0';
        const char* key = buffer;
        const char* value = p + 1;

        if (!strcmp(KEY_URL, key)) {
            SET_CONF_FIELD(conf, url, value);
        } else if (!strcmp(KEY_SSID, key)) {
            SET_CONF_FIELD(conf, ssid, value);
        } else if (!strcmp(KEY_INTERVAL, key)) {
            conf->interval = atoi(value);
        } else if (!strcmp(KEY_SCALE, key)) {
            conf->scale = atof(value);
        } else if (!strcmp(KEY_PASSWORD, key)) {
            SET_CONF_FIELD(conf, password, value);
        }
    }
    return true;
}

void write_config(File& f, Configuration* conf)
{
    f.print(KEY_URL);
    f.print('=');
    f.println(conf->url);
    f.print(KEY_SSID);
    f.print('=');
    f.println(conf->ssid);
    f.print(KEY_PASSWORD);
    f.print('=');
    f.println(conf->password);
    f.print(KEY_INTERVAL);
    f.print('=');
    f.println(conf->interval);
    f.print(KEY_SCALE);
    f.print('=');
    f.println(conf->scale);    
}

void init_config(File& f, Configuration* conf)
{
    default_config(conf);
    read_config(f, conf);
}
