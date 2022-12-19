# Price Monitor

### Introduction

ESP8266 WiFi SoC application fetches peridiocally a number from an URL and sets MCP4821 DAC to set matching voltage (with appropriate scaling). URL is expected to return a single positive floating point number. Number scale can be configured and is by default 150.


    ~ $ curl -s http://sometime.com/price/spot
    32.01


Application was designed to run an vintage analog voltage meter to show the current price of kWh.

Configuration of WiFi credentials, URL and other parameters can be done over serial terminal (115200-8-N-1) that can be activated by pressing and keeping the programming button pressed within 1 second from powering up.

**NOTE** Do not leave serial adapter connected on normal operation.

### Serial configuration terminal example

    == Options ==
    1. Info
    2. Configure
    3. Reload config
    4. Save config
    5. Dump config
    6. Delete config
    7. Scan WiFi
    8. Connect WiFi
    9. Test URL
    10. Restart
    Wifi status: WL_DISCONNECTED (7)

    > 7
    ** Scan WiFi **
    Number of available networks: 18
    TP-Link_F35C Signal: -75 dBm Encryption: WPA2
    Telia-75FC4A Signal: -76 dBm Encryption: WPA2
    Koti_BF11 Signal: -67 dBm Encryption: WPA2
    NETGEAR07 Signal: -90 dBm Encryption: WPA2
    NETGEAR07-Guest Signal: -86 dBm Encryption: Auto
    shahop Signal: -83 dBm Encryption: WPA2
    HUAWEI-0100JI Signal: -92 dBm Encryption: WPA2
    DNA-Mokkula-2G-8J58RB Signal: -87 dBm Encryption: WPA2        
    4G-Gateway-EC98 Signal: -90 dBm Encryption: WPA2
    TP-Link_BB3E Signal: -86 dBm Encryption: Auto    
    ZyXELA56ECE Signal: -58 dBm Encryption: WPA2
    TP-Link_3DC7 Signal: -87 dBm Encryption: WPA2
    RUT240_3F3F Signal: -92 dBm Encryption: WPA2
    Marras30 Signal: -83 dBm Encryption: WPA2

    > 2
    ** Configure **
    Enter SSID[MyWifi]>
    SSID: ZyXELA56ECE
    Enter password[wrongpass]>FooBar
    Password: FooBar
    Enter poll interval(s)[600s]>
    Poll interval: 600s
    Enter scale [150.00]>
    Scale: 150.00
    Enter URL[http://somesite.com/price/spot]>
    URL: http://somesite.com/price/spot
    Config set
    > 1
    ** Info **
    Version: v0.1
    == Config ==
    SSID: ZyXELA56ECE
    Password: FooBar
    URL: http://sometime.com/price/spot
    Poll interval (s): 600
    Scale: 150.00
    >
    Price Monitor v0.1
    == Options ==
    1. Info
    2. Configure
    3. Reload config
    4. Save config
    5. Dump config
    6. Delete config
    7. Scan WiFi
    8. Connect WiFi
    9. Test URL
    10. Restart
    Wifi status: WL_IDLE_STATUS (0)
    * Unsaved changes

    > 9
    ** Test URL **
    WiFi not connected.
    > 8
    ** Connect WiFi **
    Connecting to ZyXELA56ECE
    Connecting....
    Connected, IP address: 192.168.1.98
    > 9
    ** Test URL **
    Connecting http://sometime.com/price/spot
    Sending request.
    HTTP response 200
    32.01
    > 4
    ** Save Config **
    Writing config file /config.txt
    Done
    >
