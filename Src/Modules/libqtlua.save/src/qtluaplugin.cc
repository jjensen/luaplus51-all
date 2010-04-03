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


#include <QtLua/Plugin>
#include <QtLua/PluginInterface>
#include <QtLua/String>

#include "config.hh"

#define STR(n) STR_(n)
#define STR_(n) #n

namespace QtLua {

Plugin::Plugin(const String &filename)
  : _loader(QTLUA_REFNEW(Loader, filename))
{
  set_container(&_map);
  api<PluginInterface>()->register_members(*this);
}

Plugin::Loader::Loader(const String &filename)
  : QPluginLoader(filename)
{
  if (!load())
    throw String("Unable to load plugin '%': %")
      .arg(filename).arg(errorString());
}

Plugin::Loader::~Loader()
{
  unload();
}

Value Plugin::to_table(State &ls) const
{
  return Value(ls, _map);
}

const String & Plugin::get_name() const
{
  return api<PluginInterface>()->get_name();
}

const String & Plugin::get_description() const
{
  return api<PluginInterface>()->get_description();
}

String Plugin::get_value_str() const
{
  return get_name() + " : " + get_description();
}

const String & Plugin::get_plugin_ext()
{
  static const String s(STR(SHLIBEXT));
  return s;
}

void Plugin::completion_patch(String &path, String &entry, int &offset)
{
  entry += ".";
}

}

