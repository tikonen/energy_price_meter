#include <Arduino.h>

#include "software_spi.hpp"

void sw_spi_init(SoftwareSPI* spi, int mosipin, int sclkpin, int cspin, int freq)
{
    spi->mosipin = mosipin;
    spi->misopin = -1;  // spi->misopin not supported currently
    spi->sclkpin = sclkpin;
    spi->cspin = cspin;

    if (spi->cspin >= 0) {
        pinMode(spi->cspin, OUTPUT);
        digitalWrite(spi->cspin, HIGH);
    }

    if (spi->sclkpin >= 0) {
        pinMode(spi->sclkpin, OUTPUT);
        digitalWrite(spi->sclkpin, LOW);
    }
    if (spi->mosipin >= 0) {
        pinMode(spi->mosipin, OUTPUT);
        digitalWrite(spi->mosipin, LOW);
    }

    spi->periodus = 1E6 / freq;
}

void sw_spi_transfer16(const SoftwareSPI* spi, uint16_t* data)
{
    // swap to MSB
    uint8_t l = *data & 0xFF;
    uint8_t h = (*data >> 8) & 0xFF;
    uint16_t sdata = (l << 8) | h;

    sw_spi_transfer(spi, &sdata, 2);
}

void sw_spi_transfer(const SoftwareSPI* spi, void* data, int len)
{
    byte* buffer = (byte*)data;

    if (spi->cspin >= 0) {
        digitalWrite(spi->cspin, LOW);
    }
    delayMicroseconds(spi->periodus);

    // In following loop data is sent as:
    // CPOL = 0, Clock polarity
    // CPHA = 0, Sample phase
    // MSB first
    while (len-- > 0) {
        byte b = *buffer;
        byte bin = 0;
        for (int i = 0; i < 8; i++) {
            // MSB first
            digitalWrite(spi->mosipin, (b >> 7) & 1);
            b <<= 1;
            delayMicroseconds(spi->periodus);
            /*
            if (spi->misopin >= 0) {
                bin |= digitalRead(spi->misopin);
                bin <<= 1;
            }
            */
            digitalWrite(spi->sclkpin, HIGH);
            delayMicroseconds(spi->periodus);
            digitalWrite(spi->sclkpin, LOW);
        }
        *buffer = bin;
        buffer++;
    }
    digitalWrite(spi->mosipin, LOW);

    if (spi->cspin >= 0) {
        digitalWrite(spi->cspin, HIGH);
    }
    delayMicroseconds(spi->periodus);
}

// Transfer few bytes to test SPI
void sw_spi_test(SoftwareSPI* spi)
{
    delay(500);
    char data[] = "ABCDE";
    int len = strlen(data);

    sw_spi_transfer(spi, data, len);
}
