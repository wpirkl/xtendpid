
#pragma once


enum xtendpid_cmd {
    CMD_GET_MODEL = 0,
    CMD_GET_FW_VERSION,
    CMD_GET_HW_VERSION,
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


union xtendpid_cmds {
    struct xtendpid_cmd_base base;
    struct xtendpid_cmd_get_model get_model;
    struct xtendpid_cmd_get_fw_version get_fw_version;
    struct xtendpid_cmd_get_hw_version get_hw_version;
};


enum xtendpid_return_code {
    RC_SUCCESS = 0,
    RC_DEAD,
    RC_UNKNOWN_CMD,
};


struct xtendpid_answer_base {
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


union xtendpid_answer {
    struct xtendpid_answer_base base;
    struct xtendpid_answer_get_model get_model;
    struct xtendpid_answer_get_fw_version get_fw_version;
    struct xtendpid_answer_get_hw_version get_hw_version;
};


// eof
