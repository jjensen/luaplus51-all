/*
    This file is part of LibQtLua.

    LibQtLua is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    LibQtLua is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with LibQtLua.  If not, see <http://www.gnu.org/licenses/>.

    Copyright (C) 2008, Alexandre Becoulet <alexandre.becoulet@free.fr>

*/

#ifndef MAINWINDOW_HH_
#define MAINWINDOW_HH_

#include <QMainWindow>

#include <QtLua/State>
#include <QtLua/Console>

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow();

private:
  QtLua::State *state;
  QtLua::Console *console;
};

#endif

