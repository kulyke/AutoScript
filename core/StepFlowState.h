#ifndef STEPFLOWSTATE_H
#define STEPFLOWSTATE_H

#include "FlowStep.h"

#include <QObject>
#include <QImage>
#include <QString>
#include <memory>
#include <vector>

/**
 * @brief 步骤流状态基类，管理一系列任务步骤的执行
 */
class StepFlowState : public QObject
{
    Q_OBJECT

public:
    explicit StepFlowState(QObject* parent = nullptr);
    ~StepFlowState() override = default;

    virtual QString name() const = 0;
    /**
     * @brief 更新状态，执行当前步骤逻辑
     * @param frame 当前帧图像
     * @return 下一个状态指针，如果返回nullptr表示流程完成
     */
    virtual StepFlowState* update(const QImage& frame);
    /**
     * @brief 检查状态是否失败
     * @return 如果状态失败返回true，否则返回false
     */
    virtual bool isFailed() const;
    /**
     * @brief 获取状态失败的原因
     * @return 失败原因字符串
     */
    virtual QString failureReason() const;
    /**
     * @brief 获取状态的运行时消息
     * @return 运行时消息字符串
     */
    virtual QString takeRuntimeMessage();
    /**
     * @brief 检查状态是否使用外部超时
     * @return 如果使用外部超时返回true，否则返回false
     */
    virtual bool usesExternalTimeout() const;

protected:
    /**
     * @brief 添加任务步骤
     * @param step 任务步骤指针
     */
    void addStep(std::unique_ptr<FlowStep> step);
    /**
     * @brief 设置运行时消息
     * @param message 运行时消息字符串
     */
    void setRuntimeMessage(const QString& message);
    /**
     * @brief 流程完成后的回调，返回下一个状态指针
     * @return 下一个状态指针，如果返回nullptr表示流程完成
     */
    virtual StepFlowState* onFlowFinished() = 0;

private:
    std::vector<std::unique_ptr<FlowStep>> m_steps;
    int m_currentStepIndex = 0;
    bool m_failed = false;
    QString m_failureReason;
    QString m_runtimeMessage;
};

#endif