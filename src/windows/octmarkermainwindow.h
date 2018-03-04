#ifndef OCTMARKERMAINWINDOW_H
#define OCTMARKERMAINWINDOW_H

#include<functional>

#include<QMainWindow>

#include"octmarkeractions.h"

class QLabel;
class QString;
class QAction;
class QFileDialog;
class QProgressBar;
class QUrl;
class QMenu;
class CVImageWidget;
class OptionColor;

class BScanMarkerWidget;
class DWSloImage;
class CScan;
class ScrollAreaPan;
class DWDebugOutput;

class PaintMarker;


class OCTMarkerMainWindow : public QMainWindow
{
	Q_OBJECT

	static void messageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg);

	void setupMenu();
	void setupStatusBar();

	void createMarkerToolbar();
	QAction* createColorOptionAction(OptionColor& opt, const QString& text);

	QDockWidget*          dwSloImage                  = nullptr;
	ScrollAreaPan*        bscanMarkerWidgetScrollArea = nullptr;
	BScanMarkerWidget*    bscanMarkerWidget           = nullptr;
	static DWDebugOutput* dwDebugOutput;

	PaintMarker* pmm = nullptr;

	QProgressBar* loadProgressBar  = nullptr;

	OctMarkerActions generalMarkerActions;
	QList<QAction*> markerActions;
	void generateMarkerActions();

	static void setMarkersFilters(QFileDialog& fd);

	virtual void closeEvent(QCloseEvent* e);

	void handleOpenUrl(const QUrl& url, bool singleInput);

	bool catchSaveError(std::function<void ()>& saveObj, std::string& errorStr, const QString& unknownErrorMessage);
	void showErrorDialog(bool isError, const std::string& errorMessage);

protected:
	virtual void dropEvent     (QDropEvent     * event);
	virtual void dragEnterEvent(QDragEnterEvent* event);
	virtual void dragLeaveEvent(QDragLeaveEvent* event);
	virtual void dragMoveEvent (QDragMoveEvent * event);

public:
	OCTMarkerMainWindow(bool loadLastFile = true);
	~OCTMarkerMainWindow();

	bool loadFile(const QString& filename);
	bool addFile(const QString& filename);
	std::size_t loadFolder(const QString& foldername, int numMaxRecursiv = 10);
	
	static void setMarkersStringList(QStringList& filters);

signals:
	void loadLastFile();

private slots:
	void newCscanLoaded();

	void loadFileStatusSlot(bool loading);
	void loadFileProgress(double frac);

public slots:
	virtual void showLoadImageDialog();
	virtual void showImportFromFolderDialog();

	virtual void showLoadMarkersDialog();
	virtual void showSaveMarkersDialog();

	virtual void saveMatlabBinCode();
	virtual void saveMatlabWriteBinCode();

	virtual void showSaveOctScanDialog();
	virtual void screenshot();
};


class OptionInt;
class SendInt : public QObject
{
Q_OBJECT
	int v;
public:
	SendInt(int value) : v(value) {}
	void connectOptions(OptionInt& option, QAction* action);
public slots:
	void recive(bool b = true) { if(b) send(v); }
signals:
	void send(int value);
};


#endif // OCTMARKERMAINWINDOW_H
