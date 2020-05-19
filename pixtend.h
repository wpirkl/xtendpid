

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "pixtend.h"
#include "pixtend_2s.h"


uint16_t crc16(uint16_t crc, uint8_t * data, size_t len);


uint16_t crc16_calc(uint16_t crc, uint8_t data);


union pixtOut {
    struct pixtOutV2S v2s;
};


union pixtIn {
    struct pixtInV2S v2s;
};


struct pixtend {
    bool (*prepare_output)(union pixtOut * output);
    bool (*parse_input)(union pixtIn * input);
    size_t (*get_transfer_size)(void);
    bool (*get_model)(union pixtIn * input, char * model, char * submodel);
    bool (*get_fw_version)(union pixtIn * input, uint8_t * version);
    bool (*get_hw_version)(union pixtIn * input, uint8_t * version);
    size_t (*get_num_di)(void);
    uint8_t (*get_di)(union pixtIn * input, size_t di);
    size_t (*get_num_do)(void);
    size_t (*set_do)(union pixtIn * input, size_t o, bool enable);
};


static inline bool pixt_init(struct pixtend * pixt, char model, char sub_model)
{
    switch(model)
    {
        case '2': {
            if(sub_model == 'S') {
                pixtend_v2s_init(pixt);
                return true;
            } else {
                return false;
            }
        }
    }

    return false;
}


static inline const struct pixtend * pixt_get(char model, char sub_model)
{
    switch(model)
    {
        case '2': {
            if(sub_model == 'S') {
                return pixtend_v2s_get();
            }
            break;
        }
    }

    return NULL;
}


static inline bool pixt_prepare_output(const struct pixtend * pixt, union pixtOut * output)
{
    if(pixt) {
        return pixt->prepare_output(output);
    }

    return false;
}


static inline bool pixt_parse_input(const struct pixtend * pixt, union pixtIn * input)
{
    if(pixt) {
        return pixt->parse_input(input);
    }

    return false;
}


static inline size_t pixt_get_transfer_size(const struct pixtend * pixt)
{
    if(pixt) {
        return pixt->get_transfer_size();
    }

    return false;
}


static inline bool pixt_get_model(struct pixtend * pixt, union pixtIn * input, char * model, char * submodel)
{
    if(pixt) {
        return pixt->get_model(input, model, submodel);
    }

    return false;
}


static inline bool pixt_get_fw_version(struct pixtend * pixt, union pixtIn * input, uint8_t * version)
{
    if(pixt) {
        return pixt->get_fw_version(input, version);
    }

    return false;
}

static inline bool pixt_get_hw_version(struct pixtend * pixt, union pixtIn * input, uint8_t * version)
{
    if(pixt) {
        return pixt->get_hw_version(input, version);
    }

    return false;
}

static inline size_t pixt_get_num_di(struct pixtend * pixt)
{
    if(pixt) {
        return pixt->get_num_di();
    }

    return 0;
}

static inline uint8_t pixt_get_di(struct pixtend * pixt, union pixtIn * input, size_t di)
{
    if(pixt) {
        return pixt->get_di(input, di);
    }

    return 0xff;
}


// eof
