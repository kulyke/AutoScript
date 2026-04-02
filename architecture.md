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

- `core/TaskState.h`: base interface for all task states.
- `core/TaskBase.h`: base task type, status definitions, task-level timeout configuration.
- `core/TaskBase.cpp`: task execution, state transition, task status update, state failure reason exposure.
- `core/TaskManager.h`: task queue manager interface.
- `core/TaskManager.cpp`: task scheduling, frame dispatch, task cleanup, queue control, runtime task status notifications.
- `core/FlowStep.h`: primitive step interface used by step-flow states.
- `core/StepFlowState.h`: state base class that executes a sequence of steps.
- `core/StepFlowState.cpp`: step sequence engine, failure propagation, flow completion handling.

## tasks

- `tasks/ShopTask.h`: concrete task type for shop automation.
- `tasks/ShopTask.cpp`: shop task implementation shell.

## tasks/states

- `tasks/states/StMainMenuToShop.h`: state declaration for entering shop from main menu.
- `tasks/states/StMainMenuToShop.cpp`: step-flow state that clicks shop button and waits for transition.
- `tasks/states/StShop.h`: state declaration for shop page verification.
- `tasks/states/StShop.cpp`: step-flow state that waits for shop title and completes task.

## tasks/steps

- `tasks/steps/TemplateSteps.h`: reusable template-related steps, fixed device actions, retry wrapper, and timing wrappers.
- `tasks/steps/TemplateSteps.cpp`: implementations of template wait/click, tap/swipe/keyevent, retry, frame delay, and step timeout.

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
- frame flow: `ScreenCapture -> TaskManager -> TaskBase -> TaskState/StepFlowState -> Vision/Device`.