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
    } else if(next != prev) {
        // 切换到新状态，释放旧状态
        delete prev;
        m_currentState = next;
        m_stallFrames = 0;
        m_status = Running;
    } else { // next == prev
        // 状态未变化，先进入等待；持续等待超时判定失败
        ++m_stallFrames;
        if (m_stallFrames >= m_maxStallFrames) {
            m_status = NG;
        } else {
            m_status = Waiting;
        }
    }
}