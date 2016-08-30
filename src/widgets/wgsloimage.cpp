#include "wgsloimage.h"

#include "sloimagewidget.h"
#include <manager/markermanager.h>
#include <data_structure/programoptions.h>

#include <QResizeEvent>
#include <QToolBar>
#include <QAction>


WgSloImage::WgSloImage(MarkerManager& markerManger)
: QMainWindow(0)
, imageWidget(new SLOImageWidget(markerManger))
, markerManger(markerManger)
{

	imageWidget->setImageSize(size());
	setCentralWidget(imageWidget);

	QToolBar* bar = new QToolBar(this);

	QAction* showBScans = ProgramOptions::sloShowBscans.getAction();
	showBScans->setText(tr("show BScans"));
	showBScans->setIcon(QIcon(":/icons/layers.png"));
	connect(showBScans, &QAction::toggled, imageWidget, &SLOImageWidget::showBScans);
	bar->addAction(showBScans);


	QAction* showLabels = ProgramOptions::sloShowLabels.getAction();
	showLabels->setText(tr("show Labels"));
	showLabels->setIcon(QIcon(":/icons/tag_blue_edit.png"));
	connect(showLabels, &QAction::toggled, imageWidget, &SLOImageWidget::showLabels);
	bar->addAction(showLabels);


	setContextMenuPolicy(Qt::NoContextMenu);
	addToolBar(bar);
}


void WgSloImage::wheelEvent(QWheelEvent* wheelE)
{
	int deltaWheel = wheelE->delta();
	if(deltaWheel < 0)
		markerManger.previousBScan();
	else
		markerManger.nextBScan();
}


void WgSloImage::resizeEvent(QResizeEvent* event)
{
	imageWidget->setImageSize(event->size());
	QWidget::resizeEvent(event);
}
