# World Map Jump State Machine Sketch

## Goal

This state machine follows the current StepFlowState style already used by the shop flow.

The design keeps each state narrow:

- page entry state
- transform update state
- focus and pin state
- type selection state
- entry state
- post-entry verification state

## Proposed Task Entry

```cpp
class WorldMapGotoTask : public TaskBase
{
public:
    WorldMapGotoTask(VisionEngine* vision,
                     DeviceController* device,
                     WorldZoneCatalog* zoneCatalog,
                     WorldMapTransform* worldMapTransform,
                     WorldMapGotoRequest request,
                     QObject* parent = nullptr);

    QString name() const override;
};
```

Initial state:

- StMainMenuToWorldMap

## State Overview

### StMainMenuToWorldMap

Purpose:

- navigate from current top-level page into the world-map page

Expected reusable steps:

- ClickTemplateStep mainMenu.attack.button
- WaitTemplateStep attackMenu.title
- ClickTemplateStep attackMenu.worldOcean.button
- WaitTemplateStep worldOcean.title
- ClickTemplateStep worldOcean.worldMap.button
- WaitTemplateStep worldMap.title

Success transition:

- StWorldMapUpdateCamera

Failure examples:

- failed to enter attack menu
- failed to enter world ocean
- failed to enter world map

### StWorldMapUpdateCamera

Purpose:

- refresh WorldMapTransform from the current frame
- decide whether the current zone already matches the request

Fixed-layout MVP note:

- use the measured stable screen anchor `(508, 423)` as the world-center projection point
- this state only needs to establish or refresh the current `worldCenter`, not solve a moving on-screen center

Recommended custom step:

- UpdateWorldMapTransformStep

Runtime outputs:

- current transform confidence
- current nearest zone id

Success transitions:

- if current zone equals target and no refresh is required: StWorldMapVerifyZone
- otherwise: StWorldMapFocusZone

Failure examples:

- world-map transform update failed
- current frame is not a valid world-map page

### StWorldMapFocusZone

Purpose:

- move the target zone into sight
- tap the computed anchor
- wait until the pinned zone matches the target zone

Recommended custom steps:

- FocusWorldZoneStep
- EnsurePinnedZoneStep

Internal loop logic:

1. unpin previous zone if needed
2. update transform
3. if target not visible, compute swipe vector and swipe
4. recompute target tap point
5. tap target point
6. wait until pinned zone appears
7. convert pinned anchor back to WorldSpace
8. verify nearest zone equals target zone

Success transition:

- StWorldMapSelectZoneType

Failure examples:

- target could not be brought into sight within retry budget
- pinned zone does not match requested zone
- transform drift makes target tap point unstable

### StWorldMapSelectZoneType

Purpose:

- choose safe or dangerous type when the pinned zone exposes a type switch

Recommended custom step:

- SelectWorldZoneTypeStep

Decision rule:

- try preferredTypes in request order
- if not available, optionally fall back to Dangerous then Safe
- if zone has no switch UI, treat current pinned type as final

Success transition:

- StWorldMapEnterZone

Failure examples:

- requested zone type not available
- zone type switch never stabilized

### StWorldMapEnterZone

Purpose:

- click the pinned zone entrance and wait until the game enters the zone map

Recommended custom step:

- EnterWorldZoneStep

Internal checks:

- locked-zone marker exists
- entry button exists
- action-point popup appears
- reward popup blocks the page

Success transition:

- StWorldMapVerifyZone

Failure examples:

- zone locked by unexplored neighbors
- zone entrance timeout
- blocked by unhandled popup

### StWorldMapVerifyZone

Purpose:

- confirm the game actually entered the requested zone

Verification order:

1. wait for in-zone page markers
2. OCR or template match current zone title if available
3. normalize title and resolve through WorldZoneCatalog
4. if OCR fails, reopen or reuse pinned-zone inference fallback

Recommended custom step:

- VerifyWorldZoneStep

Success transition:

- flow finished

Failure examples:

- entered map but zone identity mismatch
- zone title unreadable and fallback inference unavailable

## Optional Recovery States

These are not required for MVP but should be reserved in the design.

### StWorldMapRecoverPinnedState

Use when the wrong zone remains pinned and repeated swipe-to-unpin is needed.

### StWorldMapUnlockNeighbor

Use when target zone is locked and request.allowNeighborUnlockFallback is true.

Flow:

- choose a neighbor from neighborZoneIds
- jump to the neighbor first
- finish that zone or at least unlock target adjacency
- return to StWorldMapUpdateCamera

### StWorldMapRecoverToKnownPort

Use when transform confidence becomes unstable or the page drifts into a bad camera position.

Flow:

- jump to nearest azur port
- reopen world map
- rebuild transform from a known stable page

## Suggested Step Classes

```cpp
class UpdateWorldMapTransformStep : public FlowStep;
class FocusWorldZoneStep : public FlowStep;
class EnsurePinnedZoneStep : public FlowStep;
class SelectWorldZoneTypeStep : public FlowStep;
class EnterWorldZoneStep : public FlowStep;
class VerifyWorldZoneStep : public FlowStep;
```

These steps should keep the same contract as current template steps:

- execute(frame)
- return Running, Done, or Failed
- expose errorString
- emit short runtime messages for TaskManager logs

## State Transition Sketch

```text
StMainMenuToWorldMap
  -> StWorldMapUpdateCamera

StWorldMapUpdateCamera
  -> StWorldMapVerifyZone        when already at target and no refresh requested
  -> StWorldMapFocusZone         otherwise

StWorldMapFocusZone
  -> StWorldMapSelectZoneType
  -> StWorldMapRecoverPinnedState    optional

StWorldMapSelectZoneType
  -> StWorldMapEnterZone

StWorldMapEnterZone
  -> StWorldMapVerifyZone
  -> StWorldMapUnlockNeighbor        optional on locked zone

StWorldMapVerifyZone
  -> finished
  -> StWorldMapRecoverToKnownPort    optional on transform or page corruption
```

## Constructor Sketches

```cpp
class StWorldMapUpdateCamera : public StepFlowState
{
public:
    StWorldMapUpdateCamera(VisionEngine* vision,
                           WorldZoneCatalog* zoneCatalog,
                           WorldMapTransform* transform,
                           const WorldMapGotoRequest& request,
                           QObject* parent = nullptr);

    QString name() const override;

protected:
    StepFlowState* onFlowFinished() override;
};
```

```cpp
class StWorldMapFocusZone : public StepFlowState
{
public:
    StWorldMapFocusZone(VisionEngine* vision,
                        DeviceController* device,
                        WorldZoneCatalog* zoneCatalog,
                        WorldMapTransform* transform,
                        const WorldMapGotoRequest& request,
                        QObject* parent = nullptr);

    QString name() const override;

protected:
    StepFlowState* onFlowFinished() override;
};
```

## Runtime Logging Expectations

Each state should emit one compact runtime message per meaningful transition.

Examples:

- [StWorldMapUpdateCamera] world center=1172,1036 confidence=0.84
- [StWorldMapFocusZone] swipe toward zone 134 vector=220,-96
- [StWorldMapFocusZone] pinned zone=134
- [StWorldMapSelectZoneType] selected Dangerous
- [StWorldMapEnterZone] entering zone 134
- [StWorldMapVerifyZone] verified zone=134

## MVP Cut Line

The first runnable version only needs these states:

- StMainMenuToWorldMap
- StWorldMapUpdateCamera
- StWorldMapFocusZone
- StWorldMapSelectZoneType
- StWorldMapEnterZone
- StWorldMapVerifyZone

Everything else can remain as follow-up recovery work once the basic jump path is stable.