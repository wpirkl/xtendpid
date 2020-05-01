
#include "pixtend.h"
#include "pixtend_2s.h"


void prepare_output_v2s(struct pixtOutV2S * output)
{
    int16_t crc_data;
    int16_t crc_header;

    output->crcHeaderLow = 0;
    output->crcHeaderHigh = 0;

    crc_header = crc16(0xffff, &output->byModelOut, &output->crcHeaderHigh - &output->byModelOut + 1);
    crc_data = crc16(0xffff, &output->byDigitalInDebounce01, &output->crcDataHigh - &output->byDigitalInDebounce01 + 1); 

    output->crcHeaderLow = crc_header & 0xff;
    output->crcHeaderHigh = (crc_header >> 8) & 0xff;

    output->crcDataLow = crc_data & 0xff;
    output->crcDataHigh = (crc_data >> 8) & 0xff;

}

bool parse_input_v2s(struct pixtInV2S * input)
{


    return false;
}

