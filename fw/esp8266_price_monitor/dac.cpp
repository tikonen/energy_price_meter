#include "dac.hpp"

void MCP482x_setLevel(SoftwareSPI* spi, uint16_t level)
{
    MCP482x_reg r{0};
    r.bits.W = 0;
    r.bits.GA = DAC_GAIN & 1;  // 0: 2x, 1: 1x
    r.bits.SHDN = 1;           // Enable
    r.bits.data = level;

    sw_spi_transfer16(spi, &r.reg);
}