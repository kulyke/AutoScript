#ifndef DEVICECONTROLLER_H
#define DEVICECONTROLLER_H

#include <QObject>
#include <QString>
#include <memory>

#include "../config/AdbConfig.h"

/**
 * @brief 设备控制器，负责通过ADB模拟用户操作
 * 
 */
class DeviceController : public QObject
{
    Q_OBJECT

public:
    //设备类型
    enum DeviceType {
        mumu,
        leishen,
        yeshen
    };

    explicit DeviceController(QObject *parent = nullptr);

    void setConfig(const std::shared_ptr<AdbConfig>& config);
    /**
     * @brief 断开与所有设备的连接
     */
    void disconnectAll();
    /**
     * @brief 断开与设备的连接
     */
    void disconnect();
    /**
     * @brief 连接到设备
     */
    void connect();
    /**
     * @brief 模拟点击
     * @param x 点击x坐标
     * @param y 点击y坐标
     * @return 是否执行成功
     */
    bool tap(int x,int y);
    /**
     * @brief 模拟滑动
     * @param x1 起点x坐标
     * @param y1 起点y坐标
     * @param x2 终点x坐标
     * @param y2 终点y坐标
     * @param duration 滑动持续时间，单位毫秒
     * @return 是否执行成功
     */
    bool swipe(int x1,int y1,int x2,int y2,int duration=300);
    /**
     *  常见 Android Key：
     *  3  HOME
     *  4  BACK
     *  24 VOLUME_UP
     *  25 VOLUME_DOWN
     *  26 POWER
     *  66 ENTER
     *  82 MENU
     * @brief 模拟按键事件
     * @param key Android KeyEvent code
     * @return 是否执行成功
     */
    bool keyEvent(int key);
    /**
     * @brief 模拟输入文本
     * @param text 要输入的文本
     * @return 是否输入成功
     */
    bool inputText(const QString& text);

signals:
    void actionExecuted(const QString& action);
    void actionError(const QString& err);

private:
    /**
     * @brief 执行adb命令
     * @param args 命令参数
     * @return 执行是否成功
     */
    bool execAdb(const QStringList& args);

    std::shared_ptr<AdbConfig> m_config;
    DeviceType m_deviceType = mumu;
};

#endif