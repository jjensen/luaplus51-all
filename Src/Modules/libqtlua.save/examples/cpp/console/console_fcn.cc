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

#include <QApplication>

#include "console.hh"

#include <QtLua/Function>

MainWindow::MainWindow()
  : QMainWindow()
{
  state = new QtLua::State();
  console = new QtLua::Console();

  setCentralWidget(console);

  connect(console, SIGNAL(line_validate(const QString&)),
	  state, SLOT(exec(const QString&)));

  connect(console, SIGNAL(get_completion_list(const QString &, QStringList &, int &)),
          state, SLOT(fill_completion_list(const QString &, QStringList &, int &)));

  connect(state, SIGNAL(output(const QString&)),
	  console, SLOT(print(const QString&)));

  // Add a function

  static class Fcn : public QtLua::Function
  {
    QtLua::Value::List meta_call(QtLua::State &ls, const QtLua::Value::List &args)
    {
      // This function excepts at least 1 Number argument 
      meta_call_check_args(args, 1, 0, QtLua::Value::TNumber);

      return QtLua::Value::List();
    }

    QtLua::String get_description() const
    {
      return "Useless function";
    }

    QtLua::String get_help() const
    {
      return "Use this function to perform no operation.";
    }

  } fcn;

  fcn.register_(*state, "fcn");

}

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  MainWindow mainWin;

  mainWin.show();
  return app.exec();
}

