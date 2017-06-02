#include "lecp.h"
#include <QtWidgets/QApplication>
#include <QtDebug>
#include <QFile>
#include <QTime>
#include <QTextStream>

//void myMessageOutput(QtMsgType type, const QMessageLogContext &, const QString & msg)
//{
//	QString txt;
//	switch (type) {
//	case QtDebugMsg:
//		txt = QString("Debug: %1").arg(msg);
//		break;
//	case QtWarningMsg:
//		txt = QString("Warning: %1").arg(msg);
//		break;
//	case QtCriticalMsg:
//		txt = QString("Critical: %1").arg(msg);
//		break;
//	case QtFatalMsg:
//		txt = QString("Fatal: %1").arg(msg);
//		break;
//	}
//	QFile outFile("log");
//	outFile.open(QIODevice::WriteOnly | QIODevice::Append);
//	QTextStream ts(&outFile);
//	ts << QTime::currentTime().toString() << txt << endl;
//}

int main(int argc, char *argv[])
{
	//qInstallMessageHandler(myMessageOutput);
	QApplication a(argc, argv);
	LECP w;
	w.show();

	return a.exec();
}