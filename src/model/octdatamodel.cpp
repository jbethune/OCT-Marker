#include "octdatamodel.h"

#include <manager/markerdatamanager.h>


OctDataModel::OctDataModel()
{
}


OctDataModel::~OctDataModel()
{
	for(const OctSeriesItem* file : filelist)
		delete file;
}


QVariant OctDataModel::data(const QModelIndex& index, int role) const
{
	if(!index.isValid())
		return QVariant();

	if(static_cast<std::size_t>(index.row()) >= filelist.size())
		return QVariant();

	if(role == Qt::DisplayRole)
		return filelist.at(index.row())->getFilename();
	else
		return QVariant();
}


QVariant OctDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(role != Qt::DisplayRole)
		return QVariant();

	if(orientation == Qt::Horizontal)
		return QString("Column %1").arg(section);
	else
		return QString("Row %1").arg(section);
}


bool OctDataModel::addFile(QString filename)
{
	for(const OctSeriesItem* file : filelist)
	{
		if(file->sameFile(filename))
			return false;
	}

	int position = filelist.size();
	
	beginInsertRows(QModelIndex(), position, position);
	filelist.push_back(new OctSeriesItem(filename));
	endInsertRows();
	
	return true;
}

void OctDataModel::slotClicked(QModelIndex index)
{
	if(!index.isValid())
		return;
	
	std::size_t row = static_cast<std::size_t>(index.row());
	if(row >= filelist.size())
		return;
	
	OctSeriesItem* file = filelist.at(row);
	MarkerDataManager::getInstance().openFile(file->getFilename());
}

void OctDataModel::slotDoubleClicked(QModelIndex index)
{
	if(!index.isValid())
		return;
	
	std::size_t row = static_cast<std::size_t>(index.row());
	if(row >= filelist.size())
		return;
	
	OctSeriesItem* file = filelist.at(row);
	
	
	qDebug("file doubleclicked: %s", file->getFilename().toStdString().c_str());
}


