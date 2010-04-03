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

#include <QtLua/String>
#include <QtLua/Function>
#include <QtLua/State>
#include <QtLua/Plugin>

namespace QtLua {

String Function::get_type_name() const
{
  return "QtLua::Function";
}

String Function::get_value_str() const
{
  return get_description();
}

String Function::get_help() const
{
  return "No help available for this function";
}

String Function::get_description() const
{
  return "";
}

void Function::register_(State &ls, const String &path)
{
  ls.set_global(path, Value(ls, *this));
}

void Function::register_(Plugin &plugin, const String &name)
{
  if (_loader_ref.valid()) {
    assert(_loader_ref->instance() == plugin._loader->instance());
  }

  _loader_ref = plugin._loader;
  plugin._map.insert(name, *this);
}

void Function::completion_patch(String &path, String &entry, int &offset)
{
  entry += "()";
  offset--;
}

void Function::ref_drop(int count)
{
  // Drop reference to plugin loader if statically defined Function
  // is not used anymore.
  if (count == 0)
    _loader_ref.invalidate();
}

}

