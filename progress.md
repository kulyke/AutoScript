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

## Current Flow

- `StMainMenuToShop`
  - Click shop button
  - Wait several frames for page transition
  - Enter `StShop`
- `StShop`
  - Wait for shop title
  - Complete task

## Next Candidates

- Extract task registration and creation out of `MainWindow` into a task registry or factory.
- Add template caching and resource metadata before business templates grow.
- Introduce stable task IDs instead of name-based matching in UI and manager operations.
- Add non-empty tests for StepFlowState, RetryStep, TimeoutStep, and TaskBase transitions.