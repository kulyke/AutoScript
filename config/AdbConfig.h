#ifndef ADBCONFIG_H
#define ADBCONFIG_H

#include <QString>

struct AdbConfig
{
    QString adbPath = "adb";
    QString ip = "127.0.0.1:16385";
};

#endif