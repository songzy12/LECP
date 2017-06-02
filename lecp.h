#ifndef LECP_H
#define LECP_H
#define showSpeedMax 10
#define showSpeedMin 1

#include <QtWidgets/QMainWindow>
#include <QtGui/QPainter>
#include <QtGui/QPen>

#include <qspinbox.h>
#include <qmessagebox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qdebug.h>
#include <qfiledialog.h>
#include <qslider.h>

#include "ui_lecp.h"
#include "Paint.h"
#include "lecp_doc.h"
#include "DisplayDCEL.h"

class LECP : public QMainWindow
{
	Q_OBJECT

public:
	LECP(QWidget *parent = 0);
	~LECP();

	
private:
	Ui::LECPClass ui;

	PaintWidget *paintWidget;

	//toolbar content:
	QCheckBox *sortComboBox,*vgComboBox,*chainComboBox,*dcelComboBox,*qComboBox,*lComboBox;
	QPushButton *startButton, *resetButton; // TODO: delete this two
	QSlider *speedSlider;
	QSpinBox *speedSpinBox,*pointSpinBox;

	QAction *openAction;

public:
	void createToolBar();
	void clearToolBar();
	Polygon* trans2Poly(int kernal_index);
public slots:
	//operationTB:slot
	void openFileSlot();
	void saveFileSlot();
	void resetPointsSlot();
	void randomPointsGenerationSlot();
	void finalResultShowSlot();
	void allPointsShowSlot();
	void singlePointShowSlot();

	//pointSelectTB ��ѡ��-������:slot
	void lastPointSlot();
	void nextPointSlot();

	//strartShowTB ��ʼ��ʾ-������:slot
	void startShowSlot();
	void resetShowSlot();

	//showContentTB ��ʾ����-������:slot
	void onSortSelected(int flag);
	void onVGSelected(int flag);
	void onChainSelected(int flag);

	//sortTB ����-������:slot
	void onDCELSelected(int flag);

	//qTB vg-������:slot
	void onQueueSelected(int flag);

	//lTB chain-������:slot
	void onLSelected(int flag);

	//showControlTB �ٶȿ���-������:slot
	void changeSpeedSlot(int newSpeed);

	//��֪��Ļ����ʾ��ÿ�����label(index)��ȡ�õ��Ӧ���ź�����index(start with 0)
	long long getSortedIndex(long long displayIndex);

	void polarAngleSortSlot();//����ÿ������㣬�õ��������е㰴�չ��ڸõ���м�������
	void showVisibilityGraphSlot();
	void showConvexChainSlot();
	void polarAngleSortDCELSlot();
	vector<LECP_Point*>  preprocessingPolarAngleSort();
	//DCEL ����
	void DCELAnimationSlot();
	void clearDCELAnimationSlot();
	void resetSlot();

public:
	Mesh *mesh;
	LECP_Doc *lecp_doc;
	Polygon *poly2show;
	int lastSelectedPoint;
	//show variable
	enum showMode { finalRes, allPoints, singlePoint };
	showMode currMode;
	bool showSort ;
	bool showVG ;
	bool showChain;
	bool showDCEL;
	bool showQ;
	bool showL;
	bool isStart ;
	bool isStop ;
	int showspeed;
};

#endif // LECP_H
