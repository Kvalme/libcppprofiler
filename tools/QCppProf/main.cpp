#include "qcppprofwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QCppProfWindow w;
	w.show();
	
	return a.exec();
}
