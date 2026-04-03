# AutoScript Architecture

## Root

- `CMakeLists.txt`: project build configuration, target-scoped Qt/OpenCV linking, post-build resource and DLL copy, and `AutoScriptTests` test target.
- `progress.md`: completed work and next-step tracking.
- `architecture.md`: file responsibility overview.

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

## tasks

- `tasks/TaskRegistry.h`: task metadata and factory interface for UI-to-business registration.
- `tasks/TaskRegistry.cpp`: concrete task registration table and task creation callbacks.
- `tasks/ShopTask.h`: concrete task type for shop automation.
- `tasks/ShopTask.cpp`: shop task implementation shell.

## tasks/states

- `tasks/states/StMainMenuToShop.h`: state declaration for entering shop from main menu.
- `tasks/states/StMainMenuToShop.cpp`: step-flow state that retries clicking the shop button and waits for transition.
- `tasks/states/StShop.h`: state declaration for shop page verification.
- `tasks/states/StShop.cpp`: step-flow state that retries waiting for the shop title and completes task.

## tasks/steps

- `tasks/steps/TemplateSteps.h`: reusable template-related steps, fixed device actions, retry wrapper, and timing wrappers.
- `tasks/steps/TemplateSteps.cpp`: implementations of template wait/click, tap/swipe/keyevent, retry, frame delay, and step timeout, with explicit action failure reporting and retry runtime messages.

## vision

- `vision/TemplateCatalog.h`: template metadata model and lookup interface.
- `vision/TemplateCatalog.cpp`: current logical template key registry and default thresholds.
- `vision/VisionEngine.h`: high-level vision API used by task logic.
- `vision/VisionEngine.cpp`: image conversion, template metadata resolution, cached template loading, and matching.
- `vision/TemplateMatcher.h`: low-level matcher interface.
- `vision/TemplateMatcher.cpp`: multi-scale template matching with NMS and score output.

## resources

- `resources/resources.qrc`: Qt resource collection root.
- `resources/styles/main.qss`: UI stylesheet.
- `resources/templates/*`: template images used for recognition.
- `resources/icons/*`: UI icon assets.

## Runtime Threading Model

- UI thread: `MainWindow`, visual rendering, user interaction.
- worker thread: `ScreenCapture`, `DeviceController`, `VisionEngine`, `TaskManager`.
- frame flow: `ScreenCapture -> TaskManager -> TaskBase -> StepFlowState -> Vision/Device`.

## Current Optimization Focus

- Expand framework tests upward into TaskManager scheduling, failure handling, and vision/template catalog behavior.
- Keep task ID tooling lightweight; UI display or copy support is useful for debugging, but not a blocker for new business flows.
- Treat further build cleanup as low-priority infrastructure work: reduce broad source globbing when CMake changes again.