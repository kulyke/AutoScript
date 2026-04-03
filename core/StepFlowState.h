#ifndef STEPFLOWSTATE_H
#define STEPFLOWSTATE_H

#include "FlowStep.h"

#include <QObject>
#include <QImage>
#include <QString>
#include <memory>
#include <vector>

class StepFlowState : public QObject
{
    Q_OBJECT

public:
    explicit StepFlowState(QObject* parent = nullptr);
    ~StepFlowState() override = default;

    virtual QString name() const = 0;
    virtual StepFlowState* update(const QImage& frame);
    virtual bool isFailed() const;
    virtual QString failureReason() const;
    virtual QString takeRuntimeMessage();
    virtual bool usesExternalTimeout() const;

protected:
    void addStep(std::unique_ptr<FlowStep> step);
    void setRuntimeMessage(const QString& message);
    virtual StepFlowState* onFlowFinished() = 0;

private:
    std::vector<std::unique_ptr<FlowStep>> m_steps;
    int m_currentStepIndex = 0;
    bool m_failed = false;
    QString m_failureReason;
    QString m_runtimeMessage;
};

#endif