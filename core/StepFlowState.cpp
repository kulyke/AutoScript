#include "StepFlowState.h"

#include <QDebug>

StepFlowState::StepFlowState(QObject* parent)
    : TaskState(parent)
{
}

TaskState* StepFlowState::update(const QImage& frame)
{
    if (m_failed) {
        return this;
    }

    while (m_currentStepIndex < static_cast<int>(m_steps.size())) {
        FlowStep* step = m_steps[m_currentStepIndex].get();
        const FlowStepStatus status = step->execute(frame);

        if (status == FlowStepStatus::Done) {
            qDebug() << "Step finished:" << step->name();
            ++m_currentStepIndex;
            continue;
        }

        if (status == FlowStepStatus::Failed) {
            m_failed = true;
            m_failureReason = step->errorString();
            qDebug() << "Step failed:" << step->name() << m_failureReason;
        }
        return this;
    }

    return onFlowFinished();
}

bool StepFlowState::isFailed() const
{
    return m_failed;
}

QString StepFlowState::failureReason() const
{
    return m_failureReason;
}

bool StepFlowState::usesExternalTimeout() const
{
    return false;
}

void StepFlowState::addStep(std::unique_ptr<FlowStep> step)
{
    m_steps.push_back(std::move(step));
}