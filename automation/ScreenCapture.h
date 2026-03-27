#ifndef SCREENCAPTURE_H
#define SCREENCAPTURE_H

#include <QObject>
#include <QImage>
#include <QTimer>

/**
 * @brief 屏幕捕获器，负责定期捕获屏幕图像
 * 
 */
class ScreenCapture : public QObject
{
    Q_OBJECT

public:
    explicit ScreenCapture(QObject *parent = nullptr);

    /**
     * @brief 开始屏幕捕获
     * @param intervalMs 捕获间隔，单位毫秒
     */
    void start(int intervalMs = 500);
    /**
     * @brief 停止屏幕捕获
     */
    void stop();

    void setIp(const QString& ip) {
        this->m_ip = ip;
    }

signals:
    void frameReady(const QImage& img);
    void captureError(const QString& msg);

private slots:
    void captureOnce();

private:
    QTimer m_timer;
    QString m_adbPath = "adb";
    QString m_ip = "127.0.0.1:16384";

};

#endif