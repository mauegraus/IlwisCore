#ifndef OBJECTVISUALIZATIONMODEL_H
#define OBJECTVISUALIZATIONMODEL_H

#include <QQmlListProperty>
#include "resourcemodel.h"
#include "propertyeditor.h"

class VisualizationManager;
class PropertyEditor;

class ObjectVisualizationModel : public ResourceModel
{
    Q_OBJECT

public:
    ObjectVisualizationModel();
    ObjectVisualizationModel(const Ilwis::Resource& resource, QObject *obj=0);

private:
    void setEditors();
    std::map<QString, PropertyEditor *> _propertyEditors;

};

typedef QQmlListProperty<ObjectVisualizationModel> QMLObjectVisualizationList;
Q_DECLARE_METATYPE(QMLObjectVisualizationList)

#endif // OBJECTVISUALIZATIONMODEL_H
