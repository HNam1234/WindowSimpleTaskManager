#include <windows.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h" // Include this for colored console output
#include "SystemInfoReader.h"
int main()
{   
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("[%H:%M:%S] [%^%l%$] %v"); // Set pattern for colored output
    spdlog::info("Starting CPU and RAM monitoring...");
    spdlog::info("Press Enter to stop the monitoring.");
    std::atomic<bool> isRunning{true};
    // Multithreading: 
        /*
        - is a technique where a program is divided into smaller unit of execution called threads
        - each thread shared the same memory but run independently , allowing tasks to be performed simultaneously
        - help cpu utilizing multiple cpu cores efficiently
        *Cons:
        - Leverages multiple CPU cores to execute tasks in parallel, reducing overall execution time
        - Keeps applications responsive by running background operations without blocking the main thread. For example, in a word document, one thread does auto-formatting along with the main thread
        - Makes it easier to handle large workloads or multiple simultaneous operations, such as in servers or real-time systems.
        * Before C++ 11, multithreading is very complicated, in linux, we use pthread, in window use WIN32 thread, or 3rd library like boost::thread. C++ 11 add feature like boost thread : std::thread make multithreading much easier and crossplatform.
        */
    // Creating a thread to read CPU usage every second
    std::thread readCpu(readCpuWithIntervalSeconds, 1, std::ref(isRunning));
    // Creating a thread to read RAM usage every 2 seconds
    std::thread readRam(readRamInfoWithInterval, 2, std::ref(isRunning));
    // Wait for user to press Enter
    if (std::cin.get() == '\n')
    {
        isRunning = false;
    }
    // Join threads before exiting, check joinable to avoid join twice or join a thread that was not started
    if(readCpu.joinable()) readCpu.join();
    if(readRam.joinable()) readRam.join();
    return 0;
}
