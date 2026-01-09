#include <windows.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h" // Include this for colored console output

static unsigned long long u64(const FILETIME &ft)
{
    return (static_cast<unsigned long long>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
}
struct RamInfo
{
    double RamUsed;
    double UsedBytes;
};

struct DiskInfo
{
    double DiskUsed;    // %
    double UsedBytes;   // bytes
    double TotalBytes;  // bytes
};

struct SystemInfo
{
    double CpuPercent = 0.0;
    RamInfo Ram{};
    DiskInfo Disk{};
};
 // Function to get disk usage information
DiskInfo getDiskInfo(const wchar_t *path = L"C:\\")
{
    ULARGE_INTEGER freeBytesAvailable, totalBytes, freeBytes;
    if (!GetDiskFreeSpaceExW(path, &freeBytesAvailable, &totalBytes, &freeBytes))
    {
        spdlog::error("GetDiskFreeSpaceExW failed");
        return {0.0, 0.0, 0.0};
    }

    double total = static_cast<double>(totalBytes.QuadPart);
    double free = static_cast<double>(freeBytes.QuadPart);
    double used = total - free;
    double percent = (total > 0.0) ? (used / total) * 100.0 : 0.0;

    return {percent, used, total};
}

void readDiskUsageWithInterval(int intervalSeconds, std::atomic<bool> &isRunning, SystemInfo& systemInfo, std::mutex &mutex, const wchar_t *path = L"C:\\")
{
    while (isRunning)
    {
        {
            std::lock_guard<std::mutex> lock(mutex);
            DiskInfo disk = getDiskInfo(path);
            systemInfo.Disk = disk;
        }
        std::this_thread::sleep_for(std::chrono::seconds(intervalSeconds));
    }
}

// Function to get RAM usage information
RamInfo getRamInfo()
{
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);

    double totalPhysMem = static_cast<double>(memInfo.ullTotalPhys);
    double physMemUsed = static_cast<double>(memInfo.ullTotalPhys - memInfo.ullAvailPhys);
    double ramUsedPercent = (physMemUsed / totalPhysMem) * 100.0;

    RamInfo info;
    info.RamUsed = ramUsedPercent;
    info.UsedBytes = physMemUsed;
    return info;
}
void readRamInfoWithInterval(int interval, std::atomic<bool> &isRunning, SystemInfo& systemInfo, std::mutex &mutex)
{
    while (isRunning)
    {
        {
            std::lock_guard<std::mutex> lock(mutex);
            RamInfo ramInfo = getRamInfo();
            systemInfo.Ram = ramInfo;
        }
        std::this_thread::sleep_for(std::chrono::seconds(interval));
    }
}
// Function to get CPU usage percentage
double cpu_percent_system()
{
    FILETIME idle1, kernel1, user1, idle2, kernel2, user2;
    GetSystemTimes(&idle1, &kernel1, &user1);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    GetSystemTimes(&idle2, &kernel2, &user2);

    auto idle = u64(idle2) - u64(idle1);
    auto kernel = u64(kernel2) - u64(kernel1);
    auto user = u64(user2) - u64(user1);

    auto total = kernel + user;
    if (total == 0)
        return 0.0;
    return (1.0 - (double)idle / (double)total) * 100.0;
}
void readCpuWithIntervalSeconds(int interval, std::atomic<bool> &isRunning,SystemInfo& systemInfo, std::mutex &mutex)
{
    while (isRunning)
    {
        {
            std::lock_guard<std::mutex> lock(mutex);
            systemInfo.CpuPercent = cpu_percent_system();
        }
        std::this_thread::sleep_for(std::chrono::seconds(interval));
    }
}
