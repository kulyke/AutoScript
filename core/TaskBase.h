#ifndef TASKBASE_H
#define TASKBASE_H

#include <QObject>
#include <QImage>

#include "TaskState.h"

class VisionEngine;
class DeviceController;

/**
 * @brief 任务基类，所有具体任务都应继承自此类
 * 
 */
class TaskBase : public QObject
{
    Q_OBJECT

public:
    enum ResultStatus {
        Continue,   //继续执行
        Stop        //停止执行，任务完成或失败
    };
    enum TaskStatus {
        Idle,       //空闲
        Waiting,    //等待条件满足
        Running,    //运行
        OK,         //完成
        NG          //失败
    };
    explicit TaskBase(QObject *parent = nullptr);
    ~TaskBase() override = default;
    /**
     * @brief 设置任务状态
     * @param status 任务状态
     */
    void setStatus(TaskStatus status) { m_status = status; }
    /**
     * @brief 获取任务状态
     * @return 任务状态
     */
    TaskStatus status() const { return m_status; }
    /**
     * @brief 设置初始状态
     * @param state 任务状态
     */
    void setInitialState(TaskState* state);
    /**
     * @brief 执行任务
     * @param frame 当前屏幕截图
     */
    void execute(const QImage& frame);
    /**
     * @brief 获取任务名称
     * @return 任务名称
     */
    virtual QString name() const = 0;

protected:
    TaskState* m_currentState = nullptr;

    TaskStatus m_status = Idle; //默认状态为空闲
};

#endif