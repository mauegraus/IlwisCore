#ifndef RASTERAPPLICATIONSMODULE_H
#define RASTERAPPLICATIONSMODULE_H

#include <QObject>
#include "module.h"
#include "kernel.h"
#include "factory.h"
#include "abstractfactory.h"

namespace Ilwis {
namespace BaseOperations {
class RasterOperationsModule : public Module
{
    Q_OBJECT
    Q_INTERFACES(Ilwis::Module)
public:
    explicit RasterOperationsModule(QObject *parent = 0);

    QString getInterfaceVersion() const;
    QString name() const;
    QString version() const;
    void prepare();

    Q_PLUGIN_METADATA(IID "n52.ilwis.rasteroperations" FILE  "baseoperations.json")

};
}
}

#endif // RASTERAPPLICATIONSMODULE_H
