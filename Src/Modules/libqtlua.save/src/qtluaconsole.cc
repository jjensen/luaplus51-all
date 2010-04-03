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


#include <QKeyEvent>
#include <QTextCursor>
#include <QColor>
#include <QScrollBar>
#include <QAbstractSlider>

#include <QtLua/Console>

namespace QtLua {

Console::Console(QWidget *parent)
  : QTextEdit(parent),
    _complete_re("[_.:a-zA-Z0-9]+$")
{
  _fmt_normal.setFontFamily("courier");
  _fmt_normal.setFontFixedPitch(true);
  _fmt_normal.setFontItalic(false);
  _fmt_normal.setFontPointSize(12);

  _fmt_completion = _fmt_normal;
  _fmt_completion.setFontItalic(true);

  setCurrentCharFormat(_fmt_normal);

  setWordWrapMode(QTextOption::WrapAnywhere);
  setContextMenuPolicy(Qt::NoContextMenu);
  set_prompt("$");
  display_prompt();
  _history_ndx = 0;
  _history_max = 100;
  _history.append("");
  _color_state = 0;
}

void Console::set_prompt(QString p)
{
  _prompt = p;
}

void Console::display_prompt()
{
  QTextCursor	tc;

  tc = textCursor();
  _complete_start = _prompt_start = tc.position();

  setTextColor(Qt::blue);
  tc.insertText(_prompt);

  setTextColor(Qt::black);
  tc = textCursor();
  tc.insertText(" ");

  _line_start = tc.position();
}

void Console::action_history_up()
{
  if (_history_ndx == 0)
    return;

  QTextCursor	tc = textCursor();

  tc.setPosition(_line_start, QTextCursor::MoveAnchor);
  tc.movePosition(QTextCursor::End, QTextCursor::KeepAnchor, 1);

  _history.replace(_history_ndx, tc.selectedText());

  tc.insertText(_history[--_history_ndx]);
}

void Console::action_history_down()
{
  if (_history_ndx + 1 >= _history.size())
    return;

  QTextCursor	tc = textCursor();

  tc.setPosition(_line_start, QTextCursor::MoveAnchor);
  tc.movePosition(QTextCursor::End, QTextCursor::KeepAnchor, 1);

  _history.replace(_history_ndx, tc.selectedText());

  tc.insertText(_history[++_history_ndx]);
}

void Console::action_key_complete()
{
  QStringList list;

  // get text chunk to complete
  QTextCursor	tc = textCursor();
  tc.setPosition(_line_start, QTextCursor::KeepAnchor);
  QString prefix = tc.selectedText();
  int idx = prefix.indexOf(_complete_re);

  if (idx < 0)
    {
      delete_completion_list();
      return;
    }

  int offset = 0;
  // get completion candidates list
  emit get_completion_list(prefix.mid(idx), list, offset);

  switch (list.count())
    {
      //////////// no candidate
    case 0:
      delete_completion_list();
      return;

      //////////// single candidate
    case 1: {
      QTextCursor	tc = textCursor();

      // insert
      tc.setPosition(_line_start + idx, QTextCursor::KeepAnchor);
      tc.insertText(list[0]);

      // move cursor if needed
      if (offset)
	{
	  tc.setPosition(tc.position() + offset, QTextCursor::MoveAnchor);
	  setTextCursor(tc);
	}
      delete_completion_list();
      break;
    }

      //////////// multiple candidates
    default: {
      unsigned int i;

      // find common prefix
      for (i = 0; ; i++)
	{
	  QChar c;

	  foreach(QString str, list)
	    {
	      if (i == str.size())
		goto break2;
	      if (c.isNull())
		c = str[i];
	      else if (str[i] != c)
		goto break2;
	    }
	}

    break2:

      // insert common prefix
      if (i)
	{
	  QTextCursor	tc = textCursor();

	  tc.setPosition(_line_start + idx, QTextCursor::KeepAnchor);
	  tc.insertText(list[0].left(i));
	}

      // print list
      QTextCursor	tc = textCursor();
      tc.setCharFormat(_fmt_completion); // FIXME
      tc.setPosition(_complete_start, QTextCursor::MoveAnchor);
      tc.setPosition(_prompt_start, QTextCursor::KeepAnchor);

      foreach(QString str, list)
	tc.insertText(str + "\n");

      _line_start += tc.position() - _prompt_start;
      _prompt_start = tc.position();
    }
    }
}

void Console::action_key_enter()
{
  QTextCursor	tc = textCursor();

  // select function line
  tc.setPosition(_line_start, QTextCursor::MoveAnchor);
  tc.movePosition(QTextCursor::End, QTextCursor::KeepAnchor, 1);

  QString	line = tc.selectedText().trimmed();

  // strip _history if too long
  while (_history.size() > _history_max)
    _history.removeFirst();

  // skip empty strings
  if (!line.trimmed().isEmpty())
    {
      // detect duplicate in _history
      if (_history.size() > 1)
	{
	  int	prev = _history.lastIndexOf(line, _history.size() - 2);

	  if (prev >= 0)
	    _history.removeAt(prev);
	}

      // set _history entry
      _history.replace(_history.size() - 1, line);

      _history.append("");
    }

  _history_ndx = _history.size() - 1;

  // move visible cursor to end
  tc.clearSelection();
  setTextCursor(tc);
  // new line
  append("");

  if (!line.trimmed().isEmpty())
    {
      emit line_validate(line);
    }

  delete_completion_list();
}

void Console::delete_completion_list()
{
  if (_complete_start != _prompt_start)
    {
      QTextCursor	tc = textCursor();

      tc.setPosition(_complete_start, QTextCursor::MoveAnchor);
      tc.setPosition(_prompt_start, QTextCursor::KeepAnchor);
      tc.deleteChar();

      _line_start += tc.position() - _prompt_start;
      _prompt_start = tc.position();
    }
}

void Console::keyPressEvent(QKeyEvent * e)
{
  if (e->modifiers() & Qt::ControlModifier)
    {
      switch (e->key())
	{

	case Qt::Key_C: {
	  QTextCursor	tc = textCursor();

	  tc.movePosition(QTextCursor::End, QTextCursor::MoveAnchor, 1);
	  tc.insertText("\n");
	  setTextCursor(tc);
	  display_prompt();
	} break;

	case Qt::Key_Y:
	case Qt::Key_V:
	  paste();
	  break;
	}

      ensureCursorVisible();
    }
  else
    {
	
      if (e->key() >= Qt::Key_Space  &&  e->key() <= Qt::Key_AsciiTilde)
	{
	  // Let normal ascii keys through
	  QTextEdit::keyPressEvent(e);
	  ensureCursorVisible();
	  return;
	}
    }

  if (e->modifiers() == Qt::NoModifier)
    {
      switch (e->key())
	{
	case Qt::Key_End:
	case Qt::Key_Delete:
	case Qt::Key_Right:
	  QTextEdit::keyPressEvent(e);
	  break;

	case Qt::Key_Return:
	case Qt::Key_Enter:
	  action_key_enter();
	  display_prompt();
	  break;

	case Qt::Key_Tab:
	  action_key_complete();
	  break;

	case Qt::Key_Left:
	case Qt::Key_Backspace:
	  if (textCursor().position() > _line_start)
	    QTextEdit::keyPressEvent(e);
	  break;

	case Qt::Key_Home: {
	  QTextCursor	tc = textCursor();

	  tc.setPosition(_line_start, QTextCursor::MoveAnchor);
	  setTextCursor(tc);
	} break;

	case Qt::Key_Up:
	  action_history_up();
	  break;

	case Qt::Key_Down:
	  action_history_down();
	  break;

	case Qt::Key_PageUp:
	  verticalScrollBar()->triggerAction(QAbstractSlider::SliderPageStepSub);
	  return;

	case Qt::Key_PageDown:
	  verticalScrollBar()->triggerAction(QAbstractSlider::SliderPageStepAdd);
	  return;
	}

      ensureCursorVisible();
      return;
    }
}

void Console::mousePressEvent(QMouseEvent *e)
{
  QTextCursor		tc = textCursor();

  if (e->button() & Qt::LeftButton)
    {
      // make readonly while using mouse
      _cursor_pos = tc.position();
      setReadOnly(true);
      QTextEdit::mousePressEvent(e);
    }

  if (e->button() & Qt::MidButton)
    {
      paste();
    }
}

void Console::mouseReleaseEvent(QMouseEvent *e)
{
  if ((e->button() & Qt::LeftButton))
    {
      QTextCursor	tc = textCursor();

      QTextEdit::mouseReleaseEvent(e);

      // copy selection to clipboard
      copy();

      // restore cursor position and remove readonly
      tc.setPosition(_cursor_pos, QTextCursor::MoveAnchor);
      setReadOnly(false);
      setTextCursor(tc);
    }
}

void Console::print(const QString &str)
{
  int first = 0;
  int last;
  QRegExp rx("\\0033\\[(\\d*)m");

  while ((last = str.indexOf(rx, first)) >= 0)
    {
      if (last > first)
	insertPlainText(str.mid(first, last - first));
      first = last + rx.matchedLength();

      setTextColor((Qt::GlobalColor)(rx.cap(1).toUInt()));
    }

  insertPlainText(str.mid(first, str.size() - first));
}

}

