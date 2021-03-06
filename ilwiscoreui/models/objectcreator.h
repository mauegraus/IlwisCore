#ifndef OBJECTCREATOR_H
#define OBJECTCREATOR_H

#include <QObject>
#include <QStringList>
#include <QQmlListProperty>
#include <vector>
#include "kernel.h"
#include "ilwisdata.h"
#include "ilwiscoreui_global.h"

namespace Ilwis {
class Resource;
class IlwisObject;
typedef IlwisData<IlwisObject> IIlwisObject;
class OperationResource;
}
class IlwisObjectCreatorModel;
class CatalogModel;

class ILWISCOREUISHARED_EXPORT ObjectCreator : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QStringList createableObjects READ createableObjects CONSTANT)

public:
    explicit ObjectCreator(QObject *parent = 0);
    ~ObjectCreator();


    Q_INVOKABLE QString createObject(const QVariantMap& parms);
    Q_INVOKABLE QVariantMap creatorInfo(const QString &name)  const;
    Q_INVOKABLE QObject * createModellerObject(const QVariantMap& parms, QObject *parent);

    QStringList createableObjects() const;


signals:
    void activeCreatorsChanged();
    void error(const QString& err);

public slots:

private:
    std::map<QString, IlwisObjectCreatorModel *> _creators;

    QString createNumericDomain(const QVariantMap &parms);
    QString createItemDomain(const QVariantMap &parms);
    QString createGeoreference(const QVariantMap &parms);
    QString createProjectedCoordinateSystem(const QVariantMap &parms);
    QString createWorkflow(const QVariantMap &parms);
    QString createProjectedCoordinateSystemFromCode(const QVariantMap &parms);
    QString createProjectedCoordinateSystemFromBase(const QVariantMap &parms);
    QString createRasterCoverage(const QVariantMap &parms);
    QString createScript(const QVariantMap &parms);
    QString createModel(const QVariantMap &parms);
    QString createCombinationMatrix(const QVariantMap &parms);
    QString createBoundsOnlyCoordinateSystem(const QVariantMap &parms);
    void executeoperation(const QString &expr);
    QString createOperationScriptHeader(const QVariantMap &parms);
    Ilwis::OperationResource createOperationResource(const QString &url, const QVariantMap &parms);

};

#endif // OBJECTCREATOR_H
