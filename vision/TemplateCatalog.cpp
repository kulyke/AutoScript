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
            "attackMenu.worldOcean.button",
            "resources/templates/attackMenu.worldOcean.button.png",
            0.9,
            "World Ocean button on attack menu"
        },
        {
            "worldOcean.title",
            "resources/templates/worldOcean.title.png",
            0.9,
            "Title of world ocean"
        },
        {
            "worldOcean.worldMap.button",
            "resources/templates/worldOcean.worldMap.button.png",
            0.9,
            "World Map button on world ocean"
        },
        {
            "worldMap.title",
            "resources/templates/worldMap.title.png",
            0.9,
            "Title of world map"
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