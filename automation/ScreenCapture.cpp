#include "screencapture.h"

#include <QProcess>

namespace {
const QByteArray kPngSignature("\x89PNG\r\n\x1A\n", 8);
}

ScreenCapture::ScreenCapture(QObject *parent)
    : QObject(parent), m_config(std::make_shared<AdbConfig>())
{
    m_timer.setParent(this);
    connect(&m_timer,
            &QTimer::timeout,
            this,
            &ScreenCapture::captureOnce);
}

void ScreenCapture::setConfig(const std::shared_ptr<AdbConfig> &config)
{
    if (config) {
        m_config = config;
    }
}

void ScreenCapture::start(int intervalMs)
{
    m_timer.start(intervalMs);
}

void ScreenCapture::stop()
{
    m_timer.stop();
}

void ScreenCapture::captureOnce()
{
    if (m_config->ip.isEmpty()) {
        emit captureError("IP is empty");
        return;
    }
    QProcess adb;
    QStringList args;
    args << "-s" << m_config->ip << "exec-out" << "screencap" << "-p";
    adb.start(m_config->adbPath, args);
    if(!adb.waitForStarted(1000)) {
        emit captureError("Failed to start ADB");
        return;
    }

    if(!adb.waitForFinished(3000)) {
        emit captureError("ADB timeout");
        return;
    }

    //错误检查
    const int code = adb.exitCode();
    const QString stderrText = QString::fromUtf8(adb.readAllStandardError()).trimmed();
    if (code != 0) {
        if (stderrText.isEmpty()) {
            emit captureError(QString("ADB command failed (exit %1)").arg(code));
        } else {
            emit captureError(QString("ADB command failed: %1").arg(stderrText));
        }
        return;
    }

    //读取PNG数据
    QByteArray data = adb.readAllStandardOutput();
    if(data.isEmpty()) {
        emit captureError("Empty screenshot data");
        return;
    }

    //一些adb版本可能在PNG数据前面添加一些文本，下面的代码确保我们只保留从PNG签名开始的数据。
    const int pngStart = data.indexOf(kPngSignature);
    if (pngStart < 0) {
        emit captureError("Invalid screenshot stream (PNG signature not found)");
        return;
    }
    if (pngStart > 0) {
        data = data.mid(pngStart);
    }

    //解码PNG数据为QImage
    QImage img;
    if(!img.loadFromData(data,"PNG")) {
        emit captureError("Failed to decode image (corrupted PNG data)");
        return;
    }

    emit frameReady(img);
}