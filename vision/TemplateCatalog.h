#ifndef TEMPLATECATALOG_H
#define TEMPLATECATALOG_H

#include <QList>
#include <QString>

/**
 * @brief 模板元数据结构体
 */
struct TemplateMetadata
{
    QString key;
    QString path;
    double defaultThreshold;
    QString description;
};
/**
 * @brief 模板目录类，提供模板元数据的访问接口
 */
class TemplateCatalog
{
public:
    /**
     * @brief 获取所有模板元数据定义
     * @return 模板元数据列表
     */
    static const QList<TemplateMetadata>& definitions();
    /**
     * @brief 根据键查找模板元数据
     * @param key 模板键
     * @return 模板元数据指针，如果未找到则返回nullptr
     */
    static const TemplateMetadata* findByKey(const QString& key);
};

#endif