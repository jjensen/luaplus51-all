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

// This example show how to use a VectorProxy object to access a
// QVector object from lua script. QVector store ref counted pointer to
// user defined lua UserData object.

#include <iostream>

#include <QVector>

#include <QtLua/State>
#include <QtLua/QVectorProxy>
#include <QtLua/UserData>

struct MyClass : public QtLua::UserData
{
  QTLUA_REFTYPE(MyClass);

};

int main()
{
  try {
    typedef QVector<MyClass::ptr> Container;

    // QVector we want to access from lua
    Container vector(2);

    // Vector proxy which provides access to our QVector from lua
    QtLua::QVectorProxy<Container> proxy(vector);

    QtLua::State state;
    state.openlib(QtLua::QtLuaLib);

    // Declare a lua global variable using our vector proxy
    state["vector"] = proxy;

    // Set a ref counted pointer to newly allocated object in QVector directly
    vector[0] = QTLUA_REFNEW(MyClass, );

    // Read/Write in QVector from lua using the proxy object
    state.exec_statements("vector[2] = vector[1]");

    // Read back pointer in QVector inserted from lua script
    std::cout << vector[1].ptr() << std::endl;

    // Iterate through QVector from lua script
    state.exec_statements("for key, value in each(vector) do print(key, value) end");

  } catch (QtLua::String &e) {
    std::cerr << e.constData() << std::endl;
  }

}

