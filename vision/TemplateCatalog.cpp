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