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


#ifndef QTLUACONSOLE_HH_
#define QTLUACONSOLE_HH_

#include <QTextEdit>
#include <QTextCursor>
#include <QMouseEvent>

namespace QtLua {

  /**
   * @short Qt console widget
   * @header QtLua/Console
   * @module {Base}
   *
   * This class provides a easy to use console widget for use in QtLua
   * based applications.
   *
   * This widget is a general purpose console widget with history and
   * completion capabilities.
   *
   * When used with a @ref QtLua::State lua interpreter object, it
   * only needs a few signals connections to get a working lua based
   * shell:
   * @example examples/cpp/console/console.cc:1
   */

class Console : public QTextEdit
{
  Q_OBJECT;

public:

  /** Create a console widget. */
  Console(QWidget *parent = 0);

  /** Set console prompt. */
  void set_prompt(QString p);

  /** Set history entry count. */
  inline void set_history_size(int size);

  /** Get current history content. */
  inline const QStringList & get_history() const;

  /** Set Qt regular expression used to extract text before cursor to
    * pass to completion signal.
    *
    * The default regexp @tt{[_.:a-zA-Z0-9]+$} is suited to extract
    * lua identifiers and table member access statements.
    */
  inline void set_completion_regexp(const QRegExp &regexp);

signals:

  /** Signal emited when text line is validated with enter key */
  void line_validate(const QString &str);

  /** Signal emited to query completion list.
   *
   * @param prefix text extracted before cursor.
   * @param list must be filled with completion matches by completion slot function.
   * @param cursor_offset may be decreased by completion slot function to move cursor backward on completed text. (only used on single match)
   */
  void get_completion_list(const QString &prefix, QStringList &list, int &cursor_offset);

public slots:

  /** Display text on the console */
  void print(const QString &str);

private:

  QTextCharFormat	_fmt_normal;
  QTextCharFormat	_fmt_completion;
  int			_complete_start;
  int			_prompt_start;
  int			_line_start;
  QString		_prompt;
  QStringList		_history;
  int			_history_ndx;
  int			_history_max;
  int			_cursor_pos;
  bool			_color_state;
  QRegExp		_complete_re;

  // Internal actions
  void action_key_complete();
  void action_key_enter();
  void action_history_up();
  void action_history_down();
  void display_prompt();
  void delete_completion_list();

  // Handle mouse events on console
  void mousePressEvent(QMouseEvent *e);
  void mouseReleaseEvent(QMouseEvent *e);
  void mouseDoubleClickEvent(QMouseEvent *e) { (void)e; }

  // Handle keypress
  void keyPressEvent(QKeyEvent * e);
};

/*
  Text color can be changed by writing #x where 'x' is a lower case
  letter code:

  c Qt::black  2   Black (#000000) 
  d Qt::white  3   White (#ffffff) 
  e Qt::darkGray  4   Dark gray (#808080) 
  f Qt::gray  5   Gray (#a0a0a4) 
  g Qt::lightGray  6   Light gray (#c0c0c0) 
  h Qt::red  7   Red (#ff0000) 
  i Qt::green  8   Green (#00ff00) 
  j Qt::blue  9   Blue (#0000ff) 
  k Qt::cyan  10   Cyan (#00ffff) 
  l Qt::magenta  11   Magenta (#ff00ff) 
  m Qt::yellow  12   Yellow (#ffff00) 
  n Qt::darkRed  13   Dark red (#800000) 
  o Qt::darkGreen  14   Dark green (#008000) 
  p Qt::darkBlue  15   Dark blue (#000080) 
  q Qt::darkCyan  16   Dark cyan (#008080) 
  r Qt::darkMagenta  17   Dark magenta (#800080) 
  s Qt::darkYellow  18   Dark yellow (#808000) 
  
*/

}

#endif


