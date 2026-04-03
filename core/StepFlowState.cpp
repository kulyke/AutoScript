#include "StepFlowState.h"

#include <QDebug>

StepFlowState::StepFlowState(QObject* parent)
    : QObject(parent)
{
}

StepFlowState* StepFlowState::update(const QImage& frame)
{
    if (m_failed) {
        return this;
    }

    while (m_currentStepIndex < static_cast<int>(m_steps.size())) {
        FlowStep* step = m_steps[m_currentStepIndex].get();
        const FlowStepStatus status = step->execute(frame);
        const QString stepMessage = step->takeRuntimeMessage();
        if (!stepMessage.isEmpty()) {
            m_runtimeMessage = QString("[%1] %2").arg(name(), stepMessage);
        }

        if (status == FlowStepStatus::Done) {
            if (m_runtimeMessage.isEmpty()) {
                m_runtimeMessage = QString("[%1] step finished: %2")
                    .arg(name(), step->name());
            }
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

QString StepFlowState::takeRuntimeMessage()
{
    const QString message = m_runtimeMessage;
    m_runtimeMessage.clear();
    return message;
}

bool StepFlowState::usesExternalTimeout() const
{
    return false;
}

void StepFlowState::addStep(std::unique_ptr<FlowStep> step)
{
    m_steps.push_back(std::move(step));
}

void StepFlowState::setRuntimeMessage(const QString& message)
{
    m_runtimeMessage = message;
}