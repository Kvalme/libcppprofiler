#ifndef QCPPPROFWINDOW_H
#define QCPPPROFWINDOW_H

#include <QMainWindow>
#include <vector>

namespace Ui {
class QCppProfWindow;
}

class QCppProfWindow : public QMainWindow
{
	Q_OBJECT
	
public:
	explicit QCppProfWindow(QWidget *parent = 0);
	~QCppProfWindow();
	
private slots:
	void on_pushButton_released();
	void on_horizontalScrollBar_valueChanged(int value);
	void on_actionOpen_triggered();

	void on_scale_valueChanged(int value);

private:
	void parseFile(QString file);
	void addRect(double start, double end, int level, QString name);

	struct ProfData
	{
		double start;
		double end;
		int depth;
		QString name;
		std::vector<ProfData> submodules;
	};

	std::vector<ProfData> modules;

	void buildGraph(std::vector<ProfData> &mods);

	Ui::QCppProfWindow *ui;
};

#endif // QCPPPROFWINDOW_H
