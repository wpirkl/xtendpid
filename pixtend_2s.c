#include <stdio.h>

#include "pixtend.h"


void pixtend_v2s_prepare_output(union pixtOut * output)
{
    int16_t crc_data;
    int16_t crc_header;

    output->v2s.crcHeaderLow = 0;
    output->v2s.crcHeaderHigh = 0;

    crc_header = crc16(0xffff, &output->v2s.byModelOut, &output->v2s.crcHeaderHigh - &output->v2s.crcHeaderLow + 1);
    crc_data = crc16(0xffff, &output->v2s.byDigitalInDebounce01, &output->v2s.crcDataHigh - &output->v2s.crcDataHigh + 1);

    output->v2s.crcHeaderLow = crc_header & 0xff;
    output->v2s.crcHeaderHigh = (crc_header >> 8) & 0xff;

    output->v2s.crcDataLow = crc_data & 0xff;
    output->v2s.crcDataHigh = (crc_data >> 8) & 0xff;
}


bool pixtend_v2s_parse_input(union pixtIn * input)
{
    int16_t crc_data;
    int16_t crc_header;

    crc_header = crc16(0xffff, &input->v2s.byFirmware, &input->v2s.crcHeaderHigh - &input->v2s.byFirmware + 1);
    crc_data = crc16(0xffff, &input->v2s.byDigitalIn, &input->v2s.crcDataHigh - &input->v2s.byDigitalIn + 1);

    printf("crc_header: %04x\n", crc_header);
    printf("crc_data: %04x\n", crc_data);

    return !(crc_header || crc_data);
}


bool pixtend_v2s_set_do(union pixtOut * output, size_t bit, bool enable)
{
    uint8_t value;

    if(bit > 3) {
        return false;
    }

    value = output->v2s.byDigitalOut;
    if(enable) {
        value |= (uint8_t)(1 << bit);
    } else {
        value &= (uint8_t)(~(1 << bit));
    }

    output->v2s.byDigitalOut = value;

    return true;
}


bool pixtend_v2s_set_ro(union pixtOut * output, size_t bit, bool enable)
{
    uint8_t value;

    if(bit > 3) {
        return false;
    }

    value = output->v2s.byRelayOut;
    if(enable) {
        value |= (uint8_t)(1 << bit);
    } else {
        value &= (uint8_t)(~(1 << bit));
    }

    output->v2s.byRelayOut = value;

    return true;

}

bool pixtend_v2s_get_di(union pixtIn * input, size_t bit, bool * enable)
{
    if(bit > 3) {
        return false;
    }

    if(enable) {
        *enable = (input->v2s.byDigitalIn & (1 << bit)) != 0;
    }

    return true;
}

