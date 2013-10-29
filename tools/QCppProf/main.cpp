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

#include "qcppprofwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QCppProfWindow w;
	w.show();
	
	return a.exec();
}
