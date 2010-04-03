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

#ifndef QTLUAQTLIB_HH_
#define QTLUAQTLIB_HH_

#include <QFileDialog>

namespace QtLua {

  class State;

  void qtluaopen_qt(State &ls);

  /** QFileDialog widget with more usefull properties */
  class QFileDialog : public ::QFileDialog
  {
    Q_OBJECT;

    QString get_directory() const
    {
      return directory().absolutePath();
    }

    Q_PROPERTY(QString directory READ get_directory WRITE setDirectory);
    Q_PROPERTY(QStringList history READ history WRITE setHistory);
    Q_PROPERTY(QStringList selectedFiles READ selectedFiles);
#if QT_VERSION >= 0x040400
    Q_PROPERTY(QString selectedNameFilter READ selectedNameFilter);
    Q_PROPERTY(QStringList nameFilters READ nameFilters WRITE setNameFilters);
#else
    Q_PROPERTY(QString selectedNameFilter READ selectedFilter);
    Q_PROPERTY(QStringList nameFilters READ filters WRITE setFilters);
#endif
  };

}

#endif

