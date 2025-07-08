#include "libtest.h"
#include "error_handler.h"
#include <iostream>
#include <stdexcept>
#include <csignal>

Libtest::Libtest()
{
    ErrorHandler::setupSignalHandlers();
}

void Libtest::generateError(){
    int *p = nullptr;
        *p = 42; // вызовет SIGSEGV
}
