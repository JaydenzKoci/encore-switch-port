//
// Created by Jaydenz on 6/9/2025.
//

#include "enclog.h"
#include <raylib.h>

#include <iomanip>
#include <iostream>
#include <sstream>
#include <cstring>

void Encore::EncoreLog(int msgType, const char *text, va_list args) {
    std::ostringstream outputString;

    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    outputString << "[";
    outputString << std::put_time(&tm, "%H:%M:%S");
    outputString << "] ";

    outputString << "[Encore:";

    char buffer[256] = { 0 };
    char outputbuf[256] = { 0 };
    unsigned int textSize = (unsigned int)strlen(text);
    memcpy(buffer + strlen(buffer), text, (textSize < (256 - 12))? textSize : (256 - 12));
    vsprintf(outputbuf, buffer, args);

    switch (msgType)
    {
    case LOG_INFO:
        outputString << "INFO] " << outputbuf;
        std::cout << "  " << outputString.str() << std::endl;
        break;
    case LOG_ERROR:
        outputString <<"ERROR] " << outputbuf;
        std::cerr << "x " << outputString.str() << std::endl;
        break;
    case LOG_WARNING:
        outputString <<"WARN] " << outputbuf;
        std::cerr << "! " << outputString.str() << std::endl;
        break;
    case LOG_DEBUG:
        outputString <<"DEBUG] " << outputbuf;
        std::cout << "@ " << outputString.str() << std::endl;
        break;
    default: break;
    }

}

void Encore::EncoreLog(int msgType, const char *text)
{
    std::ostringstream outputString;

    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    outputString << "[";
    outputString << std::put_time(&tm, "%H:%M:%S");
    outputString << "] ";

    outputString << "[Encore:";



    switch (msgType)
    {
    case LOG_INFO:
        outputString << "INFO] ";
        outputString << text;
        std::cout << "  " << outputString.str() << std::endl;
        break;
    case LOG_ERROR:
        outputString <<"ERROR] ";
        outputString << text;
        std::cerr << "x " << outputString.str() << std::endl;
        break;
    case LOG_WARNING:
        outputString <<"WARN] ";
        outputString << text;
        std::cerr << "! " << outputString.str() << std::endl;
        break;
    case LOG_DEBUG:
        outputString <<"DEBUG] ";
        outputString << text;
        std::cout << "@ " << outputString.str() << std::endl;
        break;
    default: break;
    }
}
