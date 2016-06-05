#ifndef SELECTION_H
#define SELECTION_H


namespace Ilwis {
namespace BaseOperations {

class SelectionRaster : public SelectionBase
{
public:
    SelectionRaster();
    SelectionRaster(quint64 metaid, const Ilwis::OperationExpression &expr);
    ~SelectionRaster();

    bool execute(ExecutionContext *ctx, SymbolTable& symTable);
    static Ilwis::OperationImplementation *create(quint64 metaid,const Ilwis::OperationExpression& expr);
    Ilwis::OperationImplementation::State prepare(ExecutionContext *ctx, const SymbolTable&);

    static quint64 createMetadata();
private:
    IIlwisObject _inputObj;
    IIlwisObject _outputObj;
    std::vector<qint32> _base;
    int _zvalue = iUNDEF;

    NEW_OPERATION(SelectionRaster)    ;


};
}
}
#endif // SELECTION_H
