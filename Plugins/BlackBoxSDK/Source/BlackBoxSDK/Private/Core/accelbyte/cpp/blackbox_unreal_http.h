#pragma once

#include "../blackbox_common.h"

#include <cstdint>
#include <string>
#include <map>
#include <mutex>
#include <vector>

namespace blackbox {
namespace unreal {
struct binder_request {
    uint64_t idx = 0;
    uint64_t id = 0;
    std::string url = "";
    std::string method = "";
    std::map<std::string, std::string> headers;
    std::string body = "";
    bool fired = false;
    bool done = false;

    /** response */
    uint32_t status = 0;
    int32_t error_code = 0;
    std::map<std::string, std::string> resp_headers;
    std::string resp_body = "";

    binder_request() {}

    binder_request(
        uint64_t idx,
        uint64_t id,
        std::string url,
        std::string method,
        std::map<std::string, std::string> headers,
        std::string body,
        bool fired = false,
        bool done = false)
        : idx(idx)
        , id(id)
        , url(url)
        , method(method)
        , headers(headers)
        , body(body)
        , fired(fired)
        , done(done)
    {
    }
};
class BLACKBOX_INTERFACE unreal_http_caller {
public:
    virtual ~unreal_http_caller();

    static unreal_http_caller* get_instance();

    static void set_instance(unreal_http_caller* new_instance);

    static void destroy_instance();

    virtual bool add_request(
        uint32_t idx,
        uint64_t id,
        const std::string& method,
        const std::string& url,
        const std::map<std::string, std::string>& headers,
        const std::string& body) = 0;

    virtual size_t fire_requests() = 0;

    virtual size_t get_binding_requests(binder_request** out_requests);

    virtual bool remove_request(uint64_t id) = 0;

    size_t active_request_count();

protected:
    static unreal_http_caller* instance;

    SUPRESS_WARNING_BEGIN // C4251
        std::mutex request_mutex;
        
        std::vector<binder_request>
            requests;
    SUPRESS_WARNING_END
};
BLACKBOX_API(void) set_unreal_singleton(unreal_http_caller* singleton);
BLACKBOX_API(void) destroy_unreal_singleton();
} // namespace unreal
} // namespace blackbox