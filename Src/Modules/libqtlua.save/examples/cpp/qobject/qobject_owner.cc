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

#include <QDialog>
#include <QApplication>

#include <QtLua/State>
#include <QtLua/Value>

int main(int argc, char *argv[])
{
  /* anchor 1 */
  QApplication app(argc, argv);
  QtLua::State state;

  QDialog *qd = new QDialog();
  // Assume C++ pointers to QDialog exist as it has no parents, do not take ownership
  state["dialog1"] = qd;

  // Explicitly take ownership of new QDialog
  state["dialog2"] = QtLua::Value(state, new QDialog(), true);

  // Reuse same wrapper as dialog1 and explicitly leave ownership
  state["dialog3"] = QtLua::Value(state, qd, false);

  // Invoke QDialog show() methods from lua
  state.exec_statements("dialog1:show(); dialog2:show();");

  app.exec();

  // Delete qd QObject, dialog1 and dialog3 now refer to an empty wrapper
  delete qd;
  // Delete wrapper and associated QObject
  state.exec_statements("dialog2 = nil");
  /* anchor end */

  return 0;
}

