#include <QDebug>
#include <memory>
#include "kernel.h"
#include "ilwisdata.h"
#include "mastercatalog.h"
#include "iooptions.h"
#include "datadefinition.h"
#include "columndefinition.h"
#include "table.h"
#include "geometries.h"
#include "georeference.h"
#include "uicontextmodel.h"
#include "coveragelayermodel.h"
#include "coverage.h"
#include "layerinfoitem.h"
#include "layermanager.h"

using namespace Ilwis;

LayerManager::LayerManager(QObject *parent) :
    QObject(parent)
{
}

LayerManager::LayerManager(QObject *parent, UIContextModel *context) : QObject(parent), _uicontext(context)
{
    IlwisTypes metatype = itCOLLECTION | itCATALOGVIEW;
    Resource res("Global Layer", metatype);
    CoverageLayerModel * model = new CoverageLayerModel(_layers.size(), res,0, this);
    model->iconPath("layers.png");
    _layers.append(model);
}
void LayerManager::addVisualizationModel(CoverageLayerModel *newmodel)
{
    _layers.insert(1,newmodel);
}

void LayerManager::addDataSource(const QUrl &url, IlwisTypes tp, Ilwis::Geodrawer::DrawerInterface *drawer)
{
    try{
        if ( tp == itUNKNOWN)
            return;
        Resource resource = mastercatalog()->name2Resource(url.toString(),tp);
        if ( !resource.isValid())
            return;
        if ( _layers.size() > 0){
            ICoverage coverage(resource);
            if ( !coverage.isValid())
                return;
            if ( coverage->coordinateSystem()->isUnknown()) {// after the first layer, nothing with unknown can be added
                QString mes = QString("coordinate system 'unknown' not compatible with coordinate system of the layerview");
                kernel()->issues()->log(mes, IssueObject::itWarning);
                return;
            }
        }
        auto layer = new CoverageLayerModel(_layers.size(), resource, drawer, this);
        if  ( _layers.size() == 1)
            _layers.push_back(layer);
        else
            _layers.insert(1,new CoverageLayerModel(_layers.size(), resource, drawer, this));
        emit layerChanged();
    }
    catch(const ErrorObject& ){
    }
    catch(const std::exception& ex){
        kernel()->issues()->log(ex.what());
    }
}

bool LayerManager::zoomInMode() const
{
    return _zoomInMode;
}

void LayerManager::setZoomInMode(bool yesno)
{
    _zoomInMode = yesno;
}

bool LayerManager::hasSelectionDrawer() const
{
    return _hasSelectionDrawer;
}

void LayerManager::setHasSelectionDrawer(bool yesno)
{
    _hasSelectionDrawer = yesno;
}

void LayerManager::setScreenGeoReference(const IGeoReference &grf)
{
    _screenGrf = grf;
}

IGeoReference LayerManager::screenGrf() const
{
    return _screenGrf;
}

void LayerManager::layersView(LayersViewCommandInterface *view)
{
    _layersView = view;
}

QQmlListProperty<CoverageLayerModel> LayerManager::layers()
{
    return QQmlListProperty<CoverageLayerModel>(this, _layers);
}

CoverageLayerModel *LayerManager::layer(quint32 layerIndex){
    if ( layerIndex < _layers.size())
        return _layers[layerIndex];
    return 0;
}

QString LayerManager::layerInfo(const Coordinate &crdIn, const QString& attrName)
{
    std::vector<QString> texts;
    _layerInfoItems.clear();
    for(CoverageLayerModel *layer : _layers){
        if ( layer->object().isValid() && hasType(layer->object()->ilwisType(), itCOVERAGE)){
            ICoverage cov = layer->object().as<Coverage>();
            Coordinate crd = crdIn;
            QVariant varCsy = layer->drawer()->attributeOfDrawer("rootdrawer","coordinatesystem");
            if ( varCsy.isValid()){
                ICoordinateSystem csy = varCsy.value<ICoordinateSystem>();
                if ( csy != cov->coordinateSystem()){
                    crd = cov->coordinateSystem()->coord2coord(csy, crd);
                }
            }
            auto item = new LayerInfoItem("Layer",cov->name(), this);
            item->icon(layer->iconPath());
            item->layerHeader(true);
            _layerInfoItems.push_back(item);
            QVariant value = cov->coord2value(crd,attrName);
            if ( value.isValid()){
                QString txt;
                if ( attrName != "")
                    txt = layer->value2string(value,attrName);
                else {
                    QVariantMap vmap = value.value<QVariantMap>();
                    int activeAttIndex = layer->getActiveAttributeIndex();
                    if ( activeAttIndex == 0){ // this is the pseudo attribute representing the whole layer, not wanted here
                        activeAttIndex = 1;
                    }
                    QVariant activeAttr =  vmap[layer->visualAttributeByIndex(activeAttIndex)];
                    txt = layer->value2string(activeAttr, layer->visualAttributeByIndex(activeAttIndex));
                    auto end = vmap.end();
                    for(auto item = vmap.begin(); item != end; ++item){
                        _layerInfoItems.push_back(new LayerInfoItem(item.key(), item.value().toString(), this));
                    }
                    emit layerInfoItemsChanged();
                }
                texts.push_back(txt);

            }
        }

    }
    QString outtext;
    for(auto txt : texts){
        if ( outtext.size() != 0)
            outtext += "; ";
        outtext += txt;
    }
    if ( outtext == "")
        outtext = "?";
    return outtext;
}

void LayerManager::init()
{

}


QQmlListProperty<LayerInfoItem> LayerManager::layerInfoItems()
{
    return QQmlListProperty<LayerInfoItem>(this,_layerInfoItems);
}