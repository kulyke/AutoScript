#ifndef TASKBASE_H
#define TASKBASE_H

#include <QObject>
#include <QImage>

#include "StepFlowState.h"

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
    ~TaskBase() override;
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
    * @brief 设置任务实例唯一标识
    * @param taskId 任务实例ID
    */
    void setTaskId(const QString& taskId) { m_taskId = taskId; }
    /**
    * @brief 获取任务实例唯一标识
    * @return 任务实例ID
    */
    QString taskId() const { return m_taskId; }
    /**
     * @brief 设置初始状态
     * @param state 任务状态
     */
    void setInitialState(StepFlowState* state);
    /**
    * @brief 获取任务类型名称
    * @return 任务类型名称
    */
    QString taskTypeName() const { return name(); }
    /**
     * @brief 设置状态超时阈值（连续无进展帧数）
     */
    void setMaxStallFrames(int frames) { m_maxStallFrames = frames > 0 ? frames : 1; }
    /**
     * @brief 获取状态超时阈值
     * @return 状态超时阈值
     */
    int maxStallFrames() const { return m_maxStallFrames; }
    /**
     * @brief 执行任务
     * @param frame 当前屏幕截图
     */
    void execute(const QImage& frame);
    /**
     * @brief 获取当前状态名称
     * @return 当前状态名称
     */
    QString currentStateName() const;
    /**
     * @brief 获取任务失败原因
     * @return 任务失败原因字符串
     */
    QString failureReason() const;
    /**
     * @brief 获取任务运行时消息
     * @return 任务运行时消息字符串
     */
    QString takeRuntimeMessage();
    /**
     * @brief 获取任务名称
     * @return 任务名称
     */
    virtual QString name() const = 0;

protected:
    StepFlowState* m_currentState = nullptr;
    QString m_taskId;
    QString m_runtimeMessage;

    TaskStatus m_status = Idle; //默认状态为空闲
    int m_stallFrames = 0;       //连续无状态进展帧数
    int m_maxStallFrames = 10;   //超过后判定 NG
};

#endif