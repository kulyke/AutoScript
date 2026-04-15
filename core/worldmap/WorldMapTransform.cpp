#include "WorldMapTransform.h"

#include "VisionEngine.h"

#include <algorithm>

namespace {

QPointF clampVector(const QPointF& value, const QSizeF& limit)
{
    return QPointF(
        std::clamp(value.x(), -limit.width(), limit.width()),
        std::clamp(value.y(), -limit.height(), limit.height())
    );
}

}

WorldMapTransform::WorldMapTransform(VisionEngine* vision)
    : m_vision(vision)
{
}

void WorldMapTransform::setVisionEngine(VisionEngine* vision)
{
    m_vision = vision;
}

void WorldMapTransform::setCalibration(const WorldMapCalibration& calibration)
{
    m_calibration = calibration;
    if (m_hasWorldCenter) {
        rebuildSnapshot(m_worldCenter, m_snapshot.page, nullptr);
    }
}

WorldMapCalibration WorldMapTransform::calibration() const
{
    return m_calibration;
}

bool WorldMapTransform::hasWorldCenter() const
{
    return m_hasWorldCenter;
}

QPointF WorldMapTransform::worldCenter() const
{
    return m_worldCenter;
}

void WorldMapTransform::setWorldCenter(const QPointF& worldCenter)
{
    m_worldCenter = worldCenter;
    m_hasWorldCenter = true;
    rebuildSnapshot(worldCenter, m_snapshot.page, nullptr);
}

void WorldMapTransform::clearWorldCenter()
{
    m_worldCenter = QPointF();
    m_hasWorldCenter = false;
    m_snapshot = WorldMapTransformSnapshot();
}

bool WorldMapTransform::update(const QImage& frame, QString* error)
{
    if (frame.isNull()) {
        if (error) {
            *error = QString("World map frame is empty");
        }
        m_snapshot = WorldMapTransformSnapshot();
        return false;
    }

    const WorldMapPage page = detectPage(frame);
    if (page != WorldMapPage::WorldMap) {
        if (error) {
            *error = QString("Current frame is not recognized as world map");
        }
        m_snapshot = WorldMapTransformSnapshot();
        return false;
    }

    if (!m_hasWorldCenter) {
        if (error) {
            *error = QString("World center is not set");
        }
        m_snapshot = WorldMapTransformSnapshot();
        return false;
    }

    return rebuildSnapshot(m_worldCenter, page, error);
}

bool WorldMapTransform::updateFromKnownCenter(const QImage& frame,
                                              const QPointF& worldCenter,
                                              QString* error)
{
    m_worldCenter = worldCenter;
    m_hasWorldCenter = true;
    return update(frame, error);
}

bool WorldMapTransform::isValid() const
{
    return m_snapshot.valid;
}

WorldMapTransformSnapshot WorldMapTransform::snapshot() const
{
    return m_snapshot;
}

QPointF WorldMapTransform::worldToScreen(const QPointF& worldPoint) const
{
    if (!m_snapshot.valid) {
        return QPointF();
    }

    return m_snapshot.worldToScreen.map(worldPoint);
}

QPointF WorldMapTransform::screenToWorld(const QPointF& screenPoint) const
{
    if (!m_snapshot.valid) {
        return QPointF();
    }

    return m_snapshot.screenToWorld.map(screenPoint);
}

bool WorldMapTransform::isWorldPointVisible(const QPointF& worldPoint, const QRect& sightArea) const
{
    if (!m_snapshot.valid) {
        return false;
    }

    return sightArea.contains(worldToScreen(worldPoint).toPoint());
}

QPoint WorldMapTransform::computeTapPointForZone(const WorldZoneMetadata& zone) const
{
    return worldToScreen(zone.worldAnchor).toPoint();
}

QPointF WorldMapTransform::computeSwipeVectorToward(const QPointF& delta,
                                                    const QSizeF& swipeLimit) const
{
    if (!m_snapshot.valid) {
        return QPointF();
    }

    return clampVector(delta, swipeLimit);
}

WorldMapPage WorldMapTransform::detectPage(const QImage& frame) const
{
    if (!m_vision) {
        return WorldMapPage::Unknown;
    }

    QPoint matchPoint;
    if (m_vision->findTemplate(frame, "worldMap.title", matchPoint, -1.0)) {
        return WorldMapPage::WorldMap;
    }

    return WorldMapPage::Unknown;
}

bool WorldMapTransform::rebuildSnapshot(const QPointF& worldCenter,
                                        WorldMapPage page,
                                        QString* error)
{
    QTransform worldToScreenTransform;
    worldToScreenTransform.translate(
        m_calibration.fixedScreenCenter.x() - worldCenter.x(),
        m_calibration.fixedScreenCenter.y() - worldCenter.y());

    bool invertible = false;
    const QTransform screenToWorldTransform = worldToScreenTransform.inverted(&invertible);
    if (!invertible) {
        if (error) {
            *error = QString("World map transform is not invertible");
        }
        m_snapshot = WorldMapTransformSnapshot();
        return false;
    }

    m_snapshot.valid = true;
    m_snapshot.confidence = 1.0;
    m_snapshot.viewport = m_calibration.viewport;
    m_snapshot.referenceMapSize = m_calibration.referenceMapSize;
    m_snapshot.worldCenter = worldCenter;
    m_snapshot.screenCenter = m_calibration.fixedScreenCenter;
    m_snapshot.page = WorldMapPage::WorldMap;
    m_snapshot.worldToScreen = worldToScreenTransform;
    m_snapshot.screenToWorld = screenToWorldTransform;
    return true;
}