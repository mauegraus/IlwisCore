#include <functional>
#include <future>
#include "kernel.h"
#include "ilwisdata.h"
#include "symboltable.h"
#include "domain.h"
#include "numericdomain.h"
#include "datadefinition.h"
#include "columndefinition.h"
#include "table.h"
#include "basetable.h"
#include "flattable.h"
#include "coverage.h"
#include "rastercoverage.h"
#include "featurecoverage.h"
#include "operationhelper.h"
#include "pixeliterator.h"
#include "featureiterator.h"
#include "feature.h"
#include "operationhelpergrid.h"
#include "operationhelperfeatures.h"
#include "symboltable.h"
#include "internaldatabaseconnection.h"
#include "operationExpression.h"
#include "operationmetadata.h"
#include "commandhandler.h"
#include "operation.h"
#include "numericoperation.h"
#include "tranquilizer.h"
#include "columnjoin.h"

using namespace Ilwis;
using namespace BaseOperations;

REGISTER_OPERATION(ColumnJoin)

ColumnJoin::ColumnJoin()
{

}

ColumnJoin::ColumnJoin(quint64 metaid, const Ilwis::OperationExpression &expr) : NumericOperation(metaid, expr)
{

}

OperationImplementation *ColumnJoin::create(quint64 metaid, const Ilwis::OperationExpression &expr)
{
    return new ColumnJoin(metaid, expr);
}


void ColumnJoin::createTable(const QString &tmpTable, const ITable table)
{
    int count = table->columnCount();
    createSQL.append("CREATE TABLE ");
    createSQL.append(tmpTable);
    createSQL.append("(");
    for(int i = 0; i < count; i++){
        ColumnDefinition def = table->columndefinition(i);
        IDomain dmColumn = def.datadef().domain<>();
        IlwisTypes valueType = def.datadef().domain<>()->valueType();
        QString type = "";
        if(valueType == itINTEGER){
            type = "INTEGER";
        }else if(valueType == itDOUBLE){
            type = "DOUBLE";
        }else if(valueType == itBOOL){
            type = "BOOLEAN";
        }else{
            type = "TEXT";
        }

        createSQL.append(def.name()+" "+type);
        if(i < count-1){
            createSQL.append(",");
        }
    }
    createSQL.append(");");
}

void ColumnJoin::deleteTable(const QString &tmpTable)
{
    deleteSQL.append("DROP TABLE IF EXISTS ");
    deleteSQL.append(tmpTable);
}

void ColumnJoin::addInsertChangedDataToTempTableStmt(const QString &tmpTable, const ITable table)
{
    QString sqlBuilder;
    sqlBuilder.append(" INSERT INTO ").append(tmpTable).append(" ( ");
    sqlBuilder.append(columnNamesCommaSeparated(table));
    sqlBuilder.append(" ) VALUES ");
    for (int i = 0 ; i < table->recordCount() ; i++) {
        Record record = table->record(i);
        if (record.isChanged()) {

            // create value tuples
            sqlBuilder.append(" ( ");
            for (int j = 0 ; j < record.columnCount() ; j++) {
                ColumnDefinition coldef = table->columndefinition(j);
                QVariant value = record.cell(j);
                sqlBuilder.append(createInsertValueString(value, coldef));
                sqlBuilder.append(", ");
            }
            sqlBuilder = trimAndRemoveLastCharacter(sqlBuilder);
            sqlBuilder.append(" ), ");
        }
    }
    sqlBuilder = trimAndRemoveLastCharacter(sqlBuilder);
    sqlBuilder.append(" ; ");
    insertSQL.append(sqlBuilder);
}

QString ColumnJoin::columnNamesCommaSeparated(const ITable table) const
{
    QString columns;
    for (int i = 0 ; i < table->columnCount() ; i++) {
        ColumnDefinition coldef = table->columndefinition(i);
        columns.append(coldef.name());
        columns.append(", ");
    }
    return trimAndRemoveLastCharacter(columns);
}

QString ColumnJoin::createInsertValueString(QVariant value, const ColumnDefinition &coldef) const {
    IDomain domain = coldef.datadef().domain<>();
    if (hasType(domain->valueType(),itINTEGER)) {
        return QString::number(value.toInt());

    } else if (hasType(domain->valueType(),itDOUBLE | itFLOAT)) {
        return QString::number(value.toDouble());

    } else if (hasType(domain->valueType(),itTHEMATICITEM | itNAMEDITEM | itINDEXEDITEM | itNUMERICITEM | itTIMEITEM)) {
        QString res;
        res.append("'");
        res.append(domain->impliedValue(value).toString());
        res.append("'");
        return res;

    } else if (hasType(domain->valueType(), itDATETIME)) {
        if ( QString(value.typeName()).compare("Ilwis::Time") != 0){
            ERROR2(ERR_COULD_NOT_CONVERT_2,value.toString(), "time");
            Time time(0,0,0,0,0);
            return time.toString(itDATE);
        } else{
            Time time = value.value<Ilwis::Time>();
            return time.toString(itDATETIME);
        }
    } else if (hasType(domain->valueType(),itSTRING)){
        QString res;
        //res.append("'");
        if (domain->name() == "wkttext") {
            res.append(QString("st_geomfromtext('%1')").arg(value.toString()));

        } else {
            res.append(QString("'%1'").arg(value.toString()));

        }
        //res.append("'");
        return res;

    } else {
        ERROR0("Could not determine data type.");
        return QString("");
    }
}

QString ColumnJoin::trimAndRemoveLastCharacter(const QString &string) const
{
    QString trimmed = string.trimmed();
    return trimmed.left(trimmed.length() - 1);
}

bool ColumnJoin::execute(ExecutionContext *ctx, SymbolTable &symTable)
{
    if (_prepState == sNOTPREPARED)
        if((_prepState = prepare(ctx, symTable)) != sPREPARED)
            return false;

    boost::container::flat_map<QString, int> columns;
    for(int i=0; i < _outputTable->columnCount(); ++i){
        columns[_outputTable->columndefinition(i).name()] = i;
    }

    InternalDatabaseConnection conn;
    deleteTable(_objectname);
    bool deleteBaseTable = conn.exec(deleteSQL);
    deleteSQL = "";
    deleteTable(_joinTable);

    bool deleteJoinTable = conn.exec(deleteSQL);

    createTable(_objectname,_baseTable);
    bool createBaseTable = conn.exec(createSQL);
    if ( !createBaseTable){
        kernel()->issues()->log(conn.lastError().text());
        return false ;
    }
    createSQL = "";
    createTable(_joinTable,_inputTable);
    bool createJoinTable = conn.exec(createSQL);
    addInsertChangedDataToTempTableStmt(_objectname,_baseTable);
    bool insert1 = conn.exec(insertSQL);
    insertSQL = "";
    addInsertChangedDataToTempTableStmt(_joinTable,_inputTable);
    bool insert2 = conn.exec(insertSQL);

    selectSQL.append("SELECT * FROM ");

    selectSQL.append(_objectname);
    selectSQL.append(" INNER JOIN "+_joinTable+" ON ");
    selectSQL.append(_objectname);
    selectSQL.append(".");
    selectSQL.append(_primaryKeyColumn);
    selectSQL.append(" = ");
    selectSQL.append(_joinTable);
    selectSQL.append(".");
    selectSQL.append(_foreignKeyColumn);
    //std::cout<<selectSQL.toStdString()<<std::endl;

    InternalDatabaseConnection db;
    int count = 0;
    std::vector<QVariant> outdata(_outputTable->columnCount());
    bool queryok;
    if((queryok=db.exec(selectSQL))){
        int row = 0;
        while (db.next()){
            QSqlRecord rec = db.record();
            for(int i=0; i<rec.count(); ++i){
                QString fieldname = rec.fieldName(i);
                int index = columns[fieldname];
                outdata[index] = rec.value(i);
            }
            _outputTable->record(row,outdata);
            ++row;
        }
    }
    if ( !queryok){
        kernel()->issues()->log(db.lastError().text());
        return false ;
    }

    if ( _outputTable.isValid()) {
        IlwisTypes outputtype = itTABLE;
        if ( _outputCoverage.isValid()){
            _outputCoverage->setAttributes(_outputTable);
            if ( outputtype == itRASTER){
                IRasterCoverage raster = _outputCoverage.as<RasterCoverage>();
                IRasterCoverage inputraster = _inputCoverage.as<RasterCoverage>();
                raster->copyBinary(inputraster,0);
                QVariant var;
                var.setValue<IRasterCoverage>(raster);
                ctx->setOutput(symTable,var, raster->name(),itRASTER,raster->resource());
            }else {
                IFeatureCoverage features = _outputCoverage.as<FeatureCoverage>();
                IFeatureCoverage inputfeatures = _inputCoverage.as<FeatureCoverage>();

                int count = 0;
                for(const SPFeatureI& feature : inputfeatures){
                    auto newFeature = features->newFeatureFrom(feature);
                    auto datarecord = _outputTable->record(count++);
                    newFeature->record(datarecord);
                }
                QVariant var;
                var.setValue<IFeatureCoverage>(features);
                ctx->setOutput(symTable,var, features->name(),itFEATURE,features->resource());
            }

        }else {
            QVariant var;
            var.setValue<ITable>(_outputTable);
            ctx->setOutput(symTable,var, _outputTable->name(),itTABLE,_outputTable->resource(),_outputColumn);
        }
        return true;
    }
    return false;
}

OperationImplementation::State ColumnJoin::prepare(ExecutionContext *ctx, const SymbolTable &)
{
    QString baseTable = _expression.parm(0).value();
    QUrl dr(baseTable);
    _objectname = dr.fileName().split(".",QString::SkipEmptyParts).at(0);
    Resource res = mastercatalog()->name2Resource(baseTable, itTABLE | itCOVERAGE);
    if ( !res.isValid()){
        kernel()->issues()->log(QString(TR("%1 is not a valid resource").arg(baseTable)));
        return sPREPAREFAILED;
    }
    if ( hasType(res.ilwisType(), itCOVERAGE)){
        ICoverage cov(res);
        if (!cov.isValid() ){
            kernel()->issues()->log(QString(TR("%1 is not a valid resource").arg(baseTable)));
            return sPREPAREFAILED;
        }
        _inputCoverage = cov;
        _baseTable = _inputCoverage->attributeTable();
        if ( !_baseTable.isValid()){
            kernel()->issues()->log(QString(TR("%1 has no attribute table").arg(baseTable)));
            return sPREPAREFAILED;
        }

    }else {
        if (!_baseTable.prepare(baseTable)){
            ERROR2(ERR_COULD_NOT_LOAD_2,baseTable,"");
            return sPREPAREFAILED;
        }
    }
   _objectname = _objectname+"_base_tbl";
    _primaryKeyColumn = _expression.parm(1).value();

    QString inputTable = _expression.parm(2).value();
    QUrl url(inputTable);
    _joinTable = url.fileName().split(".",QString::SkipEmptyParts).at(0);
    _joinTable = _joinTable+"_join_tbl";
    if (!_inputTable.prepare(inputTable)) {
        ERROR2(ERR_COULD_NOT_LOAD_2,inputTable,"");
        return sPREPAREFAILED;
    }

    ColumnDefinition def1 = _baseTable->columndefinition(_primaryKeyColumn);
    IDomain dmColumn1 = def1.datadef().domain<>();
    IlwisTypes valueType1 = def1.datadef().domain<>()->valueType();

    for(int j = 0; j < _inputTable->columnCount(); j++){
        ColumnDefinition def2 = _inputTable->columndefinition(j);
        IDomain dmColumn2 = def2.datadef().domain<>();
        IlwisTypes valueType2 = def2.datadef().domain<>()->valueType();
        if((dmColumn1 == dmColumn2) && (valueType1 == valueType2) ){

            _foreignKeyColumn = def2.name();
        }
    }
    const Parameter& parm1 = _expression.parm(3);
    _inputColumn = parm1.value();
    bool ok = true;
    if ( hasType(parm1.valuetype(),itNUMBER)){
        _number1 = parm1.value().toDouble(&ok);
    }
    _outputColumn = _inputColumn;
    QString outName = _expression.parm(0, false).value();
    if (baseTable != outName) {
        if (outName == sUNDEF){
            IFlatTable  ftable;
            ftable.prepare();
            _outputTable = ftable;
        } else if(!_outputTable.prepare(outName)) {// output table doesnt need to exists
            _outputTable.prepare(QString("ilwis://internalcatalog/%1").arg(outName), _baseTable->ilwisType());
            _outputTable->name(outName);
        }
    } else{
        _outputTable = _baseTable;
    }
    for(int i=0; i < _baseTable->columnCount(); ++i){
        _outputTable->addColumn(_baseTable->columndefinition(i));
    }
    for(int i=0; i < _inputTable->columnCount(); ++i){
        _outputTable->addColumn(_inputTable->columndefinition(i));
    }

    if ( _inputCoverage.isValid()){
        if ( _inputCoverage->ilwisType() == itRASTER){
             IRasterCoverage raster = OperationHelperRaster::initialize(_inputCoverage,itRASTER, itDOMAIN|itGEOREF|itCOORDSYSTEM|itRASTERSIZE|itENVELOPE);
             _outputCoverage = raster;
        }else {
            IFeatureCoverage features = OperationHelperFeatures::initialize(_inputCoverage, itFEATURE, itDOMAIN|itCOORDSYSTEM|itENVELOPE);
            _outputCoverage = features;
        }
    }
    return sPREPARED;
}

quint64 ColumnJoin::createMetadata()
{
    OperationResource operation({"ilwis://operations/columnjoin"});
    operation.setLongName("Column Join");
    operation.setSyntax("columnjoin(base-table,column-name|number,input-table, column-name|number)");
    operation.setDescription(TR("Join a base table with another table who share a common domain"));
    operation.setInParameterCount({4});
    operation.addInParameter(0,itTABLE|itCOVERAGE, TR("base-table"),TR("Base table from which where join is to do be done"));
    operation.addInParameter(1,itSTRING | itNUMBER , TR("input column name or number"),TR("column with a numerical domain or number"));
    operation.addInParameter(2,itTABLE, TR("input-table"),TR("input table-column from which the input column will be chosen"));
    operation.addInParameter(3,itSTRING | itNUMBER, TR("input column name or number"),TR("column with a numerical domain or number"));
    operation.setOutParameterCount({1});
    operation.addOutParameter(0,itTABLE|itCOVERAGE, TR("output table"));
    operation.setKeywords("table");
    mastercatalog()->addItems({operation});
    return operation.id();
}

