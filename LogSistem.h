#pragma once
#include <fstream>
#include <string>

extern std::ofstream logStream;

void Log(std::ostream& os, const std::string& message);
void InitializeLog();