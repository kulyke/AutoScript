#ifndef WORLDMAPSTEPS_H
#define WORLDMAPSTEPS_H

#include "../../core/FlowStep.h"

#include <QElapsedTimer>
#include <QString>

class VisionEngine;
class DeviceController;
class WorldZoneCatalog;
class WorldMapTransform;
struct WorldMapRuntimeContext;

/**
 * @brief 初始化世界地图步骤
 * 该步骤负责初始化世界地图的相关上下文，包括加载区域目录、设置世界中心等操作。
 */
class InitializeWorldMapStep : public FlowStep
{
public:
    InitializeWorldMapStep(VisionEngine* vision,
                           DeviceController* device,
                           WorldZoneCatalog* zoneCatalog,
                           WorldMapTransform* transform,
                           QString stepName);

    QString name() const override;
    FlowStepStatus execute(const QImage& frame) override;
    QString takeRuntimeMessage() override;
    QString errorString() const override;

private:
    VisionEngine* m_vision;
    DeviceController* m_device;
    WorldZoneCatalog* m_zoneCatalog;
    WorldMapTransform* m_transform;
    QString m_name;
    QString m_error;
    QString m_runtimeMessage;
};

/**
 * @brief 解析当前所在区域步骤
 */
class ResolveCurrentWorldZoneStep : public FlowStep
{
public:
    ResolveCurrentWorldZoneStep(WorldZoneCatalog* zoneCatalog,
                                WorldMapTransform* transform,
                                WorldMapRuntimeContext* runtimeContext,
                                QString stepName);

    QString name() const override;
    FlowStepStatus execute(const QImage& frame) override;
    QString takeRuntimeMessage() override;
    QString errorString() const override;

private:
    WorldZoneCatalog* m_zoneCatalog;
    WorldMapTransform* m_transform;
    WorldMapRuntimeContext* m_runtimeContext;
    QString m_name;
    QString m_error;
    QString m_runtimeMessage;
};

/**
 *  @brief 聚焦目标区域步骤
 */
class FocusTargetWorldZoneStep : public FlowStep
{
public:
    FocusTargetWorldZoneStep(DeviceController* device,
                             WorldZoneCatalog* zoneCatalog,
                             WorldMapTransform* transform,
                             WorldMapRuntimeContext* runtimeContext,
                             QString stepName);

    QString name() const override;
    FlowStepStatus execute(const QImage& frame) override;
    void reset() override;
    QString takeRuntimeMessage() override;
    QString errorString() const override;

private:
    DeviceController* m_device;
    WorldZoneCatalog* m_zoneCatalog;
    WorldMapTransform* m_transform;
    WorldMapRuntimeContext* m_runtimeContext;
    QString m_name;
    QString m_error;
    QString m_runtimeMessage;
    QElapsedTimer m_settleTimer;
    bool m_waitingSettle = false;
    int m_swipeAttempts = 0;
};

/**
 * @brief 点击目标区域步骤
 */
class TapTargetWorldZoneStep : public FlowStep
{
public:
    TapTargetWorldZoneStep(DeviceController* device,
                           WorldZoneCatalog* zoneCatalog,
                           WorldMapTransform* transform,
                           WorldMapRuntimeContext* runtimeContext,
                           QString stepName);

    QString name() const override;
    FlowStepStatus execute(const QImage& frame) override;
    QString takeRuntimeMessage() override;
    QString errorString() const override;

private:
    DeviceController* m_device;
    WorldZoneCatalog* m_zoneCatalog;
    WorldMapTransform* m_transform;
    WorldMapRuntimeContext* m_runtimeContext;
    QString m_name;
    QString m_error;
    QString m_runtimeMessage;
};

/**
 * @brief 验证目标区域入口步骤
 */
class VerifyTargetWorldZoneEntryStep : public FlowStep
{
public:
    VerifyTargetWorldZoneEntryStep(VisionEngine* vision,
                                   WorldZoneCatalog* zoneCatalog,
                                   WorldMapRuntimeContext* runtimeContext,
                                   QString stepName);

    QString name() const override;
    FlowStepStatus execute(const QImage& frame) override;
    QString takeRuntimeMessage() override;
    QString errorString() const override;

private:
    VisionEngine* m_vision;
    WorldZoneCatalog* m_zoneCatalog;
    WorldMapRuntimeContext* m_runtimeContext;
    QString m_name;
    QString m_error;
    QString m_runtimeMessage;
};

#endif