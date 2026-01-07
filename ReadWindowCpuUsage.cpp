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
void readRamInfoWithInterval(int interval, std::atomic<bool> &isRunning)
{
    while (isRunning)
    {
        RamInfo ramInfo = getRamInfo();
        std::cout << "RAM Used: " << ramInfo.RamUsed << "%, Used Bytes: " << ramInfo.UsedBytes / (1024 * 1024) << " MB\n";
        std::this_thread::sleep_for(std::chrono::seconds(interval));
    }
}
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
void readCpuWithIntervalSeconds(int interval, std::atomic<bool> &isRunning)
{
    while (isRunning)
    {
        std::cout << "CPU: " << cpu_percent_system() << "%\n";
        std::this_thread::sleep_for(std::chrono::seconds(interval));
    }
}
int main()
{
    std::cout << "Press Enter to stop...\n";
    spdlog::set_level(spdlog::level::info);

    // Log messages using the default logger
    spdlog::info("Welcome to spdlog!"); // Python-like formatting
    std::atomic<bool> isRunning{true};
    std::thread readCpu(readCpuWithIntervalSeconds, 1, std::ref(isRunning));
    std::thread readRam(readRamInfoWithInterval, 2, std::ref(isRunning));
    if (std::cin.get() == '\n')
    {
        isRunning = false;
    }
    readCpu.join();
    readRam.join();
    return 0;
}
