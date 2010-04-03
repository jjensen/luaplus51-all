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

// This example show how to use a QLinkedlistProxy object to access a QLinkedlist
// object from lua script.

#include <iostream>

#include <QLinkedList>

#include <QtLua/State>
#include <QtLua/QLinkedListProxy>

int main()
{
  try {
    typedef QLinkedList<QtLua::String> Container;

    // QLinkedlist we want to access from lua
    Container linkedlist;

    // Linkedlist proxy which provides access to our QLinkedlist from lua
    QtLua::QLinkedListProxy<Container> proxy(linkedlist);

    QtLua::State state;
    state.openlib(QtLua::QtLuaLib);

    // Declare a lua global variable using our QLinkedlist proxy
    state["linkedlist"] = proxy;

    // Set a value in QLinkedlist directly
    linkedlist << "foo" << "bar" << "FOO";

    // Iterate through QLinkedlist from lua script
    state.exec_statements("for key, value in each(linkedlist) do print(key, value) end");

  } catch (QtLua::String &e) {
    std::cerr << e.constData() << std::endl;
  }

}

