#include "TemplateCatalog.h"

namespace {

QList<TemplateMetadata> buildTemplateDefinitions()
{
    return {
        {
            "shop.button",
            "resources/templates/shop_button.png",
            0.9,
            "Shop button on main menu"
        },
        {
            "shop.title",
            "resources/templates/shop_title.png",
            0.9,
            "Shop title after entering shop"
        },
        // 进攻相关模板
        {
            "mainMenu.attack.button",
            "resources/templates/mainMenu.attack.button.png",
            0.9,
            "Attack button on main menu"
        },
        {
            "attackMenu.title",
            "resources/templates/attackMenu.title.png",
            0.9,
            "Title of attack menu"
        },
        {
            "attackMenu.worldZone.button",
            "resources/templates/attackMenu.worldZone.button.png",
            0.9,
            "World Zone button on attack menu"
        },
        // 世界区域相关模板
        {
            "worldZone.title",
            "resources/templates/worldZone.title.png",
            0.9,
            "Title of world zone"
        },
        {
            "worldZone.worldMap.button",
            "resources/templates/worldZone.worldMap.button.png",
            0.9,
            "World Map button on world zone"
        },
        // 世界地图相关模板
        {
            "worldMap.title",
            "resources/templates/worldMap.title.png",
            0.9,
            "Title of world map"
        },
        {
            "worldMap.erosionLeveling1",
            "resources/templates/worldMap.erosionLeveling1.png",
            0.7,
            "Entry confirmation for default erosion leveling target zone"
        },
        {
            "worldMap.enterZone.button",
            "resources/templates/worldMap.enterZone.button.png",
            0.9,
            "Enter zone button on world map after clicking a zone"
        },
        // 计划作战相关模板
        {
            "worldZone.planBattle.button",
            "resources/templates/worldZone.planBattle.button.png",
            0.7,
            "Plan battle button on world zone page"
        },
        {
            "planBattle.title",
            "resources/templates/planBattle.title.png",
            0.9,
            "Title of battle planning page"
        },
        {
            "planBattle.confirm.button",
            "resources/templates/planBattle.confirm.button.png",
            0.9,
            "Confirm battle plan button on battle planning page"
        },
        {
            "worldZone.planBattle.stop.button",
            "resources/templates/worldZone.planBattle.stop.button.png",
            0.7,
            "Stop plan battle button on world zone page"
        },
        {
            "worldZone.planBattle.noAutoEvent.message",
            "resources/templates/worldZone.planBattle.noAutoEvent.message.png",
            0.9,
            "Plan battle stopped because no auto event is available"
        },
        {
            "worldZone.planBattle.leaveReward.button",
            "resources/templates/worldZone.planBattle.leaveReward.button.png",
            0.9,
            "Leave reward button on plan battle stop page"
        },
        // 油量相关模板
        {
            "worldZone.oil.add.button",
            "resources/templates/worldZone.oil.add.button.png",
            0.5,
            "Oil add button on world zone page"
        },
        {
            "worldZone.oil.refill.confirm.button",
            "resources/templates/worldZone.oil.refill.confirm.button.png",
            0.9,
            "Oil refill confirm button"
        },
        {
            "worldZone.oil.refill.cancel.button",
            "resources/templates/worldZone.oil.refill.cancel.button.png",
            0.9,
            "Oil refill cancel button"
        },
        {
            "worldZone.oil.refill.currentOil",
            "resources/templates/worldZone.oil.refill.currentOil.png",
            0.8,
            "Current oil anchor on oil refill dialog"
        },
        {
            "worldZone.oil.refill.supply.blue",
            "resources/templates/energySupplyBlue.png",
            0.7,
            "Blue energy supply box on oil refill dialog"
        },
        {
            "worldZone.oil.refill.supply.purple",
            "resources/templates/energySupplyPurple.png",
            0.7,
            "Purple energy supply box on oil refill dialog"
        },
        {
            "worldZone.oil.refill.supply.yellow",
            "resources/templates/energySupplyYellow.png",
            0.7,
            "Yellow energy supply box on oil refill dialog"
        },
        // 猫官商店相关模板
        {
            "worldZone.meowfficer.button",
            "resources/templates/worldZone.meowfficer.button.png",
            0.5,
            "Meowfficer shop button on world zone page"
        },
        {
            "meowfficerShop.title",
            "resources/templates/meowfficerShop.title.png",
            0.9,
            "Title of meowfficer shop"
        },
        {
            "meowfficerShop.energySupplyBox.button",
            "resources/templates/meowfficerShop.energySupplyBox.button.png",
            0.9,
            "Energy supply box item in meowfficer shop"
        },
        {
            "meowfficerShop.energySupplyBox.empty",
            "resources/templates/meowfficerShop.energySupplyBox.empty.png",
            0.9,
            "Indicator that energy supply boxes are depleted"
        },
        {
            "meowfficerShop.buy.confirm.button",
            "resources/templates/meowfficerShop.buy.confirm.button.png",
            0.9,
            "Confirm purchase button in meowfficer shop"
        }
    };
}

}

const QList<TemplateMetadata>& TemplateCatalog::definitions()
{
    static const QList<TemplateMetadata> templateDefinitions = buildTemplateDefinitions();
    return templateDefinitions;
}

const TemplateMetadata* TemplateCatalog::findByKey(const QString& key)
{
    for (const TemplateMetadata& definition : definitions()) {
        if (definition.key == key) {
            return &definition;
        }
    }

    return nullptr;
}