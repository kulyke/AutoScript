#include "WorldZoneCatalog.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <limits>

namespace {

QPointF parsePoint(const QJsonObject& object)
{
    return QPointF(object.value("x").toDouble(), object.value("y").toDouble());
}

std::optional<WorldZoneType> parseZoneType(const QString& type)
{
    const QString normalized = type.trimmed().toCaseFolded();
    if (normalized == "dangerous") {
        return WorldZoneType::Dangerous;
    }
    if (normalized == "safe") {
        return WorldZoneType::Safe;
    }
    if (normalized == "obscure") {
        return WorldZoneType::Obscure;
    }
    if (normalized == "abyssal") {
        return WorldZoneType::Abyssal;
    }
    if (normalized == "stronghold") {
        return WorldZoneType::Stronghold;
    }
    if (normalized == "archive") {
        return WorldZoneType::Archive;
    }

    return std::nullopt;
}

}

bool WorldZoneCatalog::loadFromJson(const QString& path, QString* error)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        if (error) {
            *error = QString("Failed to open zone catalog: %1").arg(path);
        }
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        if (error) {
            *error = QString("Failed to parse zone catalog: %1").arg(parseError.errorString());
        }
        return false;
    }

    QJsonArray zonesArray;
    if (document.isArray()) {
        zonesArray = document.array();
    } else if (document.isObject()) {
        zonesArray = document.object().value("zones").toArray();
    }

    if (zonesArray.isEmpty()) {
        if (error) {
            *error = QString("Zone catalog is empty: %1").arg(path);
        }
        m_zones.clear();
        m_indexById.clear();
        m_indexByAlias.clear();
        return false;
    }

    QVector<WorldZoneMetadata> parsedZones;
    parsedZones.reserve(zonesArray.size());

    for (const QJsonValue& zoneValue : zonesArray) {
        const QJsonObject zoneObject = zoneValue.toObject();
        WorldZoneMetadata zone;
        zone.zoneId = zoneObject.value("zoneId").toInt(-1);
        zone.key = zoneObject.value("key").toString();
        zone.displayName = zoneObject.value("displayName").toString();
        zone.entryTemplateKey = zoneObject.value("entryTemplateKey").toString();
        zone.worldAnchor = parsePoint(zoneObject.value("worldAnchor").toObject());
        zone.missionAnchor = parsePoint(zoneObject.value("missionAnchor").toObject());
        zone.regionId = zoneObject.value("regionId").toInt();
        zone.hazardLevel = zoneObject.value("hazardLevel").toInt();
        zone.isPort = zoneObject.value("isPort").toBool();
        zone.isAzurPort = zoneObject.value("isAzurPort").toBool();

        const QJsonArray aliases = zoneObject.value("aliases").toArray();
        for (const QJsonValue& alias : aliases) {
            zone.aliases.append(alias.toString());
        }
        if (!zone.displayName.isEmpty() && !zone.aliases.contains(zone.displayName)) {
            zone.aliases.append(zone.displayName);
        }

        const QJsonArray defaultTypes = zoneObject.value("defaultTypes").toArray();
        for (const QJsonValue& typeValue : defaultTypes) {
            const auto parsedType = parseZoneType(typeValue.toString());
            if (parsedType.has_value()) {
                zone.defaultTypes.append(*parsedType);
            }
        }

        const QJsonArray neighbors = zoneObject.value("neighborZoneIds").toArray();
        for (const QJsonValue& neighbor : neighbors) {
            zone.neighborZoneIds.append(neighbor.toInt());
        }

        if (zone.zoneId < 0 || zone.key.isEmpty()) {
            if (error) {
                *error = QString("Invalid zone entry in catalog: zoneId=%1 key=%2")
                    .arg(zone.zoneId)
                    .arg(zone.key);
            }
            return false;
        }

        parsedZones.append(zone);
    }

    m_zones = std::move(parsedZones);
    rebuildIndexes();
    return true;
}

const WorldZoneMetadata* WorldZoneCatalog::findById(int zoneId) const
{
    const auto it = m_indexById.constFind(zoneId);
    if (it == m_indexById.constEnd()) {
        return nullptr;
    }

    return &m_zones[it.value()];
}

const WorldZoneMetadata* WorldZoneCatalog::findByName(const QString& name) const
{
    const QString normalized = normalizeName(name);
    const auto it = m_indexByAlias.constFind(normalized);
    if (it == m_indexByAlias.constEnd()) {
        return nullptr;
    }

    return &m_zones[it.value()];
}

const WorldZoneMetadata* WorldZoneCatalog::nearestToWorldPoint(const QPointF& worldPoint,
                                                               std::optional<int> regionId) const
{
    const WorldZoneMetadata* bestZone = nullptr;
    double bestDistance = std::numeric_limits<double>::max();

    for (const WorldZoneMetadata& zone : m_zones) {
        if (regionId.has_value() && zone.regionId != *regionId) {
            continue;
        }

        const QPointF delta = zone.worldAnchor - worldPoint;
        const double distance = delta.x() * delta.x() + delta.y() * delta.y();
        if (distance < bestDistance) {
            bestDistance = distance;
            bestZone = &zone;
        }
    }

    return bestZone;
}

QVector<const WorldZoneMetadata*> WorldZoneCatalog::neighborsOf(int zoneId) const
{
    QVector<const WorldZoneMetadata*> results;
    const WorldZoneMetadata* zone = findById(zoneId);
    if (!zone) {
        return results;
    }

    results.reserve(zone->neighborZoneIds.size());
    for (int neighborId : zone->neighborZoneIds) {
        const WorldZoneMetadata* neighbor = findById(neighborId);
        if (neighbor) {
            results.append(neighbor);
        }
    }

    return results;
}

QVector<const WorldZoneMetadata*> WorldZoneCatalog::zones() const
{
    QVector<const WorldZoneMetadata*> results;
    results.reserve(m_zones.size());
    for (const WorldZoneMetadata& zone : m_zones) {
        results.append(&zone);
    }

    return results;
}

bool WorldZoneCatalog::isEmpty() const
{
    return m_zones.isEmpty();
}

QString WorldZoneCatalog::normalizeName(const QString& name)
{
    QString normalized = name.trimmed().toCaseFolded();
    normalized.remove(' ');
    normalized.remove('-');
    normalized.remove('_');
    return normalized;
}

void WorldZoneCatalog::rebuildIndexes()
{
    m_indexById.clear();
    m_indexByAlias.clear();

    for (int index = 0; index < m_zones.size(); ++index) {
        const WorldZoneMetadata& zone = m_zones[index];
        m_indexById.insert(zone.zoneId, index);
        m_indexByAlias.insert(normalizeName(zone.key), index);
        for (const QString& alias : zone.aliases) {
            m_indexByAlias.insert(normalizeName(alias), index);
        }
    }
}