#include <execinfo.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>

#include "error_handler.h"
#include "json_builder.h"
#include "crash_report_fields.h"


ModuleInfo g_module = {};

std::string ErrorHandler::crashFolderStr;
const char* ErrorHandler::crashFolder = nullptr;

void ErrorHandler::setupSignalHandlers( const std::string& pathFolderCrash) {
    setCrashFolder(pathFolderCrash);
    ensureCrashFolderExists();

    struct sigaction sa{};
    sa.sa_sigaction = &ErrorHandler::signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO | SA_RESTART;

    sigaction(SIGSEGV, &sa, nullptr);
    sigaction(SIGABRT, &sa, nullptr);
    sigaction(SIGFPE,  &sa, nullptr);
    sigaction(SIGILL,  &sa, nullptr);
    sigaction(SIGBUS,  &sa, nullptr);
}

uintptr_t ErrorHandler::getBaseAddress(const char* modulePath) {
    FILE* maps = fopen("/proc/self/maps", "r");
    if (!maps) return 0;

    char line[512];
    while (fgets(line, sizeof(line), maps)) {
        if (strstr(line, modulePath)) {
            // пример строки: "7f7b9d4d7000-7f7b9d6d7000 r-xp 00000000 fd:01 123456 /path/to/module"
            char addr_start_str[32] = {};
            char* dash = strchr(line, '-');
            if (!dash) continue;

            size_t len = dash - line;
            if (len >= sizeof(addr_start_str)) len = sizeof(addr_start_str) - 1;
            memcpy(addr_start_str, line, len);
            addr_start_str[len] = '\0';

            uintptr_t base = 0;
            sscanf(addr_start_str, "%lx", &base);

            fclose(maps);
            return base;
        }
    }
    fclose(maps);
    return 0;
}

void ErrorHandler::ensureCrashFolderExists() {
    struct stat st;
    if (stat(crashFolder, &st) != 0) {
        // Папка не существует — создаём с правами 0755
        mkdir(crashFolder, 0755);
    } else if (!S_ISDIR(st.st_mode)) {
        // Существующий путь не папка — обработай по ситуации
    }
}

void ErrorHandler::signalHandler(int sig, siginfo_t* info, void* ucontext) {
    // Сразу сбрасываем обработчик, чтобы при повторном сигнале был дефолтный
    signal(sig, SIG_DFL);

    // Получаем время как timestamp
    time_t t = time(nullptr);

    // Генерация имени файла: crash_<timestamp>.json
    char filename[1024];
    snprintf(filename, sizeof(filename), "%s/crash_%lld.json",
             crashFolder, static_cast<long long>(t));

    // Создание файла
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) _exit(1);

    // Строка времени (вместо localtime_r/strftime — просто timestamp)
    char timeBuf[32];
    snprintf(timeBuf, sizeof(timeBuf), "%lld", static_cast<long long>(t));

    // Получаем Instruction pointer из ucontext
    ucontext_t* ctx = (ucontext_t*)ucontext;
#if defined(__x86_64__)
    void* ip = (void*)ctx->uc_mcontext.gregs[REG_RIP];
#elif defined(__i386__)
    void* ip = (void*)ctx->uc_mcontext.gregs[REG_EIP];
#else
    void* ip = nullptr;
#endif

    Dl_info infoDl{};
    const char* lib_name = "unknown";
    uintptr_t baseAddr = 0;
    if (ip && dladdr(ip, &infoDl) && infoDl.dli_fname) {
        lib_name = infoDl.dli_fname;
        baseAddr = reinterpret_cast<uintptr_t>(infoDl.dli_fbase);
    }


    // Имя библиотеки
    //const char* lib_name = g_module.name;
    //
    //uintptr_t baseAddr = g_module.start;
    uintptr_t ipAddr = reinterpret_cast<uintptr_t>(ip);
    uintptr_t offset = 0;
    if (baseAddr != 0 && ipAddr >= baseAddr) {
        offset = ipAddr - baseAddr;
    }

    const char* signame = nullptr;
    switch (sig) {
        case SIGSEGV: signame = "SIGSEGV"; break;
        case SIGABRT: signame = "SIGABRT"; break;
        case SIGFPE:  signame = "SIGFPE";  break;
        case SIGILL:  signame = "SIGILL";  break;
        case SIGBUS:  signame = "SIGBUS";  break;
        default:      signame = "UNKNOWN"; break;
    }


    // Формируем JSON объект
    auto root = std::make_shared<JsonObject>();
    auto signalObj = std::make_shared<JsonObject>();

    signalObj->insert(CrashReportFields::SIGNAL_NAME, JsonValue(signame));
    signalObj->insert(CrashReportFields::SIGNAL_NUMBER, JsonValue(sig));
    signalObj->insert(CrashReportFields::SIGNAL_ADDRESS, JsonValue((uintptr_t)info->si_addr));

    root->insert(CrashReportFields::TIME, JsonValue(timeBuf));
    root->insert(CrashReportFields::SIGNAL, JsonValue(signalObj));
    root->insert(CrashReportFields::BINARY, JsonValue(lib_name));
    root->insert(CrashReportFields::INSTRUCTION_POINTER, JsonValue(ipAddr));
    root->insert(CrashReportFields::OFFSET, JsonValue(offset));

    // Сериализация JSON в буфер (1024 байт)
    char buf[1024];
    JsonWriter writer(buf, sizeof(buf));
    if (writer.write(JsonValue(root))) {
        write(fd, buf, writer.length());
        write(fd, "\n", 1);
    } else {
        // При ошибке сериализации пишем простую строку
        const char* err = "{\"error\":\"json write failed\"}\n";
        write(fd, err, strlen(err));
    }

    close(fd);
    _exit(128 + sig);
}

