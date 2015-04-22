/****************************************************************************
 **                                                                        **
 **  QCppProf - easy to use UI for cppprofiler                             **
 **  Copyright (C) 2013 Denis Biryukov <denis.birukov@gmail.com>           **
 **                                                                        **
 **  This program is free software: you can redistribute it and/or modify  **
 **  it under the terms of the GNU General Public License as published by  **
 **  the Free Software Foundation, either version 3 of the License, or     **
 **  (at your option) any later version.                                   **
 **                                                                        **
 **  This program is distributed in the hope that it will be useful,       **
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
 **  GNU General Public License for more details.                          **
 **                                                                        **
 **  You should have received a copy of the GNU General Public License     **
 **  along with this program.  If not, see http://www.gnu.org/licenses/.   **
 **                                                                        **
 ****************************************************************************
 **           Author: Denis Biryukov                                       **
 **  Website/Contact: http://www.blog.nsws.ru/                             **
 ****************************************************************************/

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
    void selectedRectChanged(bool selected);

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
