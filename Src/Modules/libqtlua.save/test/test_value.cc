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

#include <QtLua/State>
#include <QtLua/Value>

using namespace QtLua;

int main()
{
  try {

    {
      QtLua::State ls;

      ls["nill"] = Value(ls);
      ls["btrue"] = Value(ls, Value::True);
      ls["bfalse"] = Value(ls, Value::False);
      ls["int"] = Value(ls, 0);
      ls["double"] = Value(ls, 1.0f);
      ls["str1"] = Value(ls, "hello");
      ls["str2"] = Value(ls, String("hello"));

      Value::List res = ls.exec_statements("return nill, btrue, bfalse, int, double, str1, str2");

      ASSERT(res.size() == 7);
      ASSERT(res[0].type() == Value::TNil);
      ASSERT(res[1].to_boolean() == true);
      ASSERT(res[2].to_boolean() == false);
      ASSERT(res[3].to_number() == 0.0f);
      ASSERT(res[4].to_number() == 1.0f);
      ASSERT(res[5].to_string() == "hello");
      ASSERT(res[6].to_string() == "hello");
    }

    {
      QtLua::State ls;

      ls["btrue"] = Value::True;
      ls["bfalse"] = Value::False;
      ls["int"] = 0;
      ls["double"] = 1.0f;
      ls["str1"] = "hello";
      ls["str2"] = String("hello");

      Value::List res = ls.exec_statements("return btrue, bfalse, int, double, str1, str2");

      ASSERT(res.size() == 6);
      ASSERT(res[0].to_boolean() == true);
      ASSERT(res[1].to_boolean() == false);
      ASSERT(res[2].to_number() == 0.0f);
      ASSERT(res[3].to_number() == 1.0f);
      ASSERT(res[4].to_string() == "hello");
      ASSERT(res[5].to_string() == "hello");
    }

    {
      QtLua::State ls;

      ls["foo"] = 1.0f;
      ls["bar"] = ls["foo"];

      Value::List res = ls.exec_statements("return bar");

      ASSERT(res.size() == 1);
      ASSERT(res[0].to_boolean() == true);
    }

    {
      QtLua::State ls;

      ls.exec_statements("var=\"foo\"; tbl={}; tbl.var=\"bar\"");

      ASSERT(ls.get_global("var").to_string() == "foo");
      ASSERT(ls.get_global("tbl.var").to_string() == "bar");
    }

    {
      QtLua::State ls;

      ls.set_global("var", Value(ls, "foo"));
      ls.set_global("tbl.var", Value(ls, "bar"));

      ASSERT(ls.exec_statements("return var").at(0).to_string() == "foo");
      ASSERT(ls.exec_statements("return tbl.var").at(0).to_string() == "bar");
    }


    {
      QtLua::State ls;

      ls.openlib(MathLib);

      QtLua::Value num(ls, 3.14159f);
      QtLua::Value func = ls.get_global("math.cos");

      ASSERT(func.type_name() == "lua::function");
      ASSERT(func(num).at(0).to_number() + 1.0f < 0.001f);
    }

  } catch (QtLua::String &e) {
    std::cout << e.constData() << std::endl;
    ASSERT(0);
  }

  return 0;
}

