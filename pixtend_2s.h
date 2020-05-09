
#pragma once

#include <stdint.h>
#include <stdbool.h>

union pixtOut;
union pixtIn;
struct pixtend;


struct pixtOutV2S {
    uint8_t byModelOut;
    uint8_t byUCMode;
    uint8_t byUCCtrl0;
    uint8_t byUCCtrl1;
    uint8_t reserved0;
    uint8_t reserved1;
    uint8_t reserved2;
    uint8_t crcHeaderLow;
    uint8_t crcHeaderHigh;

    uint8_t byDigitalInDebounce01;
    uint8_t byDigitalInDebounce23;
    uint8_t byDigitalInDebounce45;
    uint8_t byDigitalInDebounce67;

    uint8_t byDigitalOut;

    uint8_t byRelayOut; 

    uint8_t byGPIOCtrl;
    uint8_t byGPIOOut;

    uint8_t byGPIODebounce01;
    uint8_t byGPIODebounce23;

    uint8_t byPWM0Ctrl0;

    uint8_t wPWM0Ctrl1Low;
    uint8_t wPWM0Ctrl1High;

    uint8_t wPWM0ALow;
    uint8_t wPWM0AHigh;

    uint8_t wPWM0BLow;
    uint8_t wPWM0BHigh;

    uint8_t byPWM1Ctrl0;
    uint8_t byPWM1Ctrl1Low;
    uint8_t byPWM1Ctrl1High;

    uint8_t byPWM1ALow;
    uint8_t byPWM1AHigh;

    uint8_t byPWM1BLow;
    uint8_t byPWM1BHigh;

    uint8_t abyRetainDataOut[32];

    uint8_t crcDataLow;
    uint8_t crcDataHigh;

} __attribute__((packed));


struct pixtInV2S {
    uint8_t byFirmware;
    uint8_t byHardware;
    uint8_t byModelIn;
    uint8_t byUCState;
    uint8_t byUCWarnings;
    uint8_t reserved0;
    uint8_t reserved1;
    uint8_t crcHeaderLow;
    uint8_t crcHeaderHigh;

    uint8_t byDigitalIn;

    uint8_t wAnalogIn0Low;
    uint8_t wAnalogIn0High;

    uint8_t wAnalogIn1Low;
    uint8_t wAnalogIn1High;

    uint8_t byGPIOIn;

    uint8_t wTemp0Low;
    uint8_t wTemp0High;
    uint8_t wHumid0Low;
    uint8_t wHumid0High;

    uint8_t wTemp1Low;
    uint8_t wTemp1High;
    uint8_t wHumid1Low;
    uint8_t wHumid1High;

    uint8_t wTemp2Low;
    uint8_t wTemp2High;
    uint8_t wHumid2Low;
    uint8_t wHumid2High;

    uint8_t wTemp3Low;
    uint8_t wTemp3High;
    uint8_t wHumid3Low;
    uint8_t wHumid3High;

    uint8_t reserved2;
    uint8_t reserved3;

    uint8_t abyRetainDataIn[32];

    uint8_t crcDataLow;
    uint8_t crcDataHigh;

} __attribute__((packed));


void pixtend_v2s_init(struct pixtend * pxt);

// void pixtend_v2s_prepare_output(union pixtOut * output);

// bool pixtend_v2s_parse_input(union pixtIn * input);

// bool pixtend_v2s_set_do(union pixtOut * output, size_t bit, bool enable);

// bool pixtend_v2s_set_ro(union pixtOut * output, size_t bit, bool enable);

// bool pixtend_v2s_get_fw(union pixtIn * input, int * version);

// bool pixtend_v2s_get_hw(union pixtIn * input, int * version);

// bool pixtend_v2s_get_model(union pixtIn * input, char * model);

// bool pixtend_v2s_get_di(union pixtIn * input, size_t bit, bool * enable);



// eof
