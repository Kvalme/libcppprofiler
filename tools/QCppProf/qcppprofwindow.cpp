#include "qcppprofwindow.h"
#include "ui_qcppprofwindow.h"
#include "cppprofiler.h"
#include "QFileDialog"
#include "QMessageBox"

#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <stack>

double stm = -1, etm;


QCppProfWindow::QCppProfWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::QCppProfWindow)
{
	ui->setupUi(this);




	ui->PlotArea->xAxis->setRange(0, 102);
	ui->PlotArea->yAxis->setRange(0, 20);

}

QCppProfWindow::~QCppProfWindow()
{
	delete ui;
}

void QCppProfWindow::on_pushButton_released()
{
	// add the text label at the top:
	QCPItemText *textLabel = new QCPItemText(ui->PlotArea);
	ui->PlotArea->addItem(textLabel);
	textLabel->setPositionAlignment(Qt::AlignTop|Qt::AlignHCenter);
	textLabel->position->setType(QCPItemPosition::ptAxisRectRatio);
	textLabel->position->setCoords(0.5, 0); // place position at center/top of axis rect
	textLabel->setText("Text Item Demo");
	textLabel->setFont(QFont(font().family(), 16)); // make font a bit larger
	textLabel->setPen(QPen(Qt::black)); // show black border around text

	// add the arrow:
	QCPItemLine *arrow = new QCPItemLine(ui->PlotArea);
	ui->PlotArea->addItem(arrow);
	arrow->start->setParentAnchor(textLabel->bottom);
	arrow->end->setCoords(4, 1.6); // point to (4, 1.6) in x-y-plot coordinates
	arrow->setHead(QCPLineEnding::esSpikeArrow);

	QCPItemRect *rect = new QCPItemRect(ui->PlotArea);
	ui->PlotArea->addItem(rect);

	rect->topLeft->setType(QCPItemPosition::ptPlotCoords);
	rect->topLeft->setCoords(0.5, 0.5);
	rect->bottomRight->setType(QCPItemPosition::ptPlotCoords);
	rect->bottomRight->setCoords(1, 0);
	rect->setBrush(QBrush(Qt::black));
	rect->setPen(QPen(Qt::red));
	rect->setVisible(true);

	ui->PlotArea->replot();
}

void QCppProfWindow::on_horizontalScrollBar_valueChanged(int value)
{
/*	if (qAbs(ui->PlotArea->xAxis->range().center()-value/ui->horizontalScrollBar->maximum()) > 0.01) // if user is dragging plot, we don't want to replot twice
	  {*/

	double size = ui->PlotArea->xAxis->range().size();
	double pos = ui->PlotArea->xAxis->range().center();
	double rel = (double)value/1000000.;
	std::cerr<<"rel:"<<rel<<std::endl;
	std::cerr<<"Pos:"<<stm + rel * (etm - stm)<<std::endl;
	ui->PlotArea->xAxis->setRange(stm + rel * (etm - stm) , ui->PlotArea->xAxis->range().size(), Qt::AlignLeft);
		ui->PlotArea->replot();
//	  }
}

void QCppProfWindow::on_actionOpen_triggered()
{
	QString fname = QFileDialog::getOpenFileName(this, "Profile data", "", "Profiling data (*.cppprof)");
	parseFile(fname);
}

using namespace CPPProfiler;

void QCppProfWindow::parseFile(QString file)
{
	int inFile = open(file.toUtf8().data(), O_RDONLY);
	if (inFile < 0)
	{
		QMessageBox::critical(this, "Unable to parse file", QString("Unable to open file %1").arg(file));
		return;
	}


	Profiler::FileHeader header, header_orig;

	if (read(inFile, &header, sizeof(header)) != sizeof(header))return;
	if (memcmp(header_orig.MAGIC, header.MAGIC, sizeof(header.MAGIC)) != 0)
	{
		QMessageBox::critical(this, "Unable to parse file", QString("Invalid magic bytes"));
		::close(inFile);
	}

	int depth = 0;
	int maxDepth = 0;
	uint32_t end_time = 0;

	modules.clear();

	std::stack<std::vector<ProfData>*> write_location;
	write_location.push(&modules);

	Profiler::RECORD_TYPE recordType;
	while (read(inFile, &recordType, sizeof(recordType)) == sizeof(recordType))
	{
		switch (recordType)
		{
			case Profiler::RECORD_TYPE::DATA_DUMP:
			{
				uint64_t start, duration;
				if (read(inFile, &start, sizeof(start)) != sizeof(start))return;
				if (read(inFile, &duration, sizeof(duration)) != sizeof(duration))return;

				ProfData dta;
				dta.start = (double)(start - header.start_time);
				dta.end = (double)(start - header.start_time + duration);
				dta.depth = depth;
				dta.name = "Data dump";

				write_location.top()->push_back(dta);

				break;
			}
			case Profiler::RECORD_TYPE::INTERNAL_RECORD:
			{
				uint64_t start, end;
				if (read(inFile, &start, sizeof(start)) != sizeof(start))return;
				if (read(inFile, &end, sizeof(end)) != sizeof(end))return;

				ProfData dta;
				dta.start = (double)(start - header.start_time);
				dta.end = (double)(end - header.start_time);
				dta.depth = depth;
				dta.name = "Profiler internal";

				write_location.top()->push_back(dta);

				break;
			}
			case Profiler::RECORD_TYPE::MODULE_START:
			{
				uint64_t start;
				uint16_t name_length;
				char *name;
				if (read(inFile, &name_length, sizeof(name_length)) != sizeof(name_length))return;
				name = new char[name_length];
				if (read(inFile, name, name_length) != name_length)return;
				if (read(inFile, &start, sizeof(start)) != sizeof(start))return;

				ProfData dta;
				dta.start = (double)(start - header.start_time);
				dta.depth = depth;
				dta.name = name;

				if (stm <0) stm = dta.start;

				write_location.top()->push_back(dta);
				ProfData &d = write_location.top()->back();

				write_location.push(&d.submodules);
				depth++;

				break;
			}
			case Profiler::RECORD_TYPE::MODULE_END:
			{
				uint64_t start;
				if (read(inFile, &start, sizeof(start)) != sizeof(start))return;

				depth--;
				write_location.pop();
				ProfData &d = write_location.top()->back();
				d.end = (double)(start - header.start_time);

				etm = d.end;

				break;
			}
		}

		maxDepth = (depth > maxDepth)?depth:maxDepth;

	}

	buildGraph(modules);

//	ui->PlotArea->xAxis->setRange(stm, etm - stm);
	int pos = (etm - stm)/1000000;
	ui->scale->setValue(pos);


	ui->PlotArea->xAxis->setRange(stm, 1000000);
	ui->PlotArea->yAxis->setRange(0, maxDepth);
	ui->PlotArea->xAxis->setAutoTickLabels(true);
	ui->PlotArea->xAxis->setAutoTickCount(10);

	ui->PlotArea->replot();
}

void QCppProfWindow::buildGraph(std::vector<ProfData> &mods)
{
	for(ProfData &pd : mods)
	{
		addRect(pd.start, pd.end, pd.depth, pd.name);
		buildGraph(pd.submodules);
	}

}

void QCppProfWindow::addRect(double start, double end, int level, QString name)
{
	QCPItemRect *rect = new QCPItemRect(ui->PlotArea);
	ui->PlotArea->addItem(rect);

	rect->topLeft->setType(QCPItemPosition::ptPlotCoords);
	rect->topLeft->setCoords(start, level);
	rect->bottomRight->setType(QCPItemPosition::ptPlotCoords);
	rect->bottomRight->setCoords(end, level+1);
	rect->setBrush(QBrush(QColor(200, 200, 200, 128)));
	rect->setPen(QPen(Qt::red));

	if (end - start > 100000)
	{
		QCPItemText *text = new QCPItemText(ui->PlotArea);
		text->setParent(rect);
		text->setText(name);
	//	text->setTextAlignment(Qt::AlignBottom | Qt::AlignLeft);
		text->position->setCoords(start + (end - start)/2., level+0.5);
		text->position->setType(QCPItemPosition::ptPlotCoords);
		text->setRotation(-90);
	}

}

void QCppProfWindow::on_scale_valueChanged(int value)
{
	double size = ui->PlotArea->xAxis->range().size();
	double pos = ui->PlotArea->xAxis->range().center();
	double rel = (double)value/1000000.;
	double newSize = rel * (etm - stm);

	ui->PlotArea->xAxis->setRange(pos - newSize/2. , rel * (etm - stm), Qt::AlignLeft);
	ui->PlotArea->replot();
}
