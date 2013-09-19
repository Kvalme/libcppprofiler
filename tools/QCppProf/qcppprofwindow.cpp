#include "qcppprofwindow.h"
#include "ui_qcppprofwindow.h"
#include "cppprofiler.h"
#include "QFileDialog"
#include "QMessageBox"
#include <QProgressBar>
#include <QTime>

#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <stack>
#include <sys/types.h>
#include <sys/stat.h>

double stm = -1, etm;


QCppProfWindow::QCppProfWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::QCppProfWindow)
{
	ui->setupUi(this);

    oldTimeScale = 1;

	ui->PlotArea->xAxis->setRange(0, 102);
	ui->PlotArea->yAxis->setRange(0, 20);

    ui->timeScale->clear();
    ui->timeScale->addItem("ns", QVariant(1));
    ui->timeScale->addItem("us", QVariant(1e3));
    ui->timeScale->addItem("ms", QVariant(1e6));
    ui->timeScale->addItem("s", QVariant(1e9));
    ui->timeScale->setCurrentIndex(0);

    progressBar = new QProgressBar;
    speedInfo = new QLabel("0/0 (0 kb/s)");
    ui->statusBar->addPermanentWidget(progressBar, 1);
    ui->statusBar->addPermanentWidget(speedInfo);

    ui->PlotArea->setInteraction(QCP::iRangeDrag);
    ui->PlotArea->setInteraction(QCP::iRangeZoom);
    ui->PlotArea->axisRect(0)->setRangeDragAxes(ui->PlotArea->xAxis, 0);
    ui->PlotArea->axisRect(0)->setRangeZoomAxes(ui->PlotArea->xAxis, 0);


    connect(ui->PlotArea, SIGNAL(beforeReplot()), this, SLOT(updatePlot()));

}

QCppProfWindow::~QCppProfWindow()
{
	delete ui;
}

void QCppProfWindow::on_horizontalScrollBar_valueChanged(int value)
{
    setPosition();
}

void QCppProfWindow::on_actionOpen_triggered()
{
	QString fname = QFileDialog::getOpenFileName(this, "Profile data", "", "Profiling data (*.cppprof)");
	parseFile(fname);
}

using namespace CPPProfiler;

void QCppProfWindow::parseFile(QString fname)
{
    QFile file(fname);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this, "Unable to parse file", QString("Unable to open file %1").arg(fname));
        return;
    }

    unsigned char *data = file.map(0, file.size());
    unsigned char *data_pointer = data;
    if (!data)
    {
        QMessageBox::critical(this, "Unable to map file", QString("Unable to map file %1").arg(fname));
        return;
    }

    int readBytes = 0;
    int size = file.size();
    //Get file size
    progressBar->setMaximum(size);

	Profiler::FileHeader header, header_orig;

    if (size - readBytes < sizeof(header))
    {
        QMessageBox::critical(this, "Unable to parse file", QString("No header"));
    }
    memcpy(&header, data_pointer, sizeof(header));
    readBytes+= sizeof(header);

/*	if (memcmp(header_orig.MAGIC, header.MAGIC, sizeof(header.MAGIC)) != 0)
	{
		QMessageBox::critical(this, "Unable to parse file", QString("Invalid magic bytes"));
		::close(inFile);
    }*/

	int depth = 0;
	int maxDepth = 0;
	uint32_t end_time = 0;
    int lastReadBytes = 0;

	modules.clear();

	std::stack<std::vector<ProfData>*> write_location;
	write_location.push(&modules);
    QTime time;
    time.start();
	Profiler::RECORD_TYPE recordType;
    while (readBytes < size)
	{
        memcpy(&recordType, data_pointer+readBytes, sizeof(recordType));
        readBytes+= sizeof(recordType);
		switch (recordType)
		{
			case Profiler::RECORD_TYPE::DATA_DUMP:
			{
				uint64_t start, duration;
                memcpy(&start, data_pointer+readBytes, sizeof(start));
                readBytes+= sizeof(start);
                memcpy(&duration, data_pointer+readBytes, sizeof(duration));
                readBytes+= sizeof(duration);

				ProfData dta;
                dta.start = (double)(start - header.start_time)/*/1000.*/;
                dta.end = (double)(start - header.start_time + duration)/*/1000.*/;
				dta.depth = depth;
				dta.name = "Data dump";

				write_location.top()->push_back(dta);

				break;
			}
			case Profiler::RECORD_TYPE::INTERNAL_RECORD:
			{
				uint64_t start, end;

                memcpy(&start, data_pointer+readBytes, sizeof(start));
                readBytes+= sizeof(start);
                memcpy(&end, data_pointer+readBytes, sizeof(end));
                readBytes+= sizeof(end);

				ProfData dta;
                dta.start = (double)(start - header.start_time)/*/1000.*/;
                dta.end = (double)(end - header.start_time)/*/1000.*/;
				dta.depth = depth;
				dta.name = "Profiler internal";

				write_location.top()->push_back(dta);

				break;
			}
			case Profiler::RECORD_TYPE::MODULE_START:
			{
				uint64_t start;
				uint16_t name_length;

                memcpy(&name_length, data_pointer+readBytes, sizeof(name_length));
                readBytes+= sizeof(name_length);
                char *name;
                name = new char[name_length];
                memcpy(name, data_pointer + readBytes, name_length);
                readBytes+= name_length;

                memcpy(&start, data_pointer+readBytes, sizeof(start));
                readBytes+= sizeof(start);

				ProfData dta;
                dta.start = (double)(start - header.start_time)/*/1000.*/;
				dta.depth = depth;
				dta.name = name;
                if (dta.name.indexOf('(') == -1) dta.shortName = dta.name;
                else dta.shortName = dta.name.section('(', 0, 0).section(' ', -1, -1);

				if (stm <0) stm = dta.start;

				write_location.top()->push_back(dta);
				ProfData &d = write_location.top()->back();

				write_location.push(&d.submodules);
				depth++;

                delete[] name;

				break;
			}
			case Profiler::RECORD_TYPE::MODULE_END:
			{
				uint64_t start;
                memcpy(&start, data_pointer+readBytes, sizeof(start));
                readBytes+= sizeof(start);

				depth--;
				write_location.pop();
				ProfData &d = write_location.top()->back();
                d.end = (double)(start - header.start_time)/*/1000.*/;

				etm = d.end;

				break;
			}
		}

		maxDepth = (depth > maxDepth)?depth:maxDepth;

        if (time.elapsed() > 100)
        {
            progressBar->setValue(readBytes);
            double speed = (double)(readBytes - lastReadBytes) / ((double)time.elapsed()/1000.);
            speed/=1024;
            speedInfo->setText(QString("%1 kb/%2 kb (%3 kb/s)").arg(readBytes/1024.).arg(progressBar->maximum()/1024.).arg(speed));
            QCoreApplication::processEvents();
            time.restart();
            lastReadBytes = readBytes;
        }
	}

    progressBar->setValue(readBytes);
    double speed = (double)(readBytes - lastReadBytes) / ((double)time.elapsed()/1000.);
    speed/=1024;
    speedInfo->setText(QString("%1 kb/%2 kb (%3 kb/s)").arg(readBytes/1024.).arg(progressBar->maximum()/1024.).arg(speed));
    QCoreApplication::processEvents();
    time.restart();
    lastReadBytes = readBytes;

	buildGraph(modules);

//	ui->PlotArea->xAxis->setRange(stm, etm - stm);
    //ui->scale->setValue(1);

    ui->PlotArea->yAxis->setRange(0, maxDepth+1);
    ui->PlotArea->xAxis->setLabel(QString("time, %1").arg(ui->timeScale->currentText()));
    ui->PlotArea->xAxis->setAutoTickLabels(true);
    ui->PlotArea->xAxis->setAutoTickCount(10);

    setRange();
    setPosition();

	ui->PlotArea->replot();
}

void QCppProfWindow::buildGraph(std::vector<ProfData> &mods)
{
    double left = ui->PlotArea->xAxis->range().lower;
    double right = ui->PlotArea->xAxis->range().upper;

//    std::cerr<<"Item count: "<<ui->PlotArea->itemCount()<<std::endl;
//    std::cerr<<"Removed items count:"<<ui->PlotArea->clearItems()<<std::endl;

    double range = ui->PlotArea->xAxis->range().size();
//    std::cerr<<"Range: "<<range<<std::endl;

    double currentScale = ui->timeScale->itemData(ui->timeScale->currentIndex()).toDouble();

	for(ProfData &pd : mods)
	{
        if ((pd.start/currentScale < left && pd.end/currentScale < left) ||
                (pd.start/currentScale > right)) continue;

        addRect(pd.start/currentScale, pd.end/currentScale, pd.depth, pd.shortName);
        buildGraph(pd.submodules);
	}

}

void QCppProfWindow::addRect(double start, double end, int level, QString name)
{
    if ((ui->PlotArea->width() * ((end - start) / ui->PlotArea->xAxis->range().size())) < 1) return;

	QCPItemRect *rect = new QCPItemRect(ui->PlotArea);
	ui->PlotArea->addItem(rect);

    rect->topLeft->setType(QCPItemPosition::ptPlotCoords);
    rect->topLeft->setCoords(start, level + 1);
	rect->bottomRight->setType(QCPItemPosition::ptPlotCoords);
    rect->bottomRight->setCoords(end, level);
	rect->setBrush(QBrush(QColor(200, 200, 200, 128)));
	rect->setPen(QPen(Qt::red));


    if ((ui->PlotArea->width() * ((end - start) / ui->PlotArea->xAxis->range().size())) > 50)
	{
		QCPItemText *text = new QCPItemText(ui->PlotArea);
		text->setParent(rect);
        text->setText(QString("%1\n(%2 %3)").arg(name).arg(end - start).arg(ui->timeScale->currentText()));
	//	text->setTextAlignment(Qt::AlignBottom | Qt::AlignLeft);
        text->position->setParentAnchor(rect->top);
        text->setPositionAlignment(Qt::AlignHCenter | Qt::AlignTop);
//		text->position->setCoords(start + (end - start)/2., level+0.5);
//		text->position->setType(QCPItemPosition::ptPlotCoords);
//		text->setRotation(-90);
	}

}

void QCppProfWindow::on_scale_valueChanged(int value)
{
    setRange();
}

void QCppProfWindow::on_timeScale_currentIndexChanged(int index)
{
    setRange();
}

void QCppProfWindow::updatePlot()
{
    ui->PlotArea->clearItems();
    buildGraph(modules);
    ui->PlotArea->replot();
}

void QCppProfWindow::setPosition()
{
    double timeScale = ui->timeScale->itemData(ui->timeScale->currentIndex()).toDouble();
    double size = ui->PlotArea->xAxis->range().size();
    double rel = (double)ui->horizontalScrollBar->value()/(double)ui->horizontalScrollBar->maximum();

    ui->PlotArea->xAxis->setRange(/*stm + */(rel * (etm - stm))/timeScale, size/*timeScale*/, Qt::AlignLeft);

    updatePlot();
}

void QCppProfWindow::setRange()
{
    double curPos = ui->PlotArea->xAxis->range().center() * oldTimeScale;
    double timeScale = ui->timeScale->itemData(ui->timeScale->currentIndex()).toDouble();
    double newRange = /*timeScale * */ui->scale->value();
    oldTimeScale = timeScale;

    ui->PlotArea->xAxis->setRange(curPos/timeScale, newRange, Qt::AlignCenter);
    ui->PlotArea->xAxis->setLabel(QString("time, %1").arg(ui->timeScale->currentText()));

    //update position slider
    double rel = (curPos/timeScale - newRange/2.)/((etm-stm)/timeScale);
    double pos = rel * (double)ui->horizontalScrollBar->maximum();

    ui->horizontalScrollBar->setValue(pos);

    updatePlot();
}


