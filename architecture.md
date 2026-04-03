# AutoScript Architecture

## Root

- `CMakeLists.txt`: project build configuration, Qt/OpenCV linking, post-build resource and DLL copy.
- `progress.md`: completed work and next-step tracking.
- `architecture.md`: file responsibility overview.

## app

- `app/main.cpp`: application entry, Qt message handler, log file initialization.
- `app/mainwindow.h`: main window declaration, UI references, worker thread members.
- `app/mainwindow.cpp`: main UI logic, thread setup, task creation/removal, start/stop orchestration, task table status and state-name sync.
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
- `core/TaskBase.cpp`: task execution, state transition, task status update, state failure reason exposure, and buffered runtime message forwarding across state switches.
- `core/TaskManager.h`: task queue manager interface.
- `core/TaskManager.cpp`: task scheduling, frame dispatch, task cleanup, queue control, runtime task status notifications, and UI log forwarding for step runtime messages.
- `core/FlowStep.h`: primitive step interface used by step-flow states.
- `core/StepFlowState.h`: the only task state base class, combining state lifecycle hooks and step-flow support.
- `core/StepFlowState.cpp`: step sequence engine, failure propagation, flow completion handling, and success/failure runtime message capture.

## tasks

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

- `vision/VisionEngine.h`: high-level vision API used by task logic.
- `vision/VisionEngine.cpp`: image conversion and template file loading before matching.
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

- Move business task registration and creation out of `MainWindow` to reduce UI/business coupling.
- Add a template cache layer above `VisionEngine` to avoid repeated disk reads during frame processing.
- Replace task name matching in UI and manager code with stable task identifiers for multi-instance business tasks.
- Fill the empty test layer so the step-flow runtime has regression coverage before business flows multiply.