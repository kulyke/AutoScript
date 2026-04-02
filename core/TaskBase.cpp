#include "taskbase.h"

TaskBase::TaskBase(QObject *parent)
    : QObject(parent)
{

}

TaskBase::~TaskBase()
{
    delete m_currentState;
    m_currentState = nullptr;
}

void TaskBase::setInitialState(TaskState *state)
{
    m_currentState = state;
    m_stallFrames = 0;
    m_status = Idle;
}

void TaskBase::execute(const QImage &frame)
{
    if(!m_currentState) {
        // 无状态表示任务已完成
        m_status = OK;
        return;
    }

    //更新并获取下一个状态
    TaskState* prev = m_currentState;
    TaskState* next = m_currentState->update(frame);

    if(next == nullptr) {
        // 状态返回空，任务完成
        delete prev;
        m_currentState = nullptr;
        m_stallFrames = 0;
        m_status = OK;
    } else if (next == prev && prev->isFailed()) {
        m_status = NG;
    } else if(next != prev) {
        // 切换到新状态，释放旧状态
        delete prev;
        m_currentState = next;
        m_stallFrames = 0;
        m_status = Running;
    } else { // next == prev
        // 状态未变化，先进入等待；部分状态自行管理步骤级超时
        if (prev->usesExternalTimeout()) {
            ++m_stallFrames;
            if (m_stallFrames >= m_maxStallFrames) {
                m_status = NG;
            } else {
                m_status = Waiting;
            }
        } else {
            m_status = Waiting;
        }
    }
}

QString TaskBase::currentStateName() const
{
    return m_currentState ? m_currentState->name() : QString();
}

QString TaskBase::failureReason() const
{
    return m_currentState ? m_currentState->failureReason() : QString();
}