// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#include "../blackbox_common.h"
#include "utils/error_codes.h"

#include <string>

typedef uint32_t error_code;

namespace blackbox {
// Extension Functions
BLACKBOX_API(error::code) stop_helper() noexcept;
BLACKBOX_API(error::code) set_blackbox_helper_heartbeat(bool activate) noexcept;
BLACKBOX_API(bool) helper_is_alive() noexcept;
BLACKBOX_API(bool) hardware_info_is_loaded() noexcept;
BLACKBOX_API(int) get_total_recorded_frame() noexcept;
BLACKBOX_API(int) get_total_recorded_keystroke() noexcept;
BLACKBOX_API(bool) get_key_state(uint64_t idx) noexcept;
BLACKBOX_API(bool) check_last_session_crash_files(const char* project_name, const char* temp_file_path) noexcept;
BLACKBOX_API(bool) search_crash_folder(const char* base_crash_folder, char* found_crash_folder, size_t found_crash_folder_len);
} // namespace blackbox
