#pragma once
#include "../core/request.h"
#include <unordered_map>
#include <mutex>
#include <memory>

namespace mc
{

    struct RequestState
    {
        int remaining{0};
        ResultCallback cb;
    };

    struct PipelineBus
    {
        std::mutex m;
        std::unordered_map<int, RequestState> req;
    };

}  // namespace mc
