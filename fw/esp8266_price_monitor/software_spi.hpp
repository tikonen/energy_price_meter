#pragma once

// Bit banging SPI implementation.

struct SoftwareSPI {
    int mosipin;
    int misopin;
    int sclkpin;
    int cspin;
    int periodus;
};

void sw_spi_init(SoftwareSPI* spi, int mosipin, int sclkpin, int cspin, int freq);
void sw_spi_transfer(const SoftwareSPI* spi, void* data, int len);
void sw_spi_transfer16(const SoftwareSPI* spi, uint16_t* data);

void sw_spi_test(SoftwareSPI* spi);