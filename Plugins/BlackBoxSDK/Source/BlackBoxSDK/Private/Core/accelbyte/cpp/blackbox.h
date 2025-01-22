// Copyright (c) 2021-2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#ifndef ACCELBYTE_BLACKBOX_H
#define ACCELBYTE_BLACKBOX_H

#include "../blackbox_common.h"
#include "blackbox_http.h"
#include "blackbox_types_cpp.h"
#include "utils/log_severity.h"

#include <stdint.h>
#include <stdio.h>
#if BLACKBOX_LINUX_PLATFORM
#    include <csignal>
#    include <ucontext.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*session_callback_t)(const bbx_callback_http_response&, const char*);
typedef void (*info_gather_callback_t)();
typedef void (*playtest_id_callback_t)(const char*);
typedef void (*log_callback)(enum bbx_log_severity, const char*);
typedef void (*match_created_callback_fn_t)(
    bool /* is_successful */, const char* /* error_message */, const char* /* match_id */);
typedef void (*match_session_started_callback_fn_t)(bool /* is_successful */, const char* /* error_message */);
typedef void (*match_session_ended_callback_fn_t)(bool /* is_successful */, const char* /* error_message */);

typedef uint32_t error_code;

struct bbx_ucontext_buffer {
    // Size of ucontext is 936 bytes in unreal's clang toolchain, this is based on old Glibc.
    char value[936];
};

// Main Functions

/**
 * @brief Initiate BlackBox SDK main modules
 *
 * @param api The currently used Graphics API
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_initiate_main_modules(uint8_t g_api = 0U);

/**
 * @brief Shutdown BlackBox SDK main modules
 *
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(void) bbx_shutdown_main_modules();

/**
 * @brief Initiate BlackBox SDK base modules
 *
 * This function must be called prior to the call to `bbx_initiate_main_modules()`
 *
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_initiate_base_modules();

/**
 * @brief Shutdown BlackBox SDK base modules
 *
 * This function must be called after the call to `bbx_shutdown_main_modules()`
 *
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(void) bbx_shutdown_base_modules();

// Main Functions - Sessions

/**
 * @brief Starting new BlackBox session
 *
 * This command will in effect start the crash video recorder and profiling
 * enabled components are customizeable from the website
 *
 * @param callback Callback on completion
 * @param playtest_id_retrieved_callback to fetch playtest id
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code)
bbx_start_new_session(
    session_callback_t request_session_id_callback, playtest_id_callback_t playtest_id_retrieved_callback);

/**
 * @brief Same as start new session but used inside the editor
 *
 * @param callback Callback on completion
 * @param playtest_id_retrieved_callback to fetch playtest id
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code)
bbx_start_new_session_on_editor(session_callback_t callback, playtest_id_callback_t playtest_id_retrieved_callback);

/**
 * @brief Get the session id or empty if the session haven't been started
 *
 * @return const char* reference to the session id string
 */
BLACKBOX_API(const char*) bbx_get_session_id();

/**
 * @brief Update existing session with external user id
 *
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code)
bbx_update_existing_session_with_external_user_id(const char* external_user_id);

/**
 * @brief Update existing session with external user id
 *
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code)
bbx_update_existing_session_with_external_session_id(const char* external_session_id);

/**
 * @brief Set the http proxy to use
 *
 * @param addr proxy address
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_set_http_proxy(const char* addr);

/**
 * @brief Tick the SDK, executing the callback
 *
 * @param dt delta time between calls
 */
BLACKBOX_API(void) bbx_tick(float dt);

/**
 * @brief Tick the SDK using engine tick (not game tick), executing the callback
 *
 * @param dt delta time between calls
 */
BLACKBOX_API(void) bbx_engine_tick(float dt);

/**
 * @brief Start gather hardware and system info
 *
 * @param callback Callback on gather information complete
 */
BLACKBOX_API(void) bbx_start_gather_device_info(info_gather_callback_t callback);

/**
 * @brief Dump gathered hardware and system info to a file
 *
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_dump_device_info_to_file(const char* file_path);

/**
 * @brief Clear all pending callbacks
 *
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_clear_pending_tasks();

// Main Functions - Key Input

/**
 * @brief Set the up key input from the game engine
 *
 * @param action_names names of key actions
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_setup_key_input(const char** action_names, size_t length);

/**
 * @brief Update the state of registered keys
 *
 * @param key_idx index of registered key
 * @param pressed state of the key
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_update_key_input(uint8_t key_idx, bool pressed);

// Main Functions - Additional Info

/**
 * @brief Additional info from the game to be shown in the website
 *
 * Currently the only supported data type is string, this function will add
 * if the field is non yet exist and update if it's already exist.
 *
 * @param field_name The info name
 * @param value The info value
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_update_additional_info(const char* field_name, const char* value);

/**
 * @brief Delete a field in Additional info list
 *
 * @param field_name The info name
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_delete_additional_info_field(const char* field_name);

/**
 * @brief Get the additional info value object
 *
 * @param field_name The info name
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(const char*) bbx_get_additional_info_value(const char* field_name);

/**
 * @brief Empties the additional info list
 *
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_empty_additional_info();

// Main Functions - Recorder
#if BLACKBOX_WINDOWS_PLATFORM
/**
 * @brief Update the blackbox video recorder texture
 *
 * This function initiate the internal texture buffer if it's not yet exist, effectively starting
 * the recording
 *
 * @param backbuffer_tex Backbuffer texture from the game
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_update_backbuffer_dx11_texture(void* backbuffer_tex);

/**
 * @brief Update the blackbox video recorder texture
 *
 * This function initiate the internal texture buffer if it's not yet exist, effectively starting
 * the recording
 *
 * @param backbuffer_tex Backbuffer texture from the game
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_update_backbuffer_dx12_texture(void* backbuffer_tex);
#endif

/**
 * @brief Stop video recording
 *
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_stop_recording();

// Main Functions - Profiler

/**
 * @brief Start the profiling module
 *
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_start_profiling();

/**
 * @brief Stop the profiling module
 *
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_stop_profiling();

/**
 * @brief Suspend the shared mem usage
 *
 * Effectively will shutdown the helper on windows
 *
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_suspend_shared_mem();

/**
 * @brief Continue the shared mem usage
 *
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_continue_shared_mem();

/**
 * @brief Register that the game has crashed
 *
 * @return error_code
 */
BLACKBOX_API(error_code) bbx_blackbox_set_game_has_crashed();

/**
 * @brief Feed the data from game engine to send as basic profiling
 *
 * @param dt frame time
 * @param gpu_render_time GPU render time
 * @param game_render_time Game thread render time
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_update_profiling_basic_data(float dt, float gpu_render_time, float game_thread_time);

/**
 * @brief Store dynamic profiling data from debug symbol file
 *
 * @param module_name the name of the main module in symbol file
 * @param symbols symbols to profile
 * @param variables variables to profile
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code)
bbx_store_profiling_data(
    const char* module_name,
    bbx_game_symbol* symbols,
    size_t symbols_len,
    bbx_game_variable* variables,
    size_t variables_len);

// Logs

/**
 * @brief Set the log severity for log callback
 *
 * Please check log_severity.h to see available severity levels
 *
 * @param enabled_sev severity levels to enable
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_set_log_severity(enum bbx_log_severity enabled_sev);

/**
 * @brief Set the log callback
 *
 * @param callback function to call when there's a log available
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_set_log_callback(log_callback);

/**
 * @brief Feed the data from unreal engine to send as log stream
 *
 * @param data log from unreal engine
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_collect_log_streaming_data(const char* data);

/**
 * @brief Stop the log streaming module
 *
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_stop_log_streaming();

/**
 * @brief Set the file compression function object
 *
 * @param tgt_fn function pointer to compression function
 */
BLACKBOX_API(void) bbx_set_file_compression_function(bool (*tgt_fn)(void*, signed int&, const void*, signed int));

/**
 * @brief Compress data using zlib compression scheme
 *
 * @param compressed_data buffer to compressed data, this is the output of this function
 * @param compressed_size size of the compressed data, this value must be set with buffer size when calling this
 * function
 * @param uncompressed_data buffer to uncompressed data, this is the input of this function
 * @param compressed_size size of the uncompressed data
 */
BLACKBOX_API(error_code)
bbx_compress_data(
    void* compressed_data, int32_t& compressed_size, const void* uncompressed_data, int32_t uncompressed_size);

// Configs

/**
 * @brief get base url
 *
 * @return const char* base url
 */
BLACKBOX_API(const char*) bbx_config_get_base_url();

/**
 * @brief get Identity and Access Management (IAM) url
 *
 * @return const char* IAM url
 */
BLACKBOX_API(const char*) bbx_config_get_iam_url();

/**
 * @brief get SDK download url
 *
 * @return std::string SDK download url
 */
BLACKBOX_API(const char*) bbx_config_get_sdk_download_url();

/**
 * @brief get releason json url
 *
 * @return std::string release json url
 */
BLACKBOX_API(const char*) bbx_config_get_release_json_url();

/**
 * @brief get game version id
 *
 * @return const char* game version id
 */
BLACKBOX_API(const char*) bbx_config_get_game_version_id();

/**
 * @brief get build id
 *
 * @return const char* build id
 */
BLACKBOX_API(const char*) bbx_config_get_build_id();

/**
 * @brief get namespace
 *
 * @return const char* namespace
 */
BLACKBOX_API(const char*) bbx_config_get_namespace();

/**
 * @brief get project id
 *
 * @return const char* project id
 */
BLACKBOX_API(const char*) bbx_config_get_project_id();

/**
 * @brief get API key
 *
 * @return const char* API key
 */
BLACKBOX_API(const char*) bbx_config_get_api_key();

/**
 * @brief get config path
 *
 * @return const char* config path
 */
BLACKBOX_API(const char*) bbx_config_get_config_path();

/**
 * @brief get webconfig fps
 *
 * @return error_code fps value
 */
BLACKBOX_API(error_code) bbx_config_get_fps();

/**
 * @brief get webconfig kps
 *
 * @return error_code kps value
 */
BLACKBOX_API(error_code) bbx_config_get_kps();

/**
 * @brief get webconfig recording second
 *
 * @return error_code recording second value
 */
BLACKBOX_API(error_code) bbx_config_get_total_second();

/**
 * @brief get webconfig substitle type
 *
 * @return const char* subtitle type value
 */
BLACKBOX_API(const char*) bbx_config_get_subtitle_type();

/**
 * @brief get webconfig enable crash reporter
 *
 * @return bool enable crash reporter value
 */
BLACKBOX_API(bool) bbx_config_get_enable_crash_reporter();

/**
 * @brief get webconfig store dxdiag
 *
 * @return bool is store dxdiag value
 */
BLACKBOX_API(bool) bbx_config_get_store_dxdiag();

/**
 * @brief get webconfig store crash video
 *
 * @return bool store crash video value
 */
BLACKBOX_API(bool) bbx_config_get_store_crash_video();

/**
 * @brief get webconfig enable basic profiling
 *
 * @return bool basic profiling value
 */
BLACKBOX_API(bool) bbx_config_get_enable_basic_profiling();

/**
 * @brief get webconfig enable cpu profiling
 *
 * @return bool cpu profiling value
 */
BLACKBOX_API(bool) bbx_config_get_enable_cpu_profiling();

/**
 * @brief get webconfig enable gpu profiling
 *
 * @return bool gpu profiling value
 */
BLACKBOX_API(bool) bbx_config_get_enable_gpu_profiling();

/**
 * @brief get webconfig enable memory profiling
 *
 * @return bool memory profiling value
 */
BLACKBOX_API(bool) bbx_config_get_enable_memory_profiling();

/**
 * @brief get crash folder
 *
 * @return const char* the crash folder path
 */
BLACKBOX_API(const char*) bbx_config_get_crash_folder();

/**
 * @brief get crash GUID
 *
 * @return const char* the crash GUID
 */
BLACKBOX_API(const char*) bbx_config_get_crash_guid();

/**
 * @brief get is issue reporter enabled
 *
 * @return bool is issue reporter enabled
 */
BLACKBOX_API(bool) bbx_config_get_enable_issue_reporter();

/**
 * @brief get issue reporter hotkey
 *
 * @return const char* issue reporter hotkey
 */
BLACKBOX_API(const char*) bbx_config_get_issue_reporter_hotkey();

/**
 * @brief get use engine for report screenshot information
 *
 * @return bool is engine used for report screenshot
 */
BLACKBOX_API(bool) bbx_config_get_use_engine_to_capture_screenshot();

/**
 * @brief get a boolean value indicating whether the log is included in the issue report
 *
 * @return bool indicates whether the log is included in the issue report
 */
BLACKBOX_API(bool) bbx_config_get_include_log_in_issue_report();

/**
 * @brief Check if BlackBox SDK currently running under a server entity
 *
 * @return bool Is a server entity
 */
BLACKBOX_API(bool) bbx_config_get_is_running_as_server();

/**
 * @brief get current iam user id
 *
 * @return const char* the current iam user id
 */
BLACKBOX_API(const char*) bbx_config_get_iam_user_id();

/**
 * @brief get current playtest id
 *
 * @return const char* the current playtest id
 */
BLACKBOX_API(const char*) bbx_config_get_playtest_id();

/**
 * @brief get current device id
 *
 * @return const char* the current device id
 */
BLACKBOX_API(const char*) bbx_config_get_device_id();

/**
 * @brief set and store base url
 *
 * @param base_url new  blackbox base url
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_config_set_base_url(const char* base_url);

/**
 * @brief set IAM url
 *
 * @param iam_url new blackbox IAM url
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_config_set_iam_url(const char* iam_url);

/**
 * @brief set your game version id
 *
 * @param game_version_id new game version id
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_config_set_game_version_id(const char* game_version_id);

#if BLACKBOX_PS4_PLATFORM || BLACKBOX_PS5_PLATFORM
/**
 * @brief set your platform SDK version
 *
 * @param platform_sdk_version new platform SDK version
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_config_set_platform_sdk_version(const char* platform_sdk_version);
#endif

/**
 * @brief set latest sdk download url
 *
 * @param download_url new sdk download url
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_config_set_sdk_download_url(const char* download_url);

/**
 * @brief set latest release json url
 *
 * @param release_json_url new release json url
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_config_set_release_json_url(const char* release_json_url);

/**
 * @brief set your game build id
 *
 * @param build_id new build id
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_config_set_build_id(const char* build_id);

/**
 * @brief set your game namespace
 *
 * @param namespace_ new namespace
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_config_set_namespace(const char* namespace_);

/**
 * @brief set your game project id
 *
 * @param project_id new project id
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_config_set_project_id(const char* project_id);

/**
 * @brief set API key
 *
 * @param api_key new API key
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_config_set_api_key(const char* api_key);

/**
 * @brief set game client pid
 *
 * @param pid new pid
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_config_set_pid(uint32_t pid);

/**
 * @brief set crash folder
 *
 * The location of crash folder is dependent on the game engine and
 * so this information needs to be fed from the engine
 *
 * @param crash_folder new crash folder
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_config_set_crash_folder(const char* crash_folder);

/**
 * @brief set is using editor
 *
 * The flag is the game currently using editor or package build
 *
 * @param is_using_editor is with editor flag
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_config_set_is_using_editor(bool is_using_editor);
/**
 * @brief set blackbox helper path
 *
 * This function tells the SDK where to look for the helper
 * @param path new helper path
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_config_set_blackbox_helper_path(const char* path);

/**
 * @brief set blackbox helper alternative path
 *
 * This function tells the SDK where to look for the helper if restricted by admin access.
 *
 * @param path new helper path
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_config_set_blackbox_helper_alternative_path(const char* path);

/**
 * @brief set blackbox issue reporter path
 *
 * This function tells the SDK where to look for issue reporter
 * @param path new issue reporter path
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_config_set_blackbox_issue_reporter_path(const char* path);

/**
 * @brief set crash guid
 *
 * @param crash_guid
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_config_set_crash_guid(const char* crash_guid);

/**
 * @brief set blackbox log source path
 *
 * This function tells the SDK where to look for the log source file
 * @param path new log source path
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_config_set_log_source_file_path(const char* path);

/**
 * @brief set rendering GPU name
 *
 * On dual GPU setup, the rendering GPU isn't always the primary GPU
 * This function is provided so that the game engine can tell the SDK which GPU is being used
 *
 * @param gpu_name new rendering GPU name
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_config_set_rendering_gpu_name(const char* gpu_name);

/**
 * @brief set gpu driver ver
 *
 * @param ver_str new driver version
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_config_set_gpu_ver(const char* ver_str);

/**
 * @brief set default config file location
 *
 * @param path file location
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_config_set_config_path(const char* path);

/**
 * @brief set engine version associated with crash handler
 *
 * @param ver_str engine version associated
 * @return error_code any errors encountered
 */
BLACKBOX_API(error_code) bbx_config_set_engine_version(const char* ver_str);

/**
 * @brief set the currently gpu device id so that the SDK will use the same id with the engine
 *
 * @param id the gpu device id currently used
 * @return error_code any errors encountered
 */
BLACKBOX_API(error_code) bbx_config_set_gpu_device_id(error_code id);

/**
 * @brief set engine type associated with crash handler
 *
 * @param engine_type engine type associated [UE4,UE5]
 * @return error_code any errors encountered
 */
BLACKBOX_API(error_code) bbx_config_set_engine(const char* engine_type);

/**
 * @brief set engine major version associated with crash handler
 *
 * @param major_version engine major version associated
 * @return error_code any errors encountered
 */
BLACKBOX_API(error_code) bbx_config_set_engine_major_version(error_code major_version);

/**
 * @brief set engine minor version associated with crash handler
 *
 * @param minor_version engine minor version associated
 * @return error_code any errors encountered
 */
BLACKBOX_API(error_code) bbx_config_set_engine_minor_version(error_code minor_version);

/**
 * @brief set engine patch version associated with crash handler
 *
 * @param patch_version engine patch version associated
 * @return error_code any errors encountered
 */
BLACKBOX_API(error_code) bbx_config_set_engine_patch_version(error_code patch_version);

/**
 * @brief enable or disable sdk features partially
 *
 * @param enable the boolean switch to enable or disable sdk features partially
 * @return error_code any errors encountered
 */
BLACKBOX_API(error_code) bbx_config_set_enable_sdk(bool enable);

/**
 * @brief set is issue reporter enabled
 *
 * @param is_issue_reporter_enabled new issue reporter enabled status
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_config_set_enable_issue_reporter(bool is_issue_reporter_enabled);

/**
 * @brief set issue reporter hotkey
 *
 * @param issue_reporter_hotkey new issue reporter hotkey
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_config_set_issue_reporter_hotkey(const char* issue_reporter_hotkey);

/**
 * @brief set config about using engine to capture screenshot
 *
 * @param use_engine_for_report_screenshot new status to use engine screenshot to capture report screenshot
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code)
bbx_config_set_use_engine_to_capture_screenshot(bool use_engine_for_report_screenshot);

/**
 * @brief set config about including log in issue report or not
 *
 * @param include_log_in_issue_report new status to include log in issue report
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code)
bbx_config_set_include_log_in_issue_report(bool include_log_in_issue_report);

/**
 * @brief Set to true if the game BlackBoxSDK attached to is a server entity
 *
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_config_set_is_running_as_server(bool in_is_server);

/**
 * @brief set iam user id for session creation
 *
 * @param iam_user_id the iam user id to be set
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_config_set_iam_user_id(const char* iam_user_id);

/**
 * @brief set playtest id for session creation
 *
 * @param playtest_id the playtest id to be set
 * @return error_code any errors encountered
 */
BLACKBOX_API(error_code) bbx_config_set_playtest_id(const char* playtest_id);

/**
 * @brief set device id for session creation
 *
 * @param device_id the device id to be set
 * @return error_code any errors encountered
 */
BLACKBOX_API(error_code) bbx_config_set_device_id(const char* device_id);

/**
 * @brief import initial config from .ini file
 *
 * @param path initial config file location
 * @return error_code any errors encountered
 */
BLACKBOX_API(error_code) bbx_import_default_config(const char* path);

// Informations

/**
 * @brief Get host OS architecture
 *
 * @return const char* architecture string
 */
BLACKBOX_API(const char*) bbx_info_get_os_architecture();

/**
 * @brief Get host OS name
 *
 * @return const char* OS name string
 */
BLACKBOX_API(const char*) bbx_info_get_os_name();

/**
 * @brief Get host OS version
 *
 * @return const char* OS version string
 */
BLACKBOX_API(const char*) bbx_info_get_os_version();

/**
 * @brief Get logged in host machine username
 *
 * @return const char* username string
 */
BLACKBOX_API(const char*) bbx_info_get_host_user_name();

/**
 * @brief Get logged in host machine name
 *
 * @return const char* machine name string
 */
BLACKBOX_API(const char*) bbx_info_get_computer_name();

/**
 * @brief Get SDK version
 *
 * @return const char* SDK version string
 */
BLACKBOX_API(const char*) bbx_info_get_version();

/**
 * @brief Load local config preferences
 * @param game_name
 *
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_load_local_config(const char* game_name);

/**
 * @brief Save local config preferences
 * @param game_name
 *
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_save_local_config(const char* game_name);

/**
 * @brief set local config enable crash reporter
 * @param new_local_config_value expected value: ["1", "0", "webconfig"]
 *
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_local_config_set_enable_crash_reporter(const char* new_local_config_value);

/**
 * @brief set local config store dxdiag
 * @param new_local_config_value expected value: ["1", "0", "webconfig"]
 *
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_local_config_set_store_dxdiag(const char* new_local_config_value);

/**
 * @brief set local config store crash video
 * @param new_local_config_value expected value: ["1", "0", "webconfig"]
 *
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_local_config_set_store_crash_video(const char* new_local_config_value);

/**
 * @brief set local config enable basic profiling
 * @param new_local_config_value expected value: ["1", "0", "webconfig"]
 *
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_local_config_set_enable_basic_profiling(const char* new_local_config_value);

/**
 * @brief get local config store crash video
 *
 * @return const char* store crash video local config value ["1", "0", "webconfig"]
 */
BLACKBOX_API(const char*) bbx_local_config_get_enable_crash_reporter();

/**
 * @brief get local config store crash video
 *
 * @return const char* store crash video local config value ["1", "0", "webconfig"]
 */
BLACKBOX_API(const char*) bbx_local_config_get_store_dxdiag();

/**
 * @brief get local config store crash video
 *
 * @return const char* store crash video local config value ["1", "0", "webconfig"]
 */
BLACKBOX_API(const char*) bbx_local_config_get_store_crash_video();

/**
 * @brief get local config store crash video
 *
 * @return const char* store crash video local config value ["1", "0", "webconfig"]
 *
 */
BLACKBOX_API(const char*) bbx_local_config_get_enable_basic_profiling();

/**
 * @brief Issue reporter API to capture screenshot with Helper and launch Issue Reporter
 *
 * @param parent_path for the issue_reporter
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_capture_screenshot(const char* parent_path);

/**
 * @brief Get issue report directory
 *
 * @param parent_path for issue_report_directory
 * @param out_dir to store get issue report directory result
 * @param out_dir_size size of get issue report directory result
 * @return bool to check if get the issue report directory succeeded or not
 */
BLACKBOX_API(bool) bbx_get_issue_report_directory(const char* parent_path, char* out_dir, size_t out_dir_size);

/**
 * @brief Issue reporter API to launch Issue Reporter only
 *
 * @param parent_path for the issue_reporter
 */
BLACKBOX_API(void) bbx_launch_issue_reporter(const char* parent_path);

// Crash Handling
#if BLACKBOX_WINDOWS_PLATFORM
/**
 * @brief Handle the client application crash for windows platform
 */
BLACKBOX_API(void) bbx_handle_crash();

#elif BLACKBOX_XBOX_ONE_PLATFORM
BLACKBOX_API(void)
bbx_handle_crash(
    const wchar_t* minidump_path,
    const wchar_t* log_path,
    void* runtime_xml_data,
    size_t runtime_xml_size,
    char* stacktrace,
    size_t stacktrace_size);

BLACKBOX_API(void) bbx_send_crash(const char* crashbin_dir, void (*cb)(bool));

#elif BLACKBOX_XBOXGDK_PLATFORM
BLACKBOX_API(void) bbx_save_crash_video();
BLACKBOX_API(void) bbx_set_gamertag(const char* gamertag);
BLACKBOX_API(void)
bbx_handle_crash(
    const wchar_t* minidump_path,
    const wchar_t* log_path,
    void* runtime_xml_data,
    size_t runtime_xml_size,
    char* stacktrace,
    size_t stacktrace_size);

BLACKBOX_API(void) bbx_send_crash(const char* crashbin_dir, void (*cb)(bool), bool should_wait = false);

#elif BLACKBOX_PS4_PLATFORM || BLACKBOX_PS5_PLATFORM
BLACKBOX_API(void) bbx_init_crash_handler();

#elif BLACKBOX_LINUX_PLATFORM
BLACKBOX_API(bool) bbx_handle_crash(int signal, siginfo_t* info, bbx_ucontext_buffer* buff);

BLACKBOX_API(void) bbx_init_crash_handler(const char* crash_dir);

#elif BLACKBOX_MAC_PLATFORM

BLACKBOX_API(void) bbx_handle_crash();
BLACKBOX_API(void) bbx_init_crash_handler(const char* crash_dir);

#endif

// Hitching handling

/**
 * @brief Set hitching video location
 *
 * @param parent_path for hitching_video_location
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_set_hitching_video_location(const char* parent_path);

/**
 * @brief Set whether is_hitching is true or false
 *
 * @param value value to set whether is_hitching is true or false
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code) bbx_set_is_hitching(bool value);

/**
 * @brief Validate config for APIKey, game version ID, and name space.
 *
 * @return bool Return true if validation success
 */
BLACKBOX_API(bool) bbx_validate_config();

/**
 * @brief Notify sdk that the game changes the resolution
 */

BLACKBOX_API(void) bbx_notify_change_game_resolution();

/**
 * @brief Create blackbox temp directory.
 *
 * @return const char* Created directory path
 */
BLACKBOX_API(const char*) bbx_generate_temp_dir();

/**
 * @brief Clean generated temp directory
 */
BLACKBOX_API(void) bbx_remove_temp_dir();

/**
 * @brief Converts a relative file path to an absolute path.
 * @param relative_path Input relative path.
 * @param out_abs_path Output buffer to store the absolute path.
 * @param max_abs_path_len Maximum size of the output buffer.
 * @param required_len Optional output parameter indicating the required buffer length, including null terminator. If
 * nullptr, it is ignored.
 *
 * @return error_code Indicating success, or the type of error encountered.
 */
BLACKBOX_API(error_code)
bbx_convert_relative_path_to_absolute(
    const char* relative_path, char* out_abs_path, size_t max_abs_path_len, size_t* required_len = nullptr);

/**
 * @brief Create blackbox session id alias folder to communicate with Blackbox Hub.
 */
BLACKBOX_API(void) bbx_generate_session_id_alias_folder(const char* session_id);

/**
 * @brief Create blackbox match session for session grouping function.
 * @param match_id match id from game BaaS platform
 * @param match_id_type the name of game BaaS used (e.g. Steam, Accelbyte Multiplayer Service, Epic OSS, etc)
 * @param callback callback function to call when the operation is completed
 *
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code)
bbx_create_match(const char* match_id, const char* match_id_type, match_created_callback_fn_t callback);

/**
 * @brief Begin BlackBox match session for session grouping function.
 * @param blackbox_match_id match id generated by BlackBox service from bbx_create_match call
 * @param callback callback function to call when the operation is completed
 *
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code)
bbx_begin_match_session(const char* blackbox_match_id, match_session_started_callback_fn_t callback);

/**
 * @brief End BlackBox match session for session grouping function.
 * @param blackbox_match_id match id generated by BlackBox service from bbx_create_match call
 * @param callback callback function to call when the operation is completed
 *
 * @return error_code Describe any errors encountered
 */
BLACKBOX_API(error_code)
bbx_end_match_session(const char* blackbox_match_id, match_session_ended_callback_fn_t callback);

/**
 * @brief Check if a feature is enabled using environment variable
 * @param envar_name name of the envar that sets if the feature is enabled or not
 *
 * @return bool Returns true if the feature is enabled
 */
BLACKBOX_API(bool) bbx_get_is_feature_enabled_from_env(const char* envar_name);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ACCELBYTE_BLACKBOX_H
