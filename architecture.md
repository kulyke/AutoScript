# AutoScript Architecture

## Root

- `CMakeLists.txt`: project build configuration, target-scoped Qt/OpenCV linking, post-build resource/DLL/script copy, and `AutoScriptTests` test target.
- `docs/world-map-zone-design.md`: design draft for Operation Siren zone metadata, coordinate spaces, and transform service boundaries.
- `docs/world-map-jump-state-machine.md`: StepFlowState-style state machine sketch for large-world cross-zone jumping.
- `progress.md`: completed work and next-step tracking.
- `architecture.md`: file responsibility overview.
- `requirements-paddleocr.txt`: Python dependency list for the PaddleOCR helper runtime.

## test

- `test/test_tasks.cpp`: QtTest-based framework tests for StepFlowState, TaskBase, RetryStep, and TimeoutStep.
- `test/test_matcher.cpp`: placeholder for future vision matcher tests.

## app

- `app/main.cpp`: application entry, Qt message handler, log file initialization.
- `app/mainwindow.h`: main window declaration, UI references, worker thread members.
- `app/mainwindow.cpp`: main UI logic, thread setup, task creation/removal, start/stop orchestration, and taskId-based table synchronization.
- `app/mainwindow.ui`: Qt Designer layout for the main window.

## automation

- `automation/ScreenCapture.h`: capture component interface and signals.
- `automation/ScreenCapture.cpp`: ADB screenshot capture, PNG decoding, frame emission.
- `automation/DeviceController.h`: device action interface for tap/swipe/key/text.
- `automation/DeviceController.cpp`: ADB command execution for device operations.

## scripts

- `scripts/paddle_ocr_service.py`: persistent Python OCR worker that keeps a TextRecognition model warm for digit-only oil OCR, normalizes the structured result payload, and returns digit-only recognition results over JSON lines.

## config

- `config/AdbConfig.h`: shared ADB configuration (`adbPath`, `ip`).

## core

- `core/TaskBase.h`: base task type, status definitions, task-level timeout configuration.
- `core/TaskBase.cpp`: task execution, state transition, task status update, stable taskId ownership, state failure reason exposure, and buffered runtime message forwarding across state switches.
- `core/TaskManager.h`: task queue manager interface.
- `core/TaskManager.cpp`: task scheduling, frame dispatch, queue-head-driven current task selection, task cleanup, taskId-based runtime notifications, and UI log forwarding for step runtime messages.
- `core/FlowStep.h`: primitive step interface used by step-flow states.
- `core/StepFlowState.h`: the only task state base class, combining state lifecycle hooks and step-flow support.
- `core/StepFlowState.cpp`: step sequence engine, failure propagation, flow completion handling, and success/failure runtime message capture.
- `core/worldmap/WorldMapTypes.h`: shared large-world enums, zone metadata, goto request, calibration, and transform snapshot types.
- `core/worldmap/WorldMapRuntimeContext.h`: shared mutable runtime context for current zone, focused zone, last pinned zone, navigation action, and goto request state.
- `core/worldmap/WorldZoneCatalog.h`: zone catalog interface for JSON loading and zone lookup.
- `core/worldmap/WorldZoneCatalog.cpp`: JSON-backed zone metadata parsing, alias indexing, nearest-zone lookup, and neighbor resolution.
- `core/worldmap/WorldMapTransform.h`: fixed-center world-map transform service interface.
- `core/worldmap/WorldMapTransform.cpp`: current fixed-layout transform implementation using stable screen center `(508, 423)`.

## tasks

- `tasks/TaskRegistry.h`: task metadata and factory interface for UI-to-business registration.
- `tasks/TaskRegistry.cpp`: concrete task registration table and task creation callbacks.
- `tasks/ErosionLevelingTask.h`: large-world task shell that only owns the task entry state and shared vision/device dependencies.
- `tasks/ErosionLevelingTask.cpp`: large-world task entry that reaches world-map bootstrap without pre-allocating map-session services or goto requests.
- `tasks/ShopTask.h`: concrete task type for shop automation.
- `tasks/ShopTask.cpp`: shop task implementation shell.

## tasks/states

- `tasks/states/StMainMenuToAttackMenu.h`: state declaration for entering the attack menu from main menu.
- `tasks/states/StMainMenuToAttackMenu.cpp`: step-flow state that enters the attack menu and branches toward world-ocean navigation.
- `tasks/states/StAttackMenuToWorldOcean.h`: state declaration for entering the world ocean page.
- `tasks/states/StAttackMenuToWorldOcean.cpp`: step-flow state that enters world ocean from the attack menu.
- `tasks/states/StMainMenuToShop.h`: state declaration for entering shop from main menu.
- `tasks/states/StMainMenuToShop.cpp`: step-flow state that retries clicking the shop button, waits for the shop title, and completes the simplified shop flow.
- `tasks/states/StWorldOceanToWorldMap.h`: state declaration for opening the world map from world ocean.
- `tasks/states/StWorldOceanToWorldMap.cpp`: step-flow state that opens the world map and hands off to world-map bootstrap.
- `tasks/states/StWorldMapBootstrap.h`: state declaration for large-world context bootstrap after reaching the world map, including shared ownership of map-session services.
- `tasks/states/StWorldMapBootstrap.cpp`: step-flow state that creates the map-session services, creates the default runtime goto request, loads zone metadata, initializes the fixed-center world-map transform, and hands shared ownership to downstream states.
- `tasks/states/StWorldMapResolveCurrentZone.h`: state declaration for resolving the current world zone after bootstrap.
- `tasks/states/StWorldMapResolveCurrentZone.cpp`: step-flow state that derives the current zone from the active world center while retaining shared ownership of map-session services.
- `tasks/states/StWorldMapFocusTargetZone.h`: state declaration for centering the requested target zone on the world map.
- `tasks/states/StWorldMapFocusTargetZone.cpp`: step-flow state that performs swipe-based target-zone focusing and hands off to target-zone entry.
- `tasks/states/StWorldMapEnterTargetZone.h`: state declaration for tapping the centered target zone and verifying arrival.
- `tasks/states/StWorldMapEnterTargetZone.cpp`: step-flow state that taps the focused zone and waits for the configured entry template to confirm the jump.
- `tasks/states/WorldOceanPlanBattleRuntimeContext.h`: shared runtime counters, depletion flag, and last observed oil value for plan-battle monitoring and recovery loops.
- `tasks/states/StWorldOceanPlanBattleMode.h`: state declaration for entering official plan battle mode in a world zone.
- `tasks/states/StWorldOceanPlanBattleMode.cpp`: step-flow state that opens plan battle mode and hands off to the monitoring loop.
- `tasks/states/StWorldOceanMonitorPlanBattle.h`: state declaration for continuous plan-battle monitoring.
- `tasks/states/StWorldOceanMonitorPlanBattle.cpp`: frame-driven monitor that OCRs the current oil value, watches for stop conditions, and transitions into recovery states.
- `tasks/states/StWorldOceanRecoverOil.h`: state declaration for oil recovery during plan battle.
- `tasks/states/StWorldOceanRecoverOil.cpp`: step-flow state that stops plan battle, refills oil, and restarts plan battle.
- `tasks/states/StWorldOceanHandleMeowfficerShop.h`: state declaration for meowfficer shop handling during plan battle.
- `tasks/states/StWorldOceanHandleMeowfficerShop.cpp`: step-flow state that buys energy supply boxes when available and ends monitoring when the inventory is depleted.
- `tasks/states/StShop.h` / `tasks/states/StShop.cpp`: removed from the repository; shop verification now lives inside `StMainMenuToShop`.

## tasks/steps

- `tasks/steps/TemplateSteps.h`: reusable template-related steps, fixed device actions, retry wrapper, and timing wrappers.
- `tasks/steps/TemplateSteps.cpp`: implementations of template wait/click, tap/swipe/keyevent, retry, frame delay, and step timeout, with explicit action failure reporting and retry runtime messages.
- `tasks/steps/WorldOceanSteps.h`: plan-battle-specific custom step declarations.
- `tasks/steps/WorldOceanSteps.cpp`: custom steps for meowfficer supply purchase and depletion detection.
- `tasks/steps/WorldMapSteps.h`: world-map-specific step declarations.
- `tasks/steps/WorldMapSteps.cpp`: world-map bootstrap, current-zone resolution, target-zone focus, target-zone tap, and entry verification steps for metadata loading and jump-chain execution.

## vision

- `vision/TemplateCatalog.h`: template metadata model and lookup interface.
- `vision/TemplateCatalog.cpp`: current logical template key registry and default thresholds, including the oil-add anchor template used by lightweight oil OCR.
- `vision/VisionEngine.h`: high-level vision API used by task logic.
- `vision/VisionEngine.cpp`: image conversion, template metadata resolution, cached template loading, template matching, anchor-based oil ROI extraction, a persistent PaddleOCR bridge, and a local fallback digit recognizer.
- `vision/TemplateMatcher.h`: low-level matcher interface.
- `vision/TemplateMatcher.cpp`: multi-scale template matching with NMS and score output.

## resources

- `resources/resources.qrc`: Qt resource collection root.
- `resources/styles/main.qss`: UI stylesheet.
- `resources/templates/*`: template images used for recognition.
- `resources/world_map/zones.json`: starter world-map metadata and fixed-center calibration seed data.
- `resources/icons/*`: UI icon assets.

## Runtime Threading Model

- UI thread: `MainWindow`, visual rendering, user interaction.
- worker thread: `ScreenCapture`, `DeviceController`, `VisionEngine`, `TaskManager`.
- frame flow: `ScreenCapture -> TaskManager -> TaskBase -> StepFlowState -> Vision/Device`.

## Current Optimization Focus

- World-map navigation now creates both map-session services and the default goto request inside bootstrap, then executes focus -> tap -> entry verification as the current jump-chain MVP.
- Plan battle mode now prefers a Paddle-backed TextRecognition helper for oil detection, falls back to the local ROI-based digit recognizer when needed, and can trigger oil refill when the recognized oil value is at most 10.
- PaddleOCR runtime now depends on the workspace .venv and caches its downloaded PP-OCRv5 models under the local user Paddle model cache on first launch.
- Keep framework tests deferred until the world-map jump flow reaches a runnable MVP.
- Treat further build cleanup as low-priority infrastructure work: reduce broad source globbing when CMake changes again.