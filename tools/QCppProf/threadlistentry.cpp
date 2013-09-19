#include "threadlistentry.h"
#include "ui_threadlistentry.h"

#include <QColorDialog>

ThreadListEntry::ThreadListEntry(int tid, QString text, QColor color, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ThreadListEntry),
	id(tid),
	t_text(text),
	t_color(color)
{
	ui->setupUi(this);

	ui->threadName->setText(t_text);
	QIcon icon;
	QPixmap pix(32, 32);
	pix.fill(color);
	icon.addPixmap(pix);
	ui->colorSelection->setIcon(icon);
}

ThreadListEntry::~ThreadListEntry()
{
	delete ui;
}

void ThreadListEntry::on_colorSelection_released()
{
	t_color = QColorDialog::getColor(t_color, this, "Choose thread color", QColorDialog::ShowAlphaChannel);
	QIcon icon;
	QPixmap pix(32, 32);
	pix.fill(t_color);
	icon.addPixmap(pix);
	ui->colorSelection->setIcon(icon);

	emit changed();
}

bool ThreadListEntry::isThreadEnabled()
{
	return ui->threadState->isChecked();
}

void ThreadListEntry::on_threadState_stateChanged(int arg1)
{
	emit changed();
}
