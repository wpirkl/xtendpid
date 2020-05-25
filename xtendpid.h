
#pragma once


enum xtendpid_cmd {
    CMD_GET_MODEL = 0,
    CMD_GET_FW_VERSION,
    CMD_GET_HW_VERSION,
    CMD_GET_DI,
    CMD_SET_DO,
    CMD_SET_RO,
};


struct xtendpid_cmd_base {
    uint8_t cmd;
};


struct xtendpid_cmd_get_model {
    struct xtendpid_cmd_base base;
};


struct xtendpid_cmd_get_fw_version {
    struct xtendpid_cmd_base base;
};


struct xtendpid_cmd_get_hw_version {
    struct xtendpid_cmd_base base;
};


struct xtendpid_cmd_get_di {
    struct xtendpid_cmd_base base;
    uint8_t di;
};

struct xtendpid_cmd_set_do {
    struct xtendpid_cmd_base base;
    uint8_t pin;
    uint8_t value;
};

struct xtendpid_cmd_set_ro {
    struct xtendpid_cmd_base base;
    uint8_t pin;
    uint8_t value;
};


union xtendpid_cmds {
    struct xtendpid_cmd_base base;
    struct xtendpid_cmd_get_model get_model;
    struct xtendpid_cmd_get_fw_version get_fw_version;
    struct xtendpid_cmd_get_hw_version get_hw_version;
    struct xtendpid_cmd_get_di get_di;
    struct xtendpid_cmd_set_do set_do;
    struct xtendpid_cmd_set_do set_ro;
};


enum xtendpid_return_code {
    RC_SUCCESS = 0,     // everything is fine
    RC_DEAD,            // worker thread is dead
    RC_UNKNOWN_CMD,     // unknown command
    RC_FAIL,            // something failed
};


struct xtendpid_answer_base {
    uint8_t cmd;
    uint8_t return_code;
};


struct xtendpid_answer_get_model {
    struct xtendpid_answer_base base;
    char model;
    char sub_model;
};


struct xtendpid_answer_get_fw_version {
    struct xtendpid_answer_base base;
    uint8_t version;
};


struct xtendpid_answer_get_hw_version {
    struct xtendpid_answer_base base;
    uint8_t version;
};


struct xtendpid_answer_get_di {
    struct xtendpid_answer_base base;
    uint8_t di;
    uint8_t value;
};


struct xtendpid_answer_set_do {
    struct xtendpid_answer_base base;
};


struct xtendpid_answer_set_ro {
    struct xtendpid_answer_base base;
};


union xtendpid_answer {
    struct xtendpid_answer_base base;
    struct xtendpid_answer_get_model get_model;
    struct xtendpid_answer_get_fw_version get_fw_version;
    struct xtendpid_answer_get_hw_version get_hw_version;
    struct xtendpid_answer_get_di get_di;
    struct xtendpid_answer_set_do set_do;
    struct xtendpid_answer_set_ro set_ro;
};


// eof
