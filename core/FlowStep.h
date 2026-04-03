#ifndef FLOWSTEP_H
#define FLOWSTEP_H

#include <QImage>
#include <QString>

enum class FlowStepStatus {
    Running,
    Done,
    Failed,
};

class FlowStep
{
public:
    virtual ~FlowStep() = default;

    virtual QString name() const = 0;
    virtual FlowStepStatus execute(const QImage& frame) = 0;
    virtual void reset() {}
    virtual QString takeRuntimeMessage() { return QString(); }
    virtual QString errorString() const { return QString(); }
};

#endif