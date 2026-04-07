#ifndef WORLDMAPTRANSFORM_H
#define WORLDMAPTRANSFORM_H

#include "WorldMapTypes.h"

#include <QImage>
#include <QPoint>

class VisionEngine;

class WorldMapTransform
{
public:
    explicit WorldMapTransform(VisionEngine* vision = nullptr);

    void setVisionEngine(VisionEngine* vision);
    void setCalibration(const WorldMapCalibration& calibration);

    WorldMapCalibration calibration() const;
    bool hasWorldCenter() const;
    QPointF worldCenter() const;
    void setWorldCenter(const QPointF& worldCenter);
    void clearWorldCenter();

    bool update(const QImage& frame, QString* error = nullptr);
    bool updateFromKnownCenter(const QImage& frame,
                               const QPointF& worldCenter,
                               QString* error = nullptr);

    bool isValid() const;
    WorldMapTransformSnapshot snapshot() const;

    QPointF worldToScreen(const QPointF& worldPoint) const;
    QPointF screenToWorld(const QPointF& screenPoint) const;

    bool isWorldPointVisible(const QPointF& worldPoint, const QRect& sightArea) const;
    QPoint computeTapPointForZone(const WorldZoneMetadata& zone) const;
    QPointF computeSwipeVectorToward(const QPointF& worldPoint,
                                     const QSizeF& swipeLimit) const;

private:
    WorldMapPage detectPage(const QImage& frame) const;
    bool rebuildSnapshot(const QPointF& worldCenter,
                         WorldMapPage page,
                         QString* error = nullptr);

    VisionEngine* m_vision = nullptr;
    WorldMapCalibration m_calibration;
    WorldMapTransformSnapshot m_snapshot;
    QPointF m_worldCenter;
    bool m_hasWorldCenter = false;
};

#endif