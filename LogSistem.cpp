#include "LogSistem.h"
#include <Windows.h>
#include <chrono>
#include <format>

std::ofstream logStream;

void Log(std::ostream& os, const std::string& message) {
    os << message << std::endl;
    OutputDebugStringA(message.c_str());
}

void InitializeLog() {
    auto now = std::chrono::system_clock::now();
    auto nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
    std::chrono::zoned_time localTime{ std::chrono::current_zone(), nowSeconds };
    std::string dateString = std::format("{:%Y%m%d_%H%M%S}", localTime);

    std::string logFilePath = std::string("logs/") + dateString + ".log";
    logStream.open(logFilePath);
}