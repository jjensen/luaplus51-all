
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

#include "treeview.hh"
							/* anchor 1 */
MainWindow::MainWindow()
  : QMainWindow()
{
  // Lua state
  state = new QtLua::State();

  // Create tree root node
  QtLua::ListItem::ptr root = QTLUA_REFNEW(QtLua::ListItem, );

  // Set as lua global
  (*state)["root"] = root;

  // Insert 2 new nodes
  QTLUA_REFNEW(QtLua::Item, "foo")->insert(root);
  QTLUA_REFNEW(QtLua::Item, "foo2")->insert(root);

  // Create Qt view widget and set model
  treeview = new QTreeView(0);
  setCentralWidget(treeview);
  model = new QtLua::ItemModel(root);
  treeview->setModel(model);

  // Rename node from lua script
  state->exec_statements("root.bar = root.foo2");
}
							/* anchor end */

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  MainWindow mainWin;

  mainWin.show();
  return app.exec();
}


