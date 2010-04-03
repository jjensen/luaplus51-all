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

#include "test.hh"
#include "test_qobject_arg.hh"

int main()
{
  try {
  {
    QtLua::State ls;

    MyObjectUD *myobj = new MyObjectUD();

    ls.exec_statements("function f(obj, ud) v = ud; end");

    ASSERT(ls["f"].connect(myobj, "ud_arg(Ref<UserData>)"));

    ASSERT(ls["v"].type() == Value::TNil);

    myobj->send(QTLUA_REFNEW(MyData, 18));

    ASSERT(ls["v"].type() == Value::TUserData);
    ASSERT(ls["v"][0].to_number() == 18);

    ASSERT(ls["f"].disconnect(myobj, "ud_arg(Ref<UserData>)"));

    ls["o"] = myobj;

    ASSERT(!myobj->_ud.valid());
    ls.exec_statements("o:ud_slot(v)");
    ASSERT(myobj->_ud.dynamiccast<MyData>()->_data == 18);
  }

  {
    QtLua::State ls;

    MyObjectQO *myobj = new MyObjectQO();

    ls.exec_statements("function f(obj, qo) v = qo; end");

    ASSERT(ls["f"].connect(myobj, "qo_arg(QObject*)"));

    ASSERT(ls["v"].type() == Value::TNil);

    QObject *qo = new QObject();
    qo->setObjectName("qo");
    myobj->send(qo);

    ASSERT(ls["v"].type() == Value::TUserData);
    ASSERT(ls["v"]["objectName"].to_string() == "qo");

    //    ASSERT(ls["f"].disconnect(myobj, "qo_arg(Ref<UserData>)"));

    ls["o"] = myobj;

    ASSERT(!myobj->_qo);
    ls.exec_statements("o:qo_slot(v)");
    ASSERT(myobj->_qo == qo);
  }

  } catch (QtLua::String &e) {
    std::cout << e.constData() << std::endl;
    ASSERT(0);
  }

  return 0;
}

