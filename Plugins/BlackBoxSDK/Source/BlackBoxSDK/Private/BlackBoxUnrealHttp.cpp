// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "BlackBoxCommon.h"
#if BLACKBOX_UE_PS5 || BLACKBOX_UE_PS4
#    include "BlackBoxUnrealHttp.h"
#    include <mutex>
#    include "Http.h"
#    include "Modules/ModuleManager.h"

#    include "BlackBoxLog.h"

namespace bbxu = blackbox::unreal;

bool bbxu::sdk_http_impl::add_request(
    uint32_t idx,
    uint64_t id,
    const std::string& method,
    const std::string& url,
    const std::map<std::string, std::string>& headers,
    const std::string& body)
{
    bbxu::binder_request request = {idx, id, url, method, headers, body};
    {
        std::unique_lock<std::mutex> lock(request_mutex);
        requests.push_back(request);
    }

    return true;
}

size_t bbxu::sdk_http_impl::fire_requests()
{
    size_t requests_fired = 0;

    {
        std::unique_lock<std::mutex> lock(request_mutex);
        for (auto& request : requests) {
            if (request.fired) {
                continue;
            }

            bool status = make_request(request);

            if (status) {
                requests_fired++;
            }
        }
    }

    return requests_fired;
}

bool bbxu::sdk_http_impl::remove_request(uint64_t id)
{
    std::unique_lock<std::mutex> lock(request_mutex);
    auto itr = std::find_if(
        std::begin(requests), std::end(requests), [&](const bbxu::binder_request& req) { return req.id == id; });

    if (itr != requests.end()) {
        requests.erase(itr);
        return true;
    }
    else {
        return false;
    }
}

bool bbxu::sdk_http_impl::make_request(bbxu::binder_request& request)
{
    auto HttpRequest = FHttpModule::Get().CreateRequest();

    // Note: This is required so that the lambda below will not try and copy the request object that will be
    // uninitialized by call time
    uint64 request_id = request.id;

    // Construct http request
    HttpRequest->OnProcessRequestComplete().BindLambda(
        [this, request_id](FHttpRequestPtr Request, FHttpResponsePtr Response, bool Successful) {
            auto itr = std::find_if(requests.begin(), requests.end(), [request_id](const bbxu::binder_request& req) {
                return req.id == request_id;
            });

            if (!Successful) {
                itr->done = true;
                has_finished = true;

                return;
            }
            else if (itr != requests.end()) {
                itr->status = Response->GetResponseCode();
                itr->resp_body = std::string(TCHAR_TO_UTF8(*Response->GetContentAsString()));
                itr->error_code = 0; // UE doesn't seem to have any way to get an underlying error code

                for (auto& header : Response->GetAllHeaders()) {
                    auto header_pair = get_header_as_pair(header);
                    itr->resp_headers.insert(header_pair);
                }

                itr->done = true;

                has_finished = true;
            }
        });

    TArray<uint8> BodyArr;

    for (char& byte : request.body) { BodyArr.Push(byte); }

    HttpRequest->SetURL(FString(request.url.c_str()));
    HttpRequest->SetVerb(FString(request.method.c_str()));
    HttpRequest->SetContent(BodyArr);

    for (auto& header : request.headers) {
        HttpRequest->SetHeader(FString(header.first.c_str()), FString(header.second.c_str()));
    }

    request.fired = true;

    // clear it so we don't OOM for crashbin upload
    request.body.clear();

    return HttpRequest->ProcessRequest();
}

std::pair<std::string, std::string> bbxu::sdk_http_impl::get_header_as_pair(FString& header)
{
    size_t colon_pos = header.Find(":");

    FString field = header.Mid(0, colon_pos).TrimStartAndEnd();
    FString value = header.Mid(colon_pos + 1).TrimStartAndEnd();

    return std::make_pair(std::string(TCHAR_TO_UTF8(*field)), std::string(TCHAR_TO_UTF8(*value)));
}

#endif