#include "taskbase.h"

TaskBase::TaskBase(QObject *parent)
    : QObject(parent)
{

}

void TaskBase::setInitialState(TaskState *state)
{
    m_currentState = state;
}

void TaskBase::execute(const QImage &frame)
{
    if(!m_currentState)
        return;
    //更新并获取下一个状态
    TaskState* next = m_currentState->update(frame);
    if(next != m_currentState) {
        m_currentState = next;
        m_status = Running;
    } else if (next == m_currentState) {
        m_status = NG;
    } else {
        //状态为nullptr，任务完成
        m_status = OK;
    }
}