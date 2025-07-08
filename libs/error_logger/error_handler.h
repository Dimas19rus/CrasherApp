#pragma once

#include <signal.h>
#include <cstdint>
#include <string>


struct ModuleInfo {
    uintptr_t start;
    uintptr_t end;
    const char* name;
};

extern ModuleInfo g_module;


class ErrorHandler {
public:
    static void setupSignalHandlers(const std::string& pathFolderCrash = "./crash");
    static uintptr_t getBaseAddress(const char* modulePath);
    static int counter;
    static const std::string& getCrashFolder() { return crashFolderStr; }
    static const char* getCrashFolderCStr() { return crashFolder; }

private:
    static void preloadModuleInfo();
    static void setCrashFolder(const std::string& path) {
        crashFolderStr = path;
        crashFolder = crashFolderStr.c_str();
    }
    static void ensureCrashFolderExists();
    static void signalHandler(int sig, siginfo_t* info, void* ucontext);

    static std::string crashFolderStr;
    static const char* crashFolder;

};
