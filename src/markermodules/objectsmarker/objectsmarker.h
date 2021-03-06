/*
 * Copyright (c) 2018 Kay Gawlik <kaydev@amarunet.de> <kay.gawlik@beuth-hochschule.de> <kay.gawlik@charite.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef OBJECTSMARKER_H
#define OBJECTSMARKER_H

#include "../bscanmarkerbase.h"

#include<vector>
#include<QPoint>
#include<QList>

#include"objectsmarkerfactory.h"

class QAction;
class QToolBar;
class QActionGroup;
class QWidget;

class ObjectsmarkerScene;
class RectItem;
class QGraphicsItem;
class WidgetObjectMarker;


class Objectsmarker : public BscanMarkerBase
{
	Q_OBJECT
	friend class ObjectsMarkerPTree;
public:
// 	typedef std::map<std::string, RectItem*> RectItems;
// 	typedef RectItems::value_type RectItemsTypes;


	Objectsmarker(OctMarkerManager* markerManager);
	~Objectsmarker();

	virtual bool drawBScan() const                         override { return true;  }

	virtual bool keyPressEvent    (QKeyEvent*  , BScanMarkerWidget*) override;

	virtual void saveState(boost::property_tree::ptree& markerTree)  override;
	virtual void loadState(boost::property_tree::ptree& markerTree)  override;


	virtual       QGraphicsScene* getGraphicsScene()       override;
	virtual const QGraphicsScene* getGraphicsScene() const override;

	QWidget* getWidget() override;

	virtual void drawMarker(QPainter&, BScanMarkerWidget*, const QRect& drawrect) const override;


// 	virtual RedrawRequest mouseMoveEvent   (QMouseEvent* event, BScanMarkerWidget* markerWidget) override;
// 	virtual RedrawRequest mousePressEvent  (QMouseEvent* event, BScanMarkerWidget* markerWidget) override;
// 	virtual RedrawRequest mouseReleaseEvent(QMouseEvent* event, BScanMarkerWidget* markerWidget) override;
// 	virtual void contextMenuEvent (QContextMenuEvent* /*event*/) {}


	virtual void setActBScan(std::size_t bscan)  override;
	virtual void newSeriesLoaded(const OctData::Series* series, boost::property_tree::ptree& markerTree) override;

private:
	void resetMarkerObjects(const OctData::Series* series);
	void removeAllItems();


	ObjectsmarkerScene* graphicsScene = nullptr;
	ObjectsmarkerFactory objectsfactory;


	void removeItems(const QList<QGraphicsItem*>& items);
// 	RectItems rectItems;

	std::vector<std::vector<RectItem*>> itemsList;
	std::size_t actBScanSceneNr = 0;

	WidgetObjectMarker* widget = nullptr;
};

#endif // OBJECTSMARKER_H
