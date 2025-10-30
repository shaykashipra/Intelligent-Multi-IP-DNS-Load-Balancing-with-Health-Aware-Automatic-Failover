#ifndef LOGGER_H
#define LOGGER_H

#include <omnetpp.h>
using namespace omnetpp ;

class Logger {
public:
    static void info(const char* module, const char* format, ...) {
        va_list args;
        va_start(args, format);
        char buffer[256];
        vsnprintf(buffer, sizeof(buffer), format, args);
        EV << module << " INFO: " << buffer << "\n";
        va_end(args);
    }

    static void warn(const char* module, const char* format, ...) {
        va_list args;
        va_start(args, format);
        char buffer[256];
        vsnprintf(buffer, sizeof(buffer), format, args);
        EV << module << " WARN: " << buffer << "\n";
        va_end(args);
    }

    static void error(const char* module, const char* format, ...) {
        va_list args;
        va_start(args, format);
        char buffer[256];
        vsnprintf(buffer, sizeof(buffer), format, args);
        EV << module << " ERROR: " << buffer << "\n";
        va_end(args);
    }
};

#endif
