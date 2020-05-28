#include <stdio.h>

#include "pixtend.h"


static bool pixtend_v2s_prepare_output(union pixtOut * output)
{
    int16_t crc_data;
    int16_t crc_header;

    output->v2s.byModelOut = 'S';
    output->v2s.crcHeaderLow = 0;
    output->v2s.crcHeaderHigh = 0;
    output->v2s.crcDataLow = 0;
    output->v2s.crcDataHigh = 0;

    crc_header = crc16(0xffff, &output->v2s.byModelOut, &output->v2s.crcHeaderLow - &output->v2s.byModelOut);
    crc_data = crc16(0xffff, &output->v2s.byDigitalInDebounce01, &output->v2s.crcDataLow - &output->v2s.byDigitalInDebounce01);

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


static size_t pixtend_v2s_get_num_di(void)
{
    return 8;
}


static uint8_t pixtend_v2s_get_di(union pixtIn * input, size_t bit)
{
    if(bit >= pixtend_v2s_get_num_di()) {
        return 0xff;
    }

    return input->v2s.byDigitalIn & (1 << bit);
}


static size_t pixtend_v2s_get_num_do(void)
{
    return 4;
}


static bool pixtend_v2s_set_do(union pixtOut * output, size_t bit, bool enable)
{
    uint8_t value;

    if(bit >= pixtend_v2s_get_num_do()) {
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


static size_t pixtend_v2s_get_num_ro(void)
{
    return 4;
}


static bool pixtend_v2s_set_ro(union pixtOut * output, size_t bit, bool enable)
{
    uint8_t value;

    if(bit >= pixtend_v2s_get_num_ro()) {
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


static const struct pixtend pixtend_v2s = {
    .prepare_output = pixtend_v2s_prepare_output,
    .parse_input =pixtend_v2s_parse_input,
    .get_transfer_size = pixtend_v2s_get_transfer_size,
    .get_model = pixtend_v2s_get_model,
    .get_fw_version = pixtend_v2s_get_fw,
    .get_hw_version = pixtend_v2s_get_hw,
    .get_num_di = pixtend_v2s_get_num_di,
    .get_di = pixtend_v2s_get_di,
    .get_num_do = pixtend_v2s_get_num_do,
    .set_do = pixtend_v2s_set_do,
    .get_num_ro = pixtend_v2s_get_num_ro,
    .set_ro = pixtend_v2s_set_ro,
};


const struct pixtend * pixtend_v2s_get(void) {
    return &pixtend_v2s;
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
        pxt->get_num_di = pixtend_v2s_get_num_di;
        pxt->get_di = pixtend_v2s_get_di;
        pxt->get_num_do = pixtend_v2s_get_num_do;
        pxt->set_do = pixtend_v2s_set_do;
        pxt->get_num_ro = pixtend_v2s_get_num_ro;
        pxt->set_ro = pixtend_v2s_set_ro;
    }
}

// eof
