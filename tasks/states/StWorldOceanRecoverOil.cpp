#include "stworldoceanrecoveroil.h"

#include "visionengine.h"
#include "devicecontroller.h"
#include "steps/TemplateSteps.h"

#include "StWorldOceanPlanBattleMode.h"
#include "WorldOceanPlanBattleRuntimeContext.h"

#include <QDebug>

StWorldOceanRecoverOil::StWorldOceanRecoverOil(VisionEngine* vision,
                                               DeviceController* device,
                                               std::shared_ptr<WorldOceanPlanBattleRuntimeContext> runtimeContext,
                                               QObject* parent)
    : StepFlowState(parent)
    , m_vision(vision)
    , m_device(device)
    , m_runtimeContext(std::move(runtimeContext))
{
    addStep(std::make_unique<RetryStep>(
        std::make_unique<TimeoutMillisecondsStep>(
            std::make_unique<ClickTemplateStep>(
                m_vision,
                m_device,
                "worldZone.oil.add.button",
                -1.0,
                "Click oil add button"),
            2200,
            "Timeout click oil add button"),
        2,
        "Retry click oil add button"));
        
    // 等待油量补给对话框出现，确认点击加油按钮后界面有响应；如果界面无响应可能是误点了其他按钮，导致后续步骤找不到预期模板而失败
    addStep(std::make_unique<DelayMillisecondsStep>(100, "Wait oil refill dialog"));

    addStep(std::make_unique<RetryStep>(
        std::make_unique<TimeoutMillisecondsStep>(
            std::make_unique<ClickTemplateStep>(
                m_vision,
                m_device,
                "worldZone.oil.refill.confirm.button",
                -1.0,
                "Click oil refill confirm button"),
            2200,
            "Timeout click oil refill confirm button"),
        2,
        "Retry click oil refill confirm button"));

    addStep(std::make_unique<DelayMillisecondsStep>(160, "Wait oil refill complete"));

    // addStep(std::make_unique<RetryStep>(
    //     std::make_unique<TimeoutMillisecondsStep>(
    //         std::make_unique<WaitTemplateStep>(
    //             m_vision,
    //             "worldZone.title",
    //             -1.0,
    //             "Wait world zone title after oil refill"),
    //         3200,
    //         "Timeout wait world zone title after oil refill"),
    //     1,
    //     "Retry wait world zone title after oil refill"));
}

StWorldOceanRecoverOil::~StWorldOceanRecoverOil()
{
    qDebug() << "StWorldOceanRecoverOil destroyed";
}

QString StWorldOceanRecoverOil::name() const
{
    return "StWorldOceanRecoverOil";
}

StepFlowState* StWorldOceanRecoverOil::onFlowFinished()
{
    if (m_runtimeContext) {
        ++m_runtimeContext->oilRecoveryCount;
    }

    setRuntimeMessage("[StWorldOceanRecoverOil] finished");
    return new StWorldOceanPlanBattleMode(
        m_vision,
        m_device,
        m_runtimeContext);
}