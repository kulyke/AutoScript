#ifndef WORLDZONECATALOG_H
#define WORLDZONECATALOG_H

#include "WorldMapTypes.h"

#include <QHash>
#include <QVector>

#include <optional>

class WorldZoneCatalog
{
public:
    bool loadFromJson(const QString& path, QString* error = nullptr);

    const WorldZoneMetadata* findById(int zoneId) const;
    const WorldZoneMetadata* findByName(const QString& name) const;
    /**
     * 根据世界坐标点查找最近的区域元数据，返回一个指向该区域元数据的指针。如果提供了regionId参数，则仅在指定大区内进行搜索。
      * @param worldPoint 世界坐标系中的点，用于查找最近的区域
      * @param regionId 可选的大区ID，如果提供则仅在该大区内搜索
      * @return 指向最近区域元数据的指针，如果没有找到则返回nullptr
     */
    const WorldZoneMetadata* nearestToWorldPoint(const QPointF& worldPoint,
                                                 std::optional<int> regionId = std::nullopt) const;
    QVector<const WorldZoneMetadata*> neighborsOf(int zoneId) const;
    QVector<const WorldZoneMetadata*> zones() const;
    bool isEmpty() const;

private:
    static QString normalizeName(const QString& name);
    void rebuildIndexes();

    QVector<WorldZoneMetadata> m_zones;
    QHash<int, int> m_indexById; // zoneId -> index in m_zones
    QHash<QString, int> m_indexByAlias; // normalized alias -> index in m_zones
};

#endif