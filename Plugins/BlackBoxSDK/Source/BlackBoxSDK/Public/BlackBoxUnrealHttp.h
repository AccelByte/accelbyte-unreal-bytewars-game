// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/accelbyte/cpp/blackbox_unreal_http.h"

namespace blackbox {
namespace unreal {
class sdk_http_impl : public unreal_http_caller {
public:
    bool add_request(
        uint32_t idx,
        uint64_t id,
        const std::string& method,
        const std::string& url,
        const std::map<std::string, std::string>& headers,
        const std::string& body);

    size_t fire_requests();

    bool remove_request(uint64_t id);

private:
    bool make_request(binder_request& request);

    std::pair<std::string, std::string> get_header_as_pair(FString& header);

    bool has_finished = false;
};
} // namespace unreal
} // namespace blackbox