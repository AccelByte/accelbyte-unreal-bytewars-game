// Copyright (c) 2020-2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "stdint.h"

enum bbx_log_severity {
    BBX_LOG_ERROR = 1 << 0,
    BBX_LOG_WARNING = 1 << 1,
    BBX_LOG_INFO = 1 << 2,
    BBX_LOG_VERBOSE = 1 << 3,
    BBX_LOG_DEBUG = 1 << 4
};
