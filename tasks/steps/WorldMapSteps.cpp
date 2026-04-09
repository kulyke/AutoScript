#include "WorldMapSteps.h"

#include "devicecontroller.h"
#include "visionengine.h"
#include "../../core/worldmap/WorldMapTransform.h"
#include "../../core/worldmap/WorldMapRuntimeContext.h"
#include "../../core/worldmap/WorldZoneCatalog.h"

#include <QCoreApplication>
#include <QDir>

#include <cmath>

namespace {

constexpr int kWorldMapFocusSettleFrames = 3; //焦点设定后需要等待的帧数，以确保界面稳定下来，避免过早地进行下一步操作导致误触或界面未更新的问题
constexpr int kWorldMapFocusMaxSwipes = 4; //最大连续滑动次数限制，防止在某些情况下（如解析错误或界面异常）导致过度滑动，增加误触风险。这个值可以根据实际情况调整，通常设置为3-5次比较合理。
constexpr int kWorldMapFocusSwipeDurationMs = 350; //每次滑动的持续时间，过短可能导致滑动不被识别，过长则可能增加操作时间。350ms是一个相对适中的值，可以根据设备和游戏的响应情况进行调整。
constexpr double kWorldMapFocusTolerancePx = 24.0; //焦点容忍度（像素），表示当目标点与当前点的屏幕坐标差距在这个范围内时，认为已经成功对焦，无需继续滑动。这个值需要根据游戏界面的设计和实际测试进行调整，通常设置在20-30像素之间比较合理。
const QSizeF kWorldMapFocusSwipeLimit(280.0, 180.0); //焦点滑动限制（像素），表示每次滑动的最大屏幕坐标偏移量，过大可能导致滑动过度，过小则可能需要多次滑动才能完成对焦。这个值需要根据游戏界面的设计和实际测试进行调整，通常设置在200-300像素之间比较合理。

/**
 * @brief 判断给定的坐标偏移是否在焦点容忍范围内
 */
bool isWithinFocusTolerance(const QPointF& delta)
{
    return std::abs(delta.x()) <= kWorldMapFocusTolerancePx
        && std::abs(delta.y()) <= kWorldMapFocusTolerancePx;
}

}

InitializeWorldMapStep::InitializeWorldMapStep(VisionEngine* vision,
                                               WorldZoneCatalog* zoneCatalog,
                                               WorldMapTransform* transform,
                                               QString stepName)
    : m_vision(vision)
    , m_zoneCatalog(zoneCatalog)
    , m_transform(transform)
    , m_name(std::move(stepName))
{
}

QString InitializeWorldMapStep::name() const
{
    return m_name;
}

FlowStepStatus InitializeWorldMapStep::execute(const QImage& frame)
{
    m_error.clear();
    m_runtimeMessage.clear();

    if (!m_vision) {
        m_error = "vision is null";
        return FlowStepStatus::Failed;
    }
    if (!m_zoneCatalog) {
        m_error = "world zone catalog is null";
        return FlowStepStatus::Failed;
    }
    if (!m_transform) {
        m_error = "world map transform is null";
        return FlowStepStatus::Failed;
    }

    if (m_zoneCatalog->isEmpty()) {
        const QString zoneCatalogPath = QDir(QCoreApplication::applicationDirPath())
            .filePath("resources/world_map/zones.json");
        QString loadError;
        if (!m_zoneCatalog->loadFromJson(zoneCatalogPath, &loadError)) {
            m_error = loadError;
            return FlowStepStatus::Failed;
        }
    }

    if (!m_transform->hasWorldCenter()) {
        const WorldMapCalibration calibration = m_transform->calibration();
        m_transform->setWorldCenter(QPointF(
            calibration.referenceMapSize.width() / 2.0,
            calibration.referenceMapSize.height() / 2.0));
    }

    QString updateError;
    if (!m_transform->update(frame, &updateError)) {
        m_error = updateError;
        return FlowStepStatus::Failed;
    }

    const WorldMapTransformSnapshot snapshot = m_transform->snapshot();
    m_runtimeMessage = QString("world map initialized: center=(%1,%2) screenAnchor=(%3,%4)")
        .arg(snapshot.worldCenter.x(), 0, 'f', 1)
        .arg(snapshot.worldCenter.y(), 0, 'f', 1)
        .arg(snapshot.screenCenter.x(), 0, 'f', 1)
        .arg(snapshot.screenCenter.y(), 0, 'f', 1);
    return FlowStepStatus::Done;
}

QString InitializeWorldMapStep::takeRuntimeMessage()
{
    const QString message = m_runtimeMessage;
    m_runtimeMessage.clear();
    return message;
}

QString InitializeWorldMapStep::errorString() const
{
    return m_error;
}

ResolveCurrentWorldZoneStep::ResolveCurrentWorldZoneStep(WorldZoneCatalog* zoneCatalog,
                                                         WorldMapTransform* transform,
                                                         WorldMapRuntimeContext* runtimeContext,
                                                         QString stepName)
    : m_zoneCatalog(zoneCatalog)
    , m_transform(transform)
    , m_runtimeContext(runtimeContext)
    , m_name(std::move(stepName))
{
}

QString ResolveCurrentWorldZoneStep::name() const
{
    return m_name;
}

FlowStepStatus ResolveCurrentWorldZoneStep::execute(const QImage& frame)
{
    Q_UNUSED(frame);

    m_error.clear();
    m_runtimeMessage.clear();

    if (!m_zoneCatalog) {
        m_error = "world zone catalog is null";
        return FlowStepStatus::Failed;
    }
    if (!m_transform) {
        m_error = "world map transform is null";
        return FlowStepStatus::Failed;
    }
    if (!m_runtimeContext) {
        m_error = "world map runtime context is null";
        return FlowStepStatus::Failed;
    }
    if (!m_transform->isValid()) {
        m_error = "world map transform is invalid";
        return FlowStepStatus::Failed;
    }

    // const QPointF worldCenter = m_transform->worldCenter();
    const QPointF currentWorldZonePoint = QPoint(260.0, 560.0); //测试用：直接使用一个固定的世界坐标点来解析当前区域，避免实际解析过程中可能遇到的各种问题
    const WorldZoneMetadata* zone = m_zoneCatalog->nearestToWorldPoint(currentWorldZonePoint);
    if (!zone) {
        m_error = "failed to resolve current world zone from world center";
        return FlowStepStatus::Failed;
    }

    m_runtimeContext->currentZoneId = zone->zoneId;
    m_runtimeContext->currentZoneName = zone->displayName;
    m_runtimeContext->lastResolutionSource = "nearest-world-center";

    m_runtimeMessage = QString("current zone resolved: id=%1 name=%2 source=%3")
        .arg(zone->zoneId)
        .arg(zone->displayName)
        .arg(m_runtimeContext->lastResolutionSource);
    return FlowStepStatus::Done;
}

QString ResolveCurrentWorldZoneStep::takeRuntimeMessage()
{
    const QString message = m_runtimeMessage;
    m_runtimeMessage.clear();
    return message;
}

QString ResolveCurrentWorldZoneStep::errorString() const
{
    return m_error;
}

FocusTargetWorldZoneStep::FocusTargetWorldZoneStep(DeviceController* device,
                                                   WorldZoneCatalog* zoneCatalog,
                                                   WorldMapTransform* transform,
                                                   WorldMapRuntimeContext* runtimeContext,
                                                   QString stepName)
    : m_device(device)
    , m_zoneCatalog(zoneCatalog)
    , m_transform(transform)
    , m_runtimeContext(runtimeContext)
    , m_name(std::move(stepName))
{
}

QString FocusTargetWorldZoneStep::name() const
{
    return m_name;
}

FlowStepStatus FocusTargetWorldZoneStep::execute(const QImage& frame)
{
    Q_UNUSED(frame);

    m_error.clear();
    m_runtimeMessage.clear();

    if (!m_device) {
        m_error = "device is null";
        return FlowStepStatus::Failed;
    }
    if (!m_zoneCatalog) {
        m_error = "world zone catalog is null";
        return FlowStepStatus::Failed;
    }
    if (!m_transform) {
        m_error = "world map transform is null";
        return FlowStepStatus::Failed;
    }
    if (!m_runtimeContext) {
        m_error = "world map runtime context is null";
        return FlowStepStatus::Failed;
    }
    if (!m_transform->isValid()) {
        m_error = "world map transform is invalid";
        return FlowStepStatus::Failed;
    }

    if (m_waitFrames > 0) {
        --m_waitFrames;
        return FlowStepStatus::Running;
    }

    if (!m_runtimeContext->hasTargetZoneRequest()) {
        m_runtimeContext->clearFocusedZone();
        m_runtimeContext->lastNavigationAction = "no-target-request";
        m_runtimeMessage = "no target zone request; skip world-map focus";
        return FlowStepStatus::Done;
    }

    const WorldZoneMetadata* targetZone =
        m_zoneCatalog->findById(m_runtimeContext->gotoRequest.targetZoneId);
    if (!targetZone) {
        m_error = QString("target zone not found: id=%1")
            .arg(m_runtimeContext->gotoRequest.targetZoneId);
        return FlowStepStatus::Failed;
    }

    const WorldMapTransformSnapshot snapshot = m_transform->snapshot();
    const QPointF targetScreenPoint = m_transform->worldToScreen(targetZone->worldAnchor);
    const QPointF delta = targetScreenPoint - snapshot.screenCenter;
    if (isWithinFocusTolerance(delta)) {
        m_runtimeContext->focusedZoneId = targetZone->zoneId;
        m_runtimeContext->focusedZoneName = targetZone->displayName;
        m_runtimeContext->lastNavigationAction = m_swipeAttempts > 0
            ? "target-centered-after-swipe"
            : "target-already-centered";
        m_runtimeMessage = QString("target zone centered: id=%1 name=%2")
            .arg(targetZone->zoneId)
            .arg(targetZone->displayName);
        return FlowStepStatus::Done;
    }

    if (m_swipeAttempts >= kWorldMapFocusMaxSwipes) {
        m_error = QString("target zone %1 was not centered after %2 swipes")
            .arg(targetZone->zoneId)
            .arg(kWorldMapFocusMaxSwipes);
        return FlowStepStatus::Failed;
    }

    const QPointF worldOffset =
        m_transform->computeSwipeVectorToward(targetZone->worldAnchor, kWorldMapFocusSwipeLimit);
    if (std::abs(worldOffset.x()) < 1.0 && std::abs(worldOffset.y()) < 1.0) {
        m_error = QString("computed zero focus offset for target zone %1")
            .arg(targetZone->zoneId);
        return FlowStepStatus::Failed;
    }

    const QPointF gestureVector = -worldOffset;
    const QPoint startPoint = snapshot.screenCenter.toPoint();
    const QPoint endPoint = (snapshot.screenCenter + gestureVector).toPoint();

    if (!m_device->swipe(startPoint.x(),
                         startPoint.y(),
                         endPoint.x(),
                         endPoint.y(),
                         kWorldMapFocusSwipeDurationMs)) {
        m_error = QString("swipe (%1,%2)->(%3,%4) failed while focusing target zone %5")
            .arg(startPoint.x())
            .arg(startPoint.y())
            .arg(endPoint.x())
            .arg(endPoint.y())
            .arg(targetZone->zoneId);
        return FlowStepStatus::Failed;
    }

    ++m_swipeAttempts;
    m_waitFrames = kWorldMapFocusSettleFrames;
    m_transform->setWorldCenter(m_transform->worldCenter() + worldOffset);

    m_runtimeContext->focusedZoneId = targetZone->zoneId;
    m_runtimeContext->focusedZoneName = targetZone->displayName;
    m_runtimeContext->lastNavigationAction = QString("focus-swipe-%1").arg(m_swipeAttempts);
    m_runtimeMessage = QString("focus target zone: id=%1 name=%2 swipe=(%3,%4)")
        .arg(targetZone->zoneId)
        .arg(targetZone->displayName)
        .arg(gestureVector.x(), 0, 'f', 1)
        .arg(gestureVector.y(), 0, 'f', 1);
    return FlowStepStatus::Running;
}

void FocusTargetWorldZoneStep::reset()
{
    m_error.clear();
    m_runtimeMessage.clear();
    m_waitFrames = 0;
    m_swipeAttempts = 0;
}

QString FocusTargetWorldZoneStep::takeRuntimeMessage()
{
    const QString message = m_runtimeMessage;
    m_runtimeMessage.clear();
    return message;
}

QString FocusTargetWorldZoneStep::errorString() const
{
    return m_error;
}