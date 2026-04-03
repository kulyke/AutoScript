#include "TemplateSteps.h"

#include "visionengine.h"
#include "devicecontroller.h"

#include <utility>

WaitTemplateStep::WaitTemplateStep(VisionEngine* vision,
                                   QString templateRef,
                                   double threshold,
                                   QString stepName)
    : m_vision(vision)
    , m_templateRef(std::move(templateRef))
    , m_threshold(threshold)
    , m_name(std::move(stepName))
{
}

QString WaitTemplateStep::name() const
{
    return m_name;
}

FlowStepStatus WaitTemplateStep::execute(const QImage& frame)
{
    m_error.clear();

    if (!m_vision) {
        m_error = "vision is null";
        return FlowStepStatus::Failed;
    }

    QPoint point;
    if (m_vision->findTemplate(frame, m_templateRef, point, m_threshold)) {
        return FlowStepStatus::Done;
    }
    return FlowStepStatus::Running;
}

QString WaitTemplateStep::errorString() const
{
    return m_error;
}

ClickTemplateStep::ClickTemplateStep(VisionEngine* vision,
                                     DeviceController* device,
                                     QString templateRef,
                                     double threshold,
                                     QString stepName)
    : m_vision(vision)
    , m_device(device)
    , m_templateRef(std::move(templateRef))
    , m_threshold(threshold)
    , m_name(std::move(stepName))
{
}

QString ClickTemplateStep::name() const
{
    return m_name;
}

FlowStepStatus ClickTemplateStep::execute(const QImage& frame)
{
    m_error.clear();

    if (!m_vision) {
        m_error = "vision is null";
        return FlowStepStatus::Failed;
    }
    if (!m_device) {
        m_error = "device is null";
        return FlowStepStatus::Failed;
    }

    QPoint point;
    if (!m_vision->findTemplate(frame, m_templateRef, point, m_threshold)) {
        return FlowStepStatus::Running;
    }

    if (!m_device->tap(point.x(), point.y())) {
        m_error = QString("tap (%1,%2) failed for template '%3'")
            .arg(point.x())
            .arg(point.y())
            .arg(m_templateRef);
        return FlowStepStatus::Failed;
    }

    return FlowStepStatus::Done;
}

QString ClickTemplateStep::errorString() const
{
    return m_error;
}

TapPointStep::TapPointStep(DeviceController* device,
                           QPoint point,
                           QString stepName)
    : m_device(device)
    , m_point(point)
    , m_name(std::move(stepName))
{
}

QString TapPointStep::name() const
{
    return m_name;
}

FlowStepStatus TapPointStep::execute(const QImage& frame)
{
    Q_UNUSED(frame);

    m_error.clear();

    if (!m_device) {
        m_error = "device is null";
        return FlowStepStatus::Failed;
    }

    if (!m_device->tap(m_point.x(), m_point.y())) {
        m_error = QString("tap (%1,%2) failed").arg(m_point.x()).arg(m_point.y());
        return FlowStepStatus::Failed;
    }

    return FlowStepStatus::Done;
}

QString TapPointStep::errorString() const
{
    return m_error;
}

SwipeStep::SwipeStep(DeviceController* device,
                     QPoint startPoint,
                     QPoint endPoint,
                     int durationMs,
                     QString stepName)
    : m_device(device)
    , m_startPoint(startPoint)
    , m_endPoint(endPoint)
    , m_durationMs(durationMs)
    , m_name(std::move(stepName))
{
}

QString SwipeStep::name() const
{
    return m_name;
}

FlowStepStatus SwipeStep::execute(const QImage& frame)
{
    Q_UNUSED(frame);

    m_error.clear();

    if (!m_device) {
        m_error = "device is null";
        return FlowStepStatus::Failed;
    }

    if (!m_device->swipe(m_startPoint.x(),
                         m_startPoint.y(),
                         m_endPoint.x(),
                         m_endPoint.y(),
                         m_durationMs)) {
        m_error = QString("swipe (%1,%2)->(%3,%4) failed")
            .arg(m_startPoint.x())
            .arg(m_startPoint.y())
            .arg(m_endPoint.x())
            .arg(m_endPoint.y());
        return FlowStepStatus::Failed;
    }

    return FlowStepStatus::Done;
}

QString SwipeStep::errorString() const
{
    return m_error;
}

KeyEventStep::KeyEventStep(DeviceController* device,
                           int keyCode,
                           QString stepName)
    : m_device(device)
    , m_keyCode(keyCode)
    , m_name(std::move(stepName))
{
}

QString KeyEventStep::name() const
{
    return m_name;
}

FlowStepStatus KeyEventStep::execute(const QImage& frame)
{
    Q_UNUSED(frame);

    m_error.clear();

    if (!m_device) {
        m_error = "device is null";
        return FlowStepStatus::Failed;
    }

    if (!m_device->keyEvent(m_keyCode)) {
        m_error = QString("keyevent %1 failed").arg(m_keyCode);
        return FlowStepStatus::Failed;
    }

    return FlowStepStatus::Done;
}

QString KeyEventStep::errorString() const
{
    return m_error;
}

DelayFramesStep::DelayFramesStep(int frameCount, QString stepName)
    : m_frameCount(frameCount)
    , m_name(std::move(stepName))
{
}

QString DelayFramesStep::name() const
{
    return m_name;
}

void DelayFramesStep::reset()
{
    m_currentFrame = 0;
}

FlowStepStatus DelayFramesStep::execute(const QImage& frame)
{
    Q_UNUSED(frame);

    if (m_frameCount <= 0) {
        return FlowStepStatus::Done;
    }

    ++m_currentFrame;
    if (m_currentFrame >= m_frameCount) {
        return FlowStepStatus::Done;
    }
    return FlowStepStatus::Running;
}

TimeoutStep::TimeoutStep(std::unique_ptr<FlowStep> innerStep,
                         int maxFrames,
                         QString stepName)
    : m_innerStep(std::move(innerStep))
    , m_maxFrames(maxFrames)
    , m_name(std::move(stepName))
{
}

QString TimeoutStep::name() const
{
    if (!m_name.isEmpty()) {
        return m_name;
    }
    return m_innerStep ? m_innerStep->name() : QString("TimeoutStep");
}

void TimeoutStep::reset()
{
    m_elapsedFrames = 0;
    m_error.clear();
    m_runtimeMessage.clear();
    if (m_innerStep) {
        m_innerStep->reset();
    }
}

QString TimeoutStep::takeRuntimeMessage()
{
    if (!m_runtimeMessage.isEmpty()) {
        const QString message = m_runtimeMessage;
        m_runtimeMessage.clear();
        return message;
    }

    return m_innerStep ? m_innerStep->takeRuntimeMessage() : QString();
}

FlowStepStatus TimeoutStep::execute(const QImage& frame)
{
    if (!m_innerStep) {
        m_error = "inner step is null";
        return FlowStepStatus::Failed;
    }

    const FlowStepStatus status = m_innerStep->execute(frame);
    if (status == FlowStepStatus::Done) {
        return FlowStepStatus::Done;
    }
    if (status == FlowStepStatus::Failed) {
        m_error = m_innerStep->errorString();
        if (m_error.isEmpty()) {
            m_error = QString("step '%1' failed").arg(m_innerStep->name());
        }
        return FlowStepStatus::Failed;
    }

    ++m_elapsedFrames;
    if (m_maxFrames > 0 && m_elapsedFrames >= m_maxFrames) {
        m_error = QString("step '%1' timed out after %2 frames")
            .arg(m_innerStep->name())
            .arg(m_maxFrames);
        return FlowStepStatus::Failed;
    }
    return FlowStepStatus::Running;
}

QString TimeoutStep::errorString() const
{
    return m_error;
}

RetryStep::RetryStep(std::unique_ptr<FlowStep> innerStep,
                     int maxRetries,
                     QString stepName)
    : m_innerStep(std::move(innerStep))
    , m_maxRetries(maxRetries)
    , m_name(std::move(stepName))
{
}

QString RetryStep::name() const
{
    if (!m_name.isEmpty()) {
        return m_name;
    }
    return m_innerStep ? m_innerStep->name() : QString("RetryStep");
}

FlowStepStatus RetryStep::execute(const QImage& frame)
{
    m_error.clear();
    m_runtimeMessage.clear();

    if (!m_innerStep) {
        m_error = "inner step is null";
        return FlowStepStatus::Failed;
    }

    const FlowStepStatus status = m_innerStep->execute(frame);
    if (status != FlowStepStatus::Failed) {
        return status;
    }

    ++m_currentRetry;
    if (m_currentRetry > m_maxRetries) {
        const QString innerError = m_innerStep->errorString();
        m_error = innerError.isEmpty()
            ? QString("step '%1' failed after %2 retries")
                  .arg(m_innerStep->name())
                  .arg(m_maxRetries)
            : QString("step '%1' failed after %2 retries: %3")
                  .arg(m_innerStep->name())
                  .arg(m_maxRetries)
                  .arg(innerError);
        return FlowStepStatus::Failed;
    }

    const QString retryReason = m_innerStep->errorString();
    m_runtimeMessage = retryReason.isEmpty()
        ? QString("retry %1/%2 for step '%3'")
              .arg(m_currentRetry)
              .arg(m_maxRetries)
              .arg(m_innerStep->name())
        : QString("retry %1/%2 for step '%3': %4")
              .arg(m_currentRetry)
              .arg(m_maxRetries)
              .arg(m_innerStep->name())
              .arg(retryReason);

    m_innerStep->reset();
    return FlowStepStatus::Running;
}

void RetryStep::reset()
{
    m_currentRetry = 0;
    m_error.clear();
    m_runtimeMessage.clear();
    if (m_innerStep) {
        m_innerStep->reset();
    }
}

QString RetryStep::takeRuntimeMessage()
{
    if (!m_runtimeMessage.isEmpty()) {
        const QString message = m_runtimeMessage;
        m_runtimeMessage.clear();
        return message;
    }

    return m_innerStep ? m_innerStep->takeRuntimeMessage() : QString();
}

QString RetryStep::errorString() const
{
    return m_error;
}