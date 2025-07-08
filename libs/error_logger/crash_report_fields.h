namespace CrashReportFields {
    constexpr const char* TIME = "time";                               // Формат: "YYYY-MM-DD HH:MM:SS"
    constexpr const char* SIGNAL = "signal";
    constexpr const char* SIGNAL_NAME = "name";                        // Пример: "SIGSEGV"
    constexpr const char* SIGNAL_NUMBER = "number";                    // Целое число, код сигнала
    constexpr const char* SIGNAL_ADDRESS = "address";                  // Адрес в hex, строка, например "0x7ffdeadbeef"
    constexpr const char* BINARY = "binary";                           // Имя бинарника/модуля
    constexpr const char* INSTRUCTION_POINTER = "instruction_pointer"; // Адрес инструкции в hex, строка
    constexpr const char* OFFSET = "offset";                           // Смещение от базового адреса в hex, строка
}
