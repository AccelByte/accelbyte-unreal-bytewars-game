// Copyright (c) 2020 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once
#include <stdint.h>

struct bbx_callback_http_response {
    uint32_t http_status;
    bool is_success;
    const char* trace_id;
};