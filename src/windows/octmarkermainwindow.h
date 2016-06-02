#ifndef OCTMARKERMAINWINDOW_H
#define OCTMARKERMAINWINDOW_H

#include <QMainWindow>

class DWSloImage;
class CScan;
class CVImageWidget;

class OCTMarkerMainWindow : public QMainWindow
{
	Q_OBJECT

	void setupMenu();

	DWSloImage*    dwSloImage  = nullptr;
	CVImageWidget* imageWidget = nullptr;
	CScan* cscan = nullptr;

	std::size_t bscanPos = 0;

	void showBScan();

public:
	OCTMarkerMainWindow();
	~OCTMarkerMainWindow();


public slots:

	virtual void showAboutDialog();
    virtual void showLoadImageDialog();

	virtual void nextBScan();
	virtual void previousBScan();
};

#endif // OCTMARKERMAINWINDOW_H
