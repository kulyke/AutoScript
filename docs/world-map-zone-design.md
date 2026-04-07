# World Map Zone And Transform Design

## Goal

This design targets Azur Lane Operation Siren style large-world navigation in the current Qt/C++ project.

The design follows three rules:

1. Store each zone once in a stable world coordinate system.
2. Convert between screen coordinates and world coordinates at runtime.
3. Validate every jump by checking pinned-zone or post-entry zone identity instead of trusting a raw tap.

## Coordinate Spaces

Use three coordinate spaces and keep them explicit in naming.

- ScreenSpace: emulator screenshot pixels, usually 1280x720.
- MapViewportSpace: the interactive world-map area inside the screenshot.
- WorldSpace: a stable reference-map coordinate system shared by all zones.

Recommended rule: all zone metadata lives in WorldSpace only.

Current measured calibration for the fixed-layout MVP:

- the world-coordinate center maps to screen point `(508, 423)`
- after cross-zone jumps, the same screen point remains stable

This means the first usable implementation can treat `(508, 423)` as a fixed screen anchor and only solve for the current `worldCenter`.

## Data Source Strategy

Do not hard-code per-route tap coordinates.

Use a static zone catalog file such as resources/world_map/zones.json with one record per zone.

Each record should contain:

- zoneId
- key
- displayName
- aliases
- worldAnchor
- missionAnchor
- regionId
- hazardLevel
- isPort
- isAzurPort
- defaultTypes
- neighborZoneIds

Example JSON shape:

```json
{
  "zoneId": 134,
  "key": "na_ocean_ne_d",
  "displayName": "NA海域东北D",
  "aliases": ["NA海域东北D", "NA Ocean NE Sector D"],
  "worldAnchor": { "x": 1176.0, "y": 1037.0 },
  "missionAnchor": { "x": 1134.0, "y": 995.0 },
  "regionId": 2,
  "hazardLevel": 2,
  "isPort": false,
  "isAzurPort": false,
  "defaultTypes": ["Dangerous", "Safe"],
  "neighborZoneIds": [131, 132, 133, 135, 141]
}
```

## Core Types

### Zone type enums

```cpp
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
    MainMenu,
    AttackMenu,
    WorldOcean,
    WorldMap,
    ZonePinned,
    ZoneTypeSelect,
    ZoneEntry,
    InZoneMap
};
```

### Zone metadata

```cpp
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
```

### Runtime target request

```cpp
struct WorldMapGotoRequest
{
    int targetZoneId = -1;
    QVector<WorldZoneType> preferredTypes;
    bool refreshCurrentZone = false;
    bool stopIfAlreadySafe = false;
    bool allowNeighborUnlockFallback = true;
};
```

### Runtime transform result

```cpp
struct WorldMapTransformSnapshot
{
    bool valid = false;
    double confidence = 0.0;

    QRect viewport;
    QSizeF referenceMapSize;

    QPointF worldCenter;
    QPointF screenCenter;

    QTransform worldToScreen;
    QTransform screenToWorld;
};
```

## Catalog Component

Recommended class split:

```cpp
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

private:
    QVector<WorldZoneMetadata> m_zones;
    QHash<int, int> m_indexById;
    QHash<QString, int> m_indexByAlias;
};
```

Responsibilities:

- Resolve user input such as zone id, Chinese name, or English alias.
- Map a pinned world point back to the nearest zone.
- Provide neighbor sets for locked-zone fallback.

## Transform Component

Recommended class:

```cpp
class WorldMapTransform
{
public:
    explicit WorldMapTransform(VisionEngine* vision);

    bool update(const QImage& frame, QString* error = nullptr);
    bool isValid() const;
    WorldMapTransformSnapshot snapshot() const;

    QPointF worldToScreen(const QPointF& worldPoint) const;
    QPointF screenToWorld(const QPointF& screenPoint) const;

    bool isWorldPointVisible(const QPointF& worldPoint, const QRect& sightArea) const;
    QPoint computeTapPointForZone(const WorldZoneMetadata& zone) const;
    QPointF computeSwipeVectorToward(const QPointF& worldPoint,
                                     const QSizeF& swipeLimit) const;

private:
    VisionEngine* m_vision = nullptr;
    WorldMapTransformSnapshot m_snapshot;
};
```

Responsibilities:

- Detect whether the current frame is the world-map page.
- Estimate the current world-map center from the current screenshot.
- Convert WorldSpace anchors into tap points on the current frame.
- Convert pinned UI anchor points back into WorldSpace for verification.

## How Transform Should Work

The implementation can evolve in three stages.

### Stage 1: fixed-layout MVP

- Lock emulator resolution and world-map zoom.
- Use a fixed viewport rectangle.
- Use the measured stable screen anchor `(508, 423)` as the on-screen world center.
- Store or infer `worldCenter` in the same coordinate system as zone metadata.
- Good enough for a runnable first version.

### Stage 2: screen-to-world calibration

- Add a world-map reference image.
- Detect current viewport contents against the reference image.
- Estimate current worldCenter.
- Compute dynamic tap positions from world anchors.

### Stage 3: robust projection

- Add homography or other perspective calibration when map projection drift appears.
- Support pinned-anchor reverse lookup and mission-anchor lookup.
- Handle small UI movement or zoom inconsistencies.

## Interaction Contract With Existing Modules

### VisionEngine

Keep VisionEngine responsible for UI-state recognition:

- page titles
- pinned-zone indicators
- zone type switch button
- zone entrance button
- locked-zone warning

WorldMapTransform should use VisionEngine only as an input helper, not as the owner of navigation state.

### DeviceController

Keep DeviceController unchanged.

World-map navigation should continue to use existing tap and swipe primitives.

### StepFlowState and FlowStep

Do not move world-map logic into TaskBase.

Add dedicated FlowStep implementations instead:

- UpdateWorldMapTransformStep
- FocusWorldZoneStep
- EnsurePinnedZoneStep
- SelectWorldZoneTypeStep
- EnterWorldZoneStep
- VerifyWorldZoneStep

## Failure Model

Represent failures explicitly so RetryStep and TimeoutStep remain effective.

Recommended failure categories:

- PageMismatch
- TransformUnavailable
- TargetNotVisibleAfterSwipe
- PinnedZoneMismatch
- ZoneTypeUnavailable
- ZoneLocked
- EntryTimeout
- ZoneVerifyFailed

## Proposed File Layout

- core/worldmap/WorldZoneCatalog.h
- core/worldmap/WorldZoneCatalog.cpp
- core/worldmap/WorldMapTransform.h
- core/worldmap/WorldMapTransform.cpp
- resources/world_map/zones.json
- resources/world_map/reference_map.png

## Implementation Notes

- Prefer JSON resources over hard-coded C++ tables once the zone count grows.
- Keep alias normalization inside WorldZoneCatalog so OCR noise handling stays localized.
- Keep world-anchor selection consistent. Use one anchor definition for both tap targeting and pin verification.
- Add region and neighbor data early, because locked-zone handling depends on them.

## Recommended MVP Order

1. Add WorldZoneCatalog with JSON-backed metadata.
2. Add fixed-layout WorldMapTransform that returns a calibrated world center.
3. Add focus, pin, and enter steps.
4. Add post-entry verification by zone title or fallback pin inference.
5. Add locked-zone neighbor fallback.