#include "devicecontroller.h"

#include <QProcess>
#include <QDebug>

DeviceController::DeviceController(QObject *parent)
    : QObject(parent)
{

}

void DeviceController::setAdbPath(const QString& path)
{
    m_adbPath = path;
}

void DeviceController::setIp(const QString &ip)
{
    m_ip = ip;
}

bool DeviceController::execAdb(const QStringList& args)
{
    QProcess adb;

    adb.start(m_adbPath, args);
    if(!adb.waitForFinished(3000)) {
        emit actionError("ADB timeout");
        return false;
    }

    int code = adb.exitCode();
    if(code != 0) {
        emit actionError("ADB command failed");
        return false;
    }

    return true;
}

void DeviceController::disconnectAll()
{
    execAdb({"disconnect"});
}

void DeviceController::disconnect()
{
    if (m_ip.isEmpty()) {
        emit actionError("IP is empty");    
    } else {
        execAdb({"disconnect", m_ip});
    }
}

void DeviceController::connect()
{
    if (m_ip.isEmpty()) {
        emit actionError("IP is empty"); 
    } else {
        execAdb({"connect", m_ip});
    }
}

bool DeviceController::tap(int x,int y)
{
    QStringList args;
    args << "shell"
         << "input"
         << "tap"
         << QString::number(x)
         << QString::number(y);

    bool ok = execAdb(args);
    if(ok)
        emit actionExecuted(QString("tap (%1,%2)").arg(x).arg(y));

    return ok;
}

bool DeviceController::swipe(int x1,int y1,int x2,int y2,int duration)
{
    QStringList args;
    args << "shell"
         << "input"
         << "swipe"
         << QString::number(x1)
         << QString::number(y1)
         << QString::number(x2)
         << QString::number(y2)
         << QString::number(duration);

    bool ok = execAdb(args);
    if(ok)
        emit actionExecuted(
            QString("swipe (%1,%2)->(%3,%4)")
            .arg(x1).arg(y1).arg(x2).arg(y2));

    return ok;
}

bool DeviceController::keyEvent(int key)
{
    QStringList args;
    args << "shell"
         << "input"
         << "keyevent"
         << QString::number(key);

    bool ok = execAdb(args);
    if(ok)
        emit actionExecuted(
            QString("keyevent %1").arg(key));

    return ok;
}

bool DeviceController::inputText(const QString& text)
{
    QStringList args;
    args << "shell"
         << "input"
         << "text"
         << text;

    bool ok = execAdb(args);
    if(ok)
        emit actionExecuted(
            QString("input text: %1").arg(text));

    return ok;
}