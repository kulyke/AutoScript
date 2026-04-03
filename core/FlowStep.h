#ifndef FLOWSTEP_H
#define FLOWSTEP_H

#include <QImage>
#include <QString>

enum class FlowStepStatus {
    Running,
    Done,
    Failed,
};

/**
 * @brief 任务步骤基类，定义了任务步骤的接口
 * 
 */
class FlowStep
{
public:
    virtual ~FlowStep() = default;

    virtual QString name() const = 0;
    /**
     * @brief 执行步骤逻辑
     * @param frame 当前帧图像
     * @return 步骤执行状态
     */
    virtual FlowStepStatus execute(const QImage& frame) = 0;
    /**
     * @brief 重置步骤状态
     */
    virtual void reset() {}
    /**
     * @brief 获取步骤的运行时消息
     * @return 运行时消息字符串
     */
    virtual QString takeRuntimeMessage() { return QString(); }
    /**
     * @brief 获取步骤的错误信息
     * @return 错误信息字符串
     */
    virtual QString errorString() const { return QString(); }
};

#endif