#include "kernel.h"
#include "ilwisdata.h"
#include "workflow.h"
#include "analysispattern.h"
#include "applicationsetup.h"
#include "model.h"
#include "analysismodel.h"

using namespace Ilwis;

AnalysisModel::AnalysisModel(AnalysisPattern *pattern) : _analysis(pattern)
{

}

void AnalysisModel::suicide()
{
    delete this;
}

QString AnalysisModel::name() const{
    if ( _analysis)    {
       return _analysis->name();
    }
    return "";

}

QString AnalysisModel::longName() const
{
    if (resourceRef().hasProperty("longname"))
        return resourceRef()["longname"].toString();
    return "";
}

void AnalysisModel::longName(const QString &lname)
{
    resourceRef()["longname"]=lname;
}

Resource &AnalysisModel::resourceRef()
{
    if (_analysis && _analysis->attachedModel().isValid()) {
        return _analysis->attachedModel()->resourceRef();
    }
    throw ErrorObject(TR("trying to use uninitialized analysis model"));
}

const Resource &AnalysisModel::resourceRef() const
{
    if (_analysis && _analysis->attachedModel().isValid()) {
        return _analysis->attachedModel()->resourceRef();
    }
    throw ErrorObject(TR("trying to use uninitialized analysis model"));
}

QString AnalysisModel::mainPanel() const
{
    return _panelPath;
}


