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
    void parseFile(QString fname);
	void addRect(double start, double end, int level, QString name);
    void setRange();
    void setPosition();

	struct ProfData
	{
		double start;
		double end;
		int depth;
		QString name;
        QString shortName;
		std::vector<ProfData> submodules;
	};

	std::vector<ProfData> modules;

	void buildGraph(std::vector<ProfData> &mods);

	Ui::QCppProfWindow *ui;
    QProgressBar *progressBar;
    QLabel *speedInfo;
    double oldTimeScale;
};

#endif // QCPPPROFWINDOW_H
