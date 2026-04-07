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
    const WorldZoneMetadata* nearestToWorldPoint(const QPointF& worldPoint,
                                                 std::optional<int> regionId = std::nullopt) const;
    QVector<const WorldZoneMetadata*> neighborsOf(int zoneId) const;
    QVector<const WorldZoneMetadata*> zones() const;
    bool isEmpty() const;

private:
    static QString normalizeName(const QString& name);
    void rebuildIndexes();

    QVector<WorldZoneMetadata> m_zones;
    QHash<int, int> m_indexById;
    QHash<QString, int> m_indexByAlias;
};

#endif