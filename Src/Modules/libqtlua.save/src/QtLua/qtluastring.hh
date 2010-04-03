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

#ifndef QTLUASTRING_HH_
#define QTLUASTRING_HH_

#include <QString>
#include <QByteArray>

namespace QtLua {

  /**
   * @short Character string class
   * @header QtLua/String
   * @module {Base}
   *
   * Lua use 8 bits character strings so @ref QString is not suitable
   * for QtLua.
   *
   * This class is used as string object in the QtLua project. It is
   * based on @ref QByteArray with added conversion facilities.
   *
   * This class is also used as exceptions type for exceptions
   * associated with lua errors.
   */
  class String : public QByteArray
  {
  public:
    /** Create an empty string */
    inline String();
    /** Create a string from @tt{const char *} */
    inline String(const char *s);
    /** Create a string from @tt{const char *} with given length */
    inline String(const char *s, int size);
    /** Create a string from @ref QByteArray */
    inline String(const QByteArray &s);
    /** Copy constructor */
    inline String(const QString &s);
    /** Replace next @tt % character in string with given string  */
    inline String & arg(const String &arg);
    /** Replace next @tt % character in string with given integer  */
    inline String & arg(int arg);
    /** @tt{const char *} cast operator */
    inline operator const char * ();
  };

}

#endif

