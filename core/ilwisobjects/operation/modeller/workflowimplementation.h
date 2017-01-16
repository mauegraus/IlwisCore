#ifndef WORKFLOWIMPL_H
#define WORKFLOWIMPL_H

#include "kernel_global.h"

namespace Ilwis {

class Workflow;
typedef IlwisData<Workflow> IWorkflow;


class KERNELSHARED_EXPORT WorkflowImplementation : public OperationImplementation
{
public:
    const QString UNRESOLVED_PARAMETER = "?_unresolved_?";

    WorkflowImplementation();
    WorkflowImplementation(quint64 metaid, const Ilwis::OperationExpression &expr);

    bool execute(ExecutionContext *ctx,SymbolTable& symTable);
    static Ilwis::OperationImplementation *create(quint64 metaid,const Ilwis::OperationExpression& expr);
    Ilwis::OperationImplementation::State prepare(ExecutionContext *ctx, const SymbolTable &);



private:
    IWorkflow _workflow;


};
}

#endif // WORKFLOWIMPL_H