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
