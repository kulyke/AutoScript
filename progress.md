# AutoScript Progress

## Completed

- Built Qt + OpenCV desktop application scaffold with CMake.
- Implemented ADB-based screen capture and stabilized PNG decoding.
- Unified `adbPath` and `ip` into shared `AdbConfig`.
- Added automatic runtime deployment for OpenCV DLLs.
- Added automatic copy of template resources into build output.
- Moved screen capture, device control, vision engine, and task manager off the UI thread.
- Improved main window styling and moved QSS to `resources/styles/main.qss`.
- Enabled adaptive main window resizing and emulator preview rescaling.
- Added template matching diagnostics and upgraded matcher to multi-scale + NMS.
- Introduced task failure cleanup to avoid task/state leaks.
- Refactored shop flow to step-flow architecture.
- Added reusable `WaitTemplateStep` and `ClickTemplateStep`.
- Added reusable `TapPointStep`, `SwipeStep`, and `KeyEventStep`.
- Added `DelayFramesStep` for page-transition buffering.
- Added `TimeoutStep` for per-step timeout control.
- Added `RetryStep` for transient step failure recovery.
- Added task-level failure logging with state and reason propagation.
- Synced task table status with runtime task execution state.
- Showed current state name directly in the task table.
- Applied `RetryStep` to the current shop entry and shop verification flow.
- Made click and template steps surface dependency and action failures explicitly.
- Added step-level retry runtime logging to the UI log path.
- Removed the redundant `TaskState` abstraction and simplified the state hierarchy to `StepFlowState` only.
- Added success runtime logging for finished steps and preserved runtime messages across state transitions.
- Reviewed the framework readiness before business development and identified the next structural optimization targets.
- Extracted task registration and task creation out of `MainWindow` into a reusable task registry.
- Tightened task UI and manager matching from fuzzy name checks to exact task type matching.
- Added template metadata registration and in-memory template caching in the vision layer.
- Switched the current shop flow from hard-coded template paths to logical template keys.
- Added stable per-instance task IDs and switched UI/manager coordination to taskId-based matching.
- Added a QtTest-based framework test target covering StepFlowState, TaskBase, TimeoutStep, and RetryStep.
- Simplified TaskManager ownership so the current task is derived from the queue head and task failure no longer stops the whole scheduler.
- Scoped OpenCV include and link settings to individual CMake targets instead of relying on global project-wide configuration.
- Drafted a Zone data model and world-map transform design for future large-world navigation flows.
- Drafted a StepFlowState-style world-map jump state machine for cross-zone travel.
- Added compilable world-map foundation classes for zone catalog loading and fixed-center transform snapshots.
- Added starter large-world zone data and runtime copy rules for world-map resources.
- Wired shared world-map runtime context into the existing Erosion-leveling task path.
- Added a bootstrap state that initializes zone metadata and the fixed-center world-map transform after entering the world map.
- Updated project docs to reflect new large-world files and the removal of `StShop`.

## Current Flow

- `StMainMenuToShop`
  - Click shop button
  - Wait for shop title
  - Complete task
- `StMainMenuToAttackMenu`
  - Click attack button
  - Wait for attack menu title
  - Enter `StAttackMenuToWorldOcean`
- `StAttackMenuToWorldOcean`
  - Click world ocean button
  - Wait for world ocean title
  - Enter `StWorldOceanToWorldMap`
- `StWorldOceanToWorldMap`
  - Click world map button
  - Wait for world map title
  - Enter `StWorldMapBootstrap`
- `StWorldMapBootstrap`
  - Load world-map zone catalog
  - Initialize fixed-center world-map transform
  - Complete current bootstrap flow

## Next Candidates

- Add world-map template keys and calibration assets for pinned-zone, zone-type, and entry-state recognition.
- Add navigation-specific world-map steps after bootstrap, starting with focus, pin verification, and zone-type selection.
- Return to framework tests after the world-map navigation path reaches a runnable MVP.