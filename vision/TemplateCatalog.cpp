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