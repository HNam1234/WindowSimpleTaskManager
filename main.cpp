#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h" // Include this for colored console output
#include "SystemInfoReader.h"
#include <sstream>
int main()
{   
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("[%H:%M:%S] [%^%l%$] %v"); // Set pattern for colored output
    spdlog::info("Starting CPU and RAM monitoring...");
    spdlog::info("Press Enter to stop the monitoring.");
    std::atomic<bool> isRunning{true};
    std::mutex systemInfoMutex; // Mutex for synchronizing access to shared data
    SystemInfo systemInfo;
    // Multithreading: 
        /*
        - is a technique where a program is divided into smaller unit of execution called threads
        - each thread shared the same memory but run independently , allowing tasks to be performed simultaneously
        - help cpu utilizing multiple cpu cores efficiently
        *Pros:
        - Leverages multiple CPU cores to execute tasks in parallel, reducing overall execution time
        - Keeps applications responsive by running background operations without blocking the main thread. For example, in a word document, one thread does auto-formatting along with the main thread
        - Makes it easier to handle large workloads or multiple simultaneous operations, such as in servers or real-time systems.
        * Before C++ 11, multithreading is very complicated, in linux, we use pthread, in window use WIN32 thread, or 3rd library like boost::thread. C++ 11 add feature like boost thread : std::thread make multithreading much easier and crossplatform.
        *Cons: (*AI generated, may not be 100% accurate)
        - Deadlocks: Occur when two or more threads are waiting indefinitely for each other to release resources, causing the program to hang.
        - Race Conditions: Happen when multiple threads access shared data simultaneously, leading to unpredictable results if not properly synchronized.
        - Starvation: A thread may be perpetually denied access to resources it needs to proceed, often due to other higher-priority threads monopolizing those resources.
        - Increased Complexity: Multithreaded programs are generally more complex to design, implement, and debug compared to single-threaded ones.
        - Overhead: Context switching between threads and synchronization mechanisms can introduce performance overhead, potentially negating the benefits of multithreading for small tasks.
        */
    // Creating a thread to read CPU usage every second
    std::thread readCpu(readCpuWithIntervalSeconds, 1, std::ref(isRunning), std::ref(systemInfo), std::ref(systemInfoMutex));
    // Creating a thread to read RAM usage every 2 seconds
    std::thread readRam(readRamInfoWithInterval, 2, std::ref(isRunning), std::ref(systemInfo), std::ref(systemInfoMutex));
    // Creating a thread to read Disk usage every 5 seconds
    std::thread readDisk(readDiskUsageWithInterval, 5, std::ref(isRunning), std::ref(systemInfo), std::ref(systemInfoMutex), L"C:\\");
    // Detach disk thread as we don't need to join it later, thats mean, we will lose control of this thread
    // the only way to stop it is to stop the whole program or using isRunning flag = false.
    readDisk.detach();

    auto thread_id_str = [](std::thread::id id) {
        std::stringstream ss;
        ss << id;
        return ss.str();
    };

    spdlog::info("readCpu thread id: {}", thread_id_str(readCpu.get_id()));
    spdlog::info("readRam thread id: {}", thread_id_str(readRam.get_id()));
    // expect thread id readDisk is 0 as it was detached, the readDisk is empty thread object now
    spdlog::info("readDisk thread id: {}", thread_id_str(readDisk.get_id()));
    spdlog::info("main thread id: {}", thread_id_str(std::this_thread::get_id()));
    // create a thread to wait for user input to stop the monitoring using lambda function instead of function pointer
    std::thread stopper([](std::atomic<bool> &isRunning) {
        // Wait for user to press Enter
        if (std::cin.get() == '\n')
        {
            isRunning = false;
        }
    }, std::ref(isRunning));
    spdlog::info("stopper thread id: {}", thread_id_str(stopper.get_id()));

    // Logger thread: reads shared SystemInfo while other threads are writing (no mutex -> can reproduce data race)
    std::thread logger([](std::atomic<bool> &isRunning, SystemInfo &systemInfo, std::mutex &systemInfoMutex) {
        while (isRunning)
        {
            std::lock_guard<std::mutex> lock(systemInfoMutex);
            spdlog::info("LOGGER -> CPU: {}%, RAM: {}%, Disk: {}%",
                         systemInfo.CpuPercent,
                         systemInfo.Ram.RamUsed,
                         systemInfo.Disk.DiskUsed);
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
    }, std::ref(isRunning), std::ref(systemInfo), std::ref(systemInfoMutex));
    spdlog::info("logger thread id: {}", thread_id_str(logger.get_id()));

    // Join threads before exiting, check joinable to avoid join twice or join a thread that was not started
    // can't join readDisk as it was detached
    if(stopper.joinable()) stopper.join();
    if(readCpu.joinable()) readCpu.join();
    if(readRam.joinable()) readRam.join();
    if(readDisk.joinable()) readDisk.join();
    if(logger.joinable()) logger.join();
    spdlog::info("All monitoring threads have been stopped. Exiting program.");
    
    return 0;
}
