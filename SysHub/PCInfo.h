#pragma once

#include <string>
#include <vector>

struct PCInfo
{
    std::string os_;
    std::string arch_;
    std::string cpuName_;
    std::string gpuName_;
    std::string gpuStatus_;
    std::vector<std::string> openCLSupport_;
    unsigned cpuSpeed_;
    unsigned cpuCoreCount_;
    unsigned ram_;
    unsigned videoRam_;
};