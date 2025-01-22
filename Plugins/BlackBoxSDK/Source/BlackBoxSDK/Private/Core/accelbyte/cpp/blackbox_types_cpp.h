// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#ifndef BLACKBOX_TYPES_CPP_H
#define BLACKBOX_TYPES_CPP_H

#include<stdint.h>

struct bbx_game_symbol {
    const char* module_id;
    uint64_t relative_address;
    const char* symbol_name;
};

struct bbx_game_variable {
    uint64_t relative_address;
    uint64_t size;
    const char* variable_name;
};

#endif