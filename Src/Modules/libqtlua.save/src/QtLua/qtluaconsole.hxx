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


#ifndef QTLUACONSOLE_HXX_
#define QTLUACONSOLE_HXX_

namespace QtLua {

  /** Set history entry count */
  inline void Console::set_history_size(int size)
  {
    _history_max = size;
  }

  /** Get current history */
  inline const QStringList & Console::get_history() const
  {
    return _history; 
  }

  void Console::set_completion_regexp(const QRegExp &regexp)
  {
    _complete_re = regexp;
  }

}

#endif

