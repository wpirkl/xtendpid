#include <stdio.h>

#include "pixtend.h"


static bool pixtend_v2s_prepare_output(union pixtOut * output)
{
    int16_t crc_data;
    int16_t crc_header;

    output->v2s.byModelOut = 'S';
    output->v2s.crcHeaderLow = 0;
    output->v2s.crcHeaderHigh = 0;

    crc_header = crc16(0xffff, &output->v2s.byModelOut, &output->v2s.crcHeaderHigh - &output->v2s.crcHeaderLow + 1);
    crc_data = crc16(0xffff, &output->v2s.byDigitalInDebounce01, &output->v2s.crcDataHigh - &output->v2s.crcDataHigh + 1);

//    printf("crc_header: %04x\n", crc_header);
//    printf("crc_data: %04x\n", crc_data);

    output->v2s.crcHeaderLow = crc_header & 0xff;
    output->v2s.crcHeaderHigh = (crc_header >> 8) & 0xff;

    output->v2s.crcDataLow = crc_data & 0xff;
    output->v2s.crcDataHigh = (crc_data >> 8) & 0xff;

    return true;
}


static bool pixtend_v2s_parse_input(union pixtIn * input)
{
    int16_t crc_data;
    int16_t crc_header;

    crc_header = crc16(0xffff, &input->v2s.byFirmware, &input->v2s.crcHeaderHigh - &input->v2s.byFirmware + 1);
    crc_data = crc16(0xffff, &input->v2s.byDigitalIn, &input->v2s.crcDataHigh - &input->v2s.byDigitalIn + 1);

//    printf("crc_header: %04x\n", crc_header);
//    printf("crc_data: %04x\n", crc_data);

    return !(crc_header || crc_data);
}


static size_t pixtend_v2s_get_transfer_size(void)
{
    // tx and rx structures are the same size, so return just one of them
    return sizeof(struct pixtOutV2S);
}


static bool pixtend_v2s_set_do(union pixtOut * output, size_t bit, bool enable)
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


static bool pixtend_v2s_set_ro(union pixtOut * output, size_t bit, bool enable)
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


static bool pixtend_v2s_get_fw(union pixtIn * input, uint8_t * version)
{

    if(version) {
        *version = input->v2s.byFirmware;
    }

    return true;
}


static bool pixtend_v2s_get_hw(union pixtIn * input, uint8_t * version)
{
    if(version) {
        *version = input->v2s.byHardware;
    }

    return true;
}


static bool pixtend_v2s_get_model(union pixtIn * input, char * model, char * submodel)
{
    if(model) {
        *model = '2';
    }
    if(submodel) {
        *submodel = (char)input->v2s.byModelIn;
    }

    return true;
}


static bool pixtend_v2s_get_di(union pixtIn * input, size_t bit, bool * enable)
{
    if(bit > 3) {
        return false;
    }

    if(enable) {
        *enable = (input->v2s.byDigitalIn & (1 << bit)) != 0;
    }

    return true;
}


void pixtend_v2s_init(struct pixtend * pxt)
{
    if(pxt) {
        pxt->prepare_output = pixtend_v2s_prepare_output;
        pxt->parse_input = pixtend_v2s_parse_input;
        pxt->get_transfer_size = pixtend_v2s_get_transfer_size;
        pxt->get_model = pixtend_v2s_get_model;
        pxt->get_fw_version = pixtend_v2s_get_fw;
        pxt->get_hw_version = pixtend_v2s_get_hw;
    }
}
