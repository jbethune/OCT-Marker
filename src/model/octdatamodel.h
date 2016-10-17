#ifndef OCTDATAMODEL_H
#define OCTDATAMODEL_H

#include <QVariant>
#include <QModelIndex>

#include <QString>
#include <vector>


class OctSeriesItem
{
	QString filename;
public:
	OctSeriesItem(const QString& filename) : filename(filename)   { }
	
	const QString& getFilename()       const                        { return filename; }
	bool sameFile(const QString& file) const                        { return filename == file; }
};


class OctDataModel : public QAbstractListModel
{
	Q_OBJECT

	std::vector<OctSeriesItem*> filelist;

	OctDataModel();
	virtual ~OctDataModel();

public:
	static OctDataModel& getInstance()                              { static OctDataModel instance; return instance;}
	
	
	int rowCount(const QModelIndex& /*parent*/ = QModelIndex()) const
	                                                                { return filelist.size(); }
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	

private slots:
	
public slots:
	bool addFile(QString filename);
	
    void slotClicked(QModelIndex index);
    void slotDoubleClicked(QModelIndex index);
};

#endif // OCTDATAMODEL_H

