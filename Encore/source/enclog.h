//
// Created by Jaydenz on 6/9/2025.
//

#ifndef ENCLOG_H
#define ENCLOG_H
#include <cstdarg>

namespace Encore {
    void EncoreLog(int msgType, const char *text, va_list args);
    void EncoreLog(int msgType, const char *text);
}

#endif //ENCLOG_H
