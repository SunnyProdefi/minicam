#pragma once
#include "types.h"
#include <functional>
#include <map>
#include <vector>
#include <string>

namespace mc
{

    struct Settings
    {
        std::map<std::string, int> ints;
        std::map<std::string, float> floats;
    };

    struct StreamConfig
    {
        int width{640}, height{480};
        PixelFormat fmt{PixelFormat::RGBA8888};
        std::string purpose{"PREVIEW"};
    };

    struct CaptureRequest
    {
        int request_id{0};
        Settings settings;
        std::vector<StreamConfig> streams;
        bool async{true};
    };

    struct CaptureResult
    {
        int request_id{0};
        Error status{Error::OK};
        std::string artifact_path;
    };

    using ResultCallback = std::function<void(const CaptureResult&)>;

}  // namespace mc
