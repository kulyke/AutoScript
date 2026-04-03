#include <QtTest>

#include "TaskBase.h"
#include "StepFlowState.h"
#include "steps/TemplateSteps.h"

#include <memory>
#include <utility>
#include <vector>

namespace {

class ScriptedStep : public FlowStep
{
public:
	struct Result {
		FlowStepStatus status;
		QString runtimeMessage;
		QString errorMessage;
	};

	ScriptedStep(QString stepName,
				 std::vector<Result> results,
				 Result resetResult = {FlowStepStatus::Done, QString(), QString()})
		: m_name(std::move(stepName))
		, m_results(std::move(results))
		, m_resetResult(std::move(resetResult))
	{
	}

	QString name() const override
	{
		return m_name;
	}

	FlowStepStatus execute(const QImage& frame) override
	{
		Q_UNUSED(frame);

		const Result result = currentResult();
		m_runtimeMessage = result.runtimeMessage;
		m_errorMessage = result.errorMessage;
		if (m_cursor + 1 < static_cast<int>(m_results.size())) {
			++m_cursor;
		}
		return result.status;
	}

	void reset() override
	{
		++m_resetCount;
		m_cursor = 0;
		if (!m_results.empty()) {
			m_results[0] = m_resetResult;
		}
	}

	QString takeRuntimeMessage() override
	{
		const QString message = m_runtimeMessage;
		m_runtimeMessage.clear();
		return message;
	}

	QString errorString() const override
	{
		return m_errorMessage;
	}

	int resetCount() const
	{
		return m_resetCount;
	}

private:
	Result currentResult() const
	{
		if (m_results.empty()) {
			return {FlowStepStatus::Done, QString(), QString()};
		}
		return m_results[m_cursor];
	}

	QString m_name;
	std::vector<Result> m_results;
	Result m_resetResult;
	int m_cursor = 0;
	int m_resetCount = 0;
	QString m_runtimeMessage;
	QString m_errorMessage;
};

class TestState : public StepFlowState
{
public:
	explicit TestState(QString stateName)
		: m_name(std::move(stateName))
	{
	}

	QString name() const override
	{
		return m_name;
	}

	void appendStep(std::unique_ptr<FlowStep> step)
	{
		addStep(std::move(step));
	}

	void setCompletionMessage(QString message)
	{
		m_completionMessage = std::move(message);
	}

	void setNextState(StepFlowState* nextState)
	{
		m_nextState = nextState;
	}

private:
	StepFlowState* onFlowFinished() override
	{
		if (!m_completionMessage.isEmpty()) {
			setRuntimeMessage(m_completionMessage);
		}
		return m_nextState;
	}

	QString m_name;
	QString m_completionMessage;
	StepFlowState* m_nextState = nullptr;
};

class TestTask : public TaskBase
{
public:
	QString name() const override
	{
		return "TestTask";
	}
};

class FrameworkTests : public QObject
{
	Q_OBJECT

private slots:
	void stepFlowStateProducesSuccessRuntimeMessage();
	void taskBaseBuffersRuntimeMessageAcrossCompletion();
	void timeoutStepFailsAfterFrameLimit();
	void retryStepResetsInnerStepAndReportsRetry();
};

void FrameworkTests::stepFlowStateProducesSuccessRuntimeMessage()
{
	TestState state("TestState");
	state.appendStep(std::make_unique<ScriptedStep>(
		"StepA",
		std::vector<ScriptedStep::Result>{{FlowStepStatus::Done, QString(), QString()}}));

	QVERIFY(state.update(QImage()) == nullptr);
	QCOMPARE(state.takeRuntimeMessage(), QString("[TestState] step finished: StepA"));
}

void FrameworkTests::taskBaseBuffersRuntimeMessageAcrossCompletion()
{
	auto* state = new TestState("CompletionState");
	state->appendStep(std::make_unique<ScriptedStep>(
		"StepDone",
		std::vector<ScriptedStep::Result>{{FlowStepStatus::Done, QString(), QString()}}));
	state->setCompletionMessage("[CompletionState] flow completed");

	TestTask task;
	task.setInitialState(state);
	task.execute(QImage());

	QCOMPARE(task.status(), TaskBase::OK);
	QCOMPARE(task.takeRuntimeMessage(), QString("[CompletionState] flow completed"));
}

void FrameworkTests::timeoutStepFailsAfterFrameLimit()
{
	TimeoutStep timeoutStep(
		std::make_unique<ScriptedStep>(
			"WaitForever",
			std::vector<ScriptedStep::Result>{{FlowStepStatus::Running, QString(), QString()}}),
		2,
		"Timeout wrapper");

	QCOMPARE(timeoutStep.execute(QImage()), FlowStepStatus::Running);
	QCOMPARE(timeoutStep.execute(QImage()), FlowStepStatus::Failed);
	QVERIFY(timeoutStep.errorString().contains("timed out after 2 frames"));
}

void FrameworkTests::retryStepResetsInnerStepAndReportsRetry()
{
	auto* innerStep = new ScriptedStep(
		"FlakyStep",
		std::vector<ScriptedStep::Result>{{FlowStepStatus::Failed, QString(), "first failure"}},
		{FlowStepStatus::Done, QString(), QString()});

	RetryStep retryStep(std::unique_ptr<FlowStep>(innerStep), 2, "Retry wrapper");

	QCOMPARE(retryStep.execute(QImage()), FlowStepStatus::Running);
	QCOMPARE(retryStep.takeRuntimeMessage(),
			 QString("retry 1/2 for step 'FlakyStep': first failure"));
	QCOMPARE(innerStep->resetCount(), 1);
	QCOMPARE(retryStep.execute(QImage()), FlowStepStatus::Done);
}

}

QTEST_MAIN(FrameworkTests)
#include "test_tasks.moc"
