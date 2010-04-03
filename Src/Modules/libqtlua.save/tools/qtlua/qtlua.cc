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

#include <QApplication>
#include <QFile>
#include <QDialog>

#include <QtLua/State>
#include <QtLua/Console>

int main(int argc, char *argv[])
{
  try {
    QApplication app(argc, argv);
    QStringList args = app.arguments();

    bool interactive = argc == 1;
    bool execute = interactive;

    QtLua::State state;
    state.openlib(QtLua::AllLibs);

    state["app"] = QtLua::Value(state, &app, false);

    for (unsigned int i = 1; i < argc; i++)
      {
	QByteArray arg(argv[i]);

	if (arg[0] == '-')
	  {
	    // option
	    if (arg == "--interactive" || arg == "-i")
	      {
		execute = interactive = true;
	      }
	    else
	      {
		std::cout
		  << "usage: qtlua [options] luafiles ..." << std::endl
		  << "  -i --interactive    show a lua console dialog" << std::endl;
	      }
	  }
	else
	  {
	    // lua chunk file
	    QFile file(argv[i]);

	    if (!file.open(QIODevice::ReadOnly))
	      throw QtLua::String("Unable to open `%' file.").arg(argv[i]);

	    execute = true;
	    state.exec_chunk(file);
	  }
      }

    if (interactive)
      {
	QtLua::Console *console = new QtLua::Console();

	QObject::connect(console, SIGNAL(line_validate(const QString&)),
			 &state, SLOT(exec(const QString&)));

	QObject::connect(console, SIGNAL(get_completion_list(const QString &, QStringList &, int &)),
		&state, SLOT(fill_completion_list(const QString &, QStringList &, int &)));

	QObject::connect(&state, SIGNAL(output(const QString&)),
		console, SLOT(print(const QString&)));

	console->show();
      }

    if (execute)
      app.exec();

  } catch (QtLua::String &e) {
    std::cerr << e.constData() << std::endl;
  }
}

