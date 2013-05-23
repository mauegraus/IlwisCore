#include <QString>
#include <QVector>
#include <QSqlQuery>
#include <QSqlError>
#include <functional>
#include <future>
#include "kernel.h"
#include "ilwis.h"
#include "angle.h"
#include "point.h"
#include "box.h"
#include "ilwisobject.h"
#include "ilwisdata.h"
#include "ellipsoid.h"
#include "geodeticdatum.h"
#include "projection.h"
#include "domain.h"
#include "numericrange.h"
#include "numericdomain.h"
#include "coordinatesystem.h"
#include "valuedefiner.h"
#include "connectorinterface.h"
#include "abstractfactory.h"
#include "connectorfactory.h"
#include "columndefinition.h"
#include "table.h"
#include "containerstatistics.h"
#include "coverage.h"
#include "georeference.h"
#include "simpelgeoreference.h"
#include "cornersgeoreference.h"
#include "grid.h"
#include "gridcoverage.h"
#include "gridinterpolator.h"
//#include "identity.h"
#include "OperationExpression.h"
#include "operationmetadata.h"
#include "operation.h"
#include "commandhandler.h"
#include "pixeliterator.h"
#include "blockiterator.h"
#include "operation.h"
#include "operationhelper.h"
#include "rasvalue.h"

using namespace Ilwis;
using namespace BaseOperations;

RasValue::RasValue()
{
}

RasValue::RasValue(quint64 metaid, const Ilwis::OperationExpression &expr) :
    OperationImplementation(metaid, expr)
{
}

bool RasValue::execute(ExecutionContext *ctx)
{
    if (_prepState == sNOTPREPARED)
        if((_prepState = prepare()) != sPREPARED)
            return false;
    double v = _inputGC->pix2value(_vox);
    if ( ctx) {
        ctx->_results.push_back(QVariant(v));
    }
    return true;
}

Ilwis::OperationImplementation *RasValue::create(quint64 metaid, const Ilwis::OperationExpression &expr)
{
        return new RasValue(metaid, expr);
}

Ilwis::OperationImplementation::State RasValue::prepare()
{
    if ( _expression.parameterCount() < 3 || _expression.parameterCount() > 4) {
        ERROR3(ERR_ILLEGAL_NUM_PARM3,"rasvalue","3 or 4",QString::number(_expression.parameterCount()));
        return sPREPAREFAILED;
    }

    QString gc = _expression.parm(0).value();
    if (!_inputGC.prepare(gc)) {
        ERROR2(ERR_COULD_NOT_LOAD_2,gc,"");
        return sPREPAREFAILED;
    }
    _vox = Voxel(_expression.parm(1).value().toInt(), _expression.parm(2).value().toInt());
    if ( _expression.parameterCount() == 4)
        _vox.z(_expression.parm(3).value().toInt());
    if (!_vox.isValid()) {
        ERROR2(ERR_INVALID_PROPERTY_FOR_2,"Pixel coordinate","rasvalue");
        return sPREPAREFAILED;
    }
    return sPREPARED;
}

quint64 RasValue::createMetadata()
{
    QString url = QString("ilwis://operations/rasvalue");
    Resource res(QUrl(url), itOPERATIONMETADATA);
    res.addProperty("namespace","ilwis");
    res.addProperty("longname","rasvalue");
    res.addProperty("syntax","rasvalue(inputgridcoverage,x,y,[,z])");
    res.addProperty("inparameters","3|4");
    res.addProperty("pin_1_type", itGRIDCOVERAGE);
    res.addProperty("pin_1_name", TR("input gridcoverage"));
    res.addProperty("pin_1_desc",TR("input gridcoverage with any domain"));
    res.addProperty("pin_2_type", itINT32);
    res.addProperty("pin_2_name", TR("column"));
    res.addProperty("pin_2_desc",TR("column number in the grid"));
    res.addProperty("pin_3_type", itINT32);
    res.addProperty("pin_3_name", TR("row"));
    res.addProperty("pin_3_desc",TR("row number in the grid"));
    res.addProperty("pin_4_type", itINT32);
    res.addProperty("pin_4_name", TR("band number"));
    res.addProperty("pin_4_desc",TR("band number in the grid, optional"));
    res.addProperty("outparameters",1);
    res.addProperty("pout_1_type", itDOUBLE);
    res.addProperty("pout_1_name", TR("value at the indicated location"));
    res.addProperty("pout_1_desc",TR(""));
    res.prepare();
    url += "=" + QString::number(res.id());
    res.setUrl(url);

    mastercatalog()->addItems({res});
    return res.id();
}
