#pragma once

#include <stdint.h>

#include "software_spi.hpp"

// Dac configuration
#define DAC_SUPPLY 3.3           // DAC supply (full swing) (V)
#define DAC_VREF 2.048           // Voltage reference in (V)
#define DAC_GAIN 2               // gain (doubles the voltage reference and halves resolution)
#define DAC_MAX ((1 << 12) - 1)  // 12-bit max

#define VOLTAGE_TO_LEVEL(v) ((v)*DAC_MAX / (DAC_GAIN * DAC_VREF))

union MCP482x_reg {
    struct {
        uint16_t data : 12;
        uint8_t SHDN : 1;  // Shutdown
        uint8_t GA : 1;    // Gain
        uint8_t : 1;       // don't care
        uint8_t W : 1;     // Write
    } bits;
    uint16_t reg;
};

static_assert(sizeof(MCP482x_reg) == 2);

void MCP482x_setLevel(SoftwareSPI* spi, uint16_t level);