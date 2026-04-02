#ifndef STEPFLOWSTATE_H
#define STEPFLOWSTATE_H

#include "TaskState.h"
#include "FlowStep.h"

#include <memory>
#include <vector>

class StepFlowState : public TaskState
{
public:
    explicit StepFlowState(QObject* parent = nullptr);
    ~StepFlowState() override = default;

    TaskState* update(const QImage& frame) override;
    bool isFailed() const override;
    QString failureReason() const override;
    bool usesExternalTimeout() const override;

protected:
    void addStep(std::unique_ptr<FlowStep> step);
    virtual TaskState* onFlowFinished() = 0;

private:
    std::vector<std::unique_ptr<FlowStep>> m_steps;
    int m_currentStepIndex = 0;
    bool m_failed = false;
    QString m_failureReason;
};

#endif