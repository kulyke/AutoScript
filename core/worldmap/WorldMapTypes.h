#ifndef WORLDMAPTYPES_H
#define WORLDMAPTYPES_H

#include <QPoint>
#include <QPointF>
#include <QRect>
#include <QSize>
#include <QSizeF>
#include <QString>
#include <QStringList>
#include <QTransform>
#include <QVector>

enum class WorldZoneType {
    Dangerous,
    Safe,
    Obscure,
    Abyssal,
    Stronghold,
    Archive
};

enum class WorldMapPage {
    Unknown,
    WorldMap
};

struct WorldZoneMetadata
{
    int zoneId = -1;
    QString key;
    QString displayName;
    QStringList aliases;

    QPointF worldAnchor;
    QPointF missionAnchor;

    int regionId = 0;
    int hazardLevel = 0;
    bool isPort = false;
    bool isAzurPort = false;

    QVector<WorldZoneType> defaultTypes;
    QVector<int> neighborZoneIds;
};

struct WorldMapGotoRequest
{
    int targetZoneId = -1;
    QVector<WorldZoneType> preferredTypes;
    bool refreshCurrentZone = false;
    bool stopIfAlreadySafe = false;
    bool allowNeighborUnlockFallback = true;
};

struct WorldMapCalibration
{
    QSize screenSize = QSize(1280, 720);
    QRect viewport = QRect(QPoint(0, 0), screenSize);
    QSizeF referenceMapSize = QSizeF(2570.0, 1696.0);
    QPointF fixedScreenCenter = QPointF(508.0, 423.0);
};

struct WorldMapTransformSnapshot
{
    bool valid = false;
    double confidence = 0.0;

    QRect viewport;
    QSizeF referenceMapSize;

    QPointF worldCenter;
    QPointF screenCenter;
    WorldMapPage page = WorldMapPage::Unknown;

    QTransform worldToScreen;
    QTransform screenToWorld;
};

#endif