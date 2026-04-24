#ifndef TEMPLATESTEPS_H
#define TEMPLATESTEPS_H

#include "../../core/FlowStep.h"

#include <memory>
#include <QElapsedTimer>
#include <QPoint>

class VisionEngine;
class DeviceController;

class WaitTemplateStep : public FlowStep
{
public:
    WaitTemplateStep(VisionEngine* vision,
                     QString templateRef,
                     double threshold,
                     QString stepName);

    QString name() const override;
    FlowStepStatus execute(const QImage& frame) override;
    QString errorString() const override;

private:
    VisionEngine* m_vision;
    QString m_templateRef;
    double m_threshold;
    QString m_name;
    QString m_error;
};

class ClickTemplateStep : public FlowStep
{
public:
    ClickTemplateStep(VisionEngine* vision,
                      DeviceController* device,
                      QString templateRef,
                      double threshold,
                      QString stepName);

    QString name() const override;
    FlowStepStatus execute(const QImage& frame) override;
    QString errorString() const override;

private:
    VisionEngine* m_vision;
    DeviceController* m_device;
    QString m_templateRef;
    double m_threshold;
    QString m_name;
    QString m_error;
};

class TapPointStep : public FlowStep
{
public:
    TapPointStep(DeviceController* device,
                 QPoint point,
                 QString stepName);

    QString name() const override;
    FlowStepStatus execute(const QImage& frame) override;
    QString errorString() const override;

private:
    DeviceController* m_device;
    QPoint m_point;
    QString m_name;
    QString m_error;
};

class SwipeStep : public FlowStep
{
public:
    SwipeStep(DeviceController* device,
              QPoint startPoint,
              QPoint endPoint,
              int durationMs,
              QString stepName);

    QString name() const override;
    FlowStepStatus execute(const QImage& frame) override;
    QString errorString() const override;

private:
    DeviceController* m_device;
    QPoint m_startPoint;
    QPoint m_endPoint;
    int m_durationMs;
    QString m_name;
    QString m_error;
};

class KeyEventStep : public FlowStep
{
public:
    KeyEventStep(DeviceController* device,
                 int keyCode,
                 QString stepName);

    QString name() const override;
    FlowStepStatus execute(const QImage& frame) override;
    QString errorString() const override;

private:
    DeviceController* m_device;
    int m_keyCode;
    QString m_name;
    QString m_error;
};

class DelayFramesStep : public FlowStep
{
public:
    DelayFramesStep(int frameCount, QString stepName);

    QString name() const override;
    FlowStepStatus execute(const QImage& frame) override;
    void reset() override;

private:
    int m_frameCount;
    int m_currentFrame = 0;
    QString m_name;
};

class DelayMillisecondsStep : public FlowStep
{
public:
    DelayMillisecondsStep(int durationMs, QString stepName);

    QString name() const override;
    FlowStepStatus execute(const QImage& frame) override;
    void reset() override;

private:
    int m_durationMs;
    QString m_name;
    QElapsedTimer m_elapsedTimer;
    bool m_started = false;
};

class TimeoutStep : public FlowStep
{
public:
    TimeoutStep(std::unique_ptr<FlowStep> innerStep,
                int maxFrames,
                QString stepName = QString());

    QString name() const override;
    FlowStepStatus execute(const QImage& frame) override;
    void reset() override;
    QString takeRuntimeMessage() override;
    QString errorString() const override;

private:
    std::unique_ptr<FlowStep> m_innerStep;
    int m_maxFrames;
    int m_elapsedFrames = 0;
    QString m_name;
    QString m_error;
    QString m_runtimeMessage;
};

class TimeoutMillisecondsStep : public FlowStep
{
public:
    TimeoutMillisecondsStep(std::unique_ptr<FlowStep> innerStep,
                            int maxDurationMs,
                            QString stepName = QString());

    QString name() const override;
    FlowStepStatus execute(const QImage& frame) override;
    void reset() override;
    QString takeRuntimeMessage() override;
    QString errorString() const override;

private:
    std::unique_ptr<FlowStep> m_innerStep;
    int m_maxDurationMs;
    QString m_name;
    QString m_error;
    QString m_runtimeMessage;
    QElapsedTimer m_elapsedTimer;
    bool m_started = false;
};

class RetryStep : public FlowStep
{
public:
    RetryStep(std::unique_ptr<FlowStep> innerStep,
              int maxRetries,
              QString stepName = QString());

    QString name() const override;
    FlowStepStatus execute(const QImage& frame) override;
    void reset() override;
    QString takeRuntimeMessage() override;
    QString errorString() const override;

private:
    std::unique_ptr<FlowStep> m_innerStep;
    int m_maxRetries;
    int m_currentRetry = 0;
    QString m_name;
    QString m_error;
    QString m_runtimeMessage;
};

#endif