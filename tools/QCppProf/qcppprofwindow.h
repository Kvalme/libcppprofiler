#ifndef QCPPPROFWINDOW_H
#define QCPPPROFWINDOW_H

#include <QMainWindow>
#include <vector>


class QProgressBar;
class QLabel;
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
	void on_horizontalScrollBar_valueChanged(int value);
	void on_actionOpen_triggered();

	void on_scale_valueChanged(int value);

    void on_timeScale_currentIndexChanged(int index);

    void updatePlot();

private:
	struct ProfData
	{
		double start;
		double end;
		int depth;
		QString name;
		QString shortName;
		std::list<ProfData> submodules;
	};

	void parseFile(QString fname, std::list<ProfData> &dat);
	void addRect(double start, double end, int level, QString name, QColor color);
    void setRange();
    void setPosition();


	std::list<std::list<ProfData>> modules;

	void buildGraph(std::list<ProfData> &mods, int offset, QColor color);

	Ui::QCppProfWindow *ui;
    QProgressBar *progressBar;
    QLabel *speedInfo;
    double oldTimeScale;
	int maxDepth;
};

#endif // QCPPPROFWINDOW_H
