Overview
========

lqt is a [Lua](http://www.lua.org) binding to the [Qt framework](http://qt.nokia.com).
It is an automated binding generated from the Qt headers, and covers almost
all classes and methods from supported Qt modules.

For more info, check the documentation, [mailing list](http://groups.google.com/group/lqt-bindings) or contact the authors:

 * Michal Kottman michal.kottman@gmail.com
 * Mauro Iazzi mauro.iazzi@gmail.com
 * Peter Kümmel syntheticpp@gmx.net

Features
--------

* automatically generated from Qt headers - remains up-to-date even with as Qt progresses
* supported modules: QtCore, QtGui, QtNetwork, QtXml, QtXmlPatterns, QtWebKit, QtOpenGL, QtSql, QtSvg, QtUiTools
* high API coverage - only a minimum of methods are not available
* C++/Qt features available:
  * method overloads
  * virtual methods (including abstract methods)
  * multiple inheritance
  * you can store additional Lua values in userdata - they act like Lua tables
  * several overloaded operators are supported
  * chosen templated classes are available, like `QList<QString>`
  * signal/slot mechanism - you can define custom slots in Lua
  * `QObject` derived objects are automatically cast to correct type thanks to Qt metaobject system
  * implicit conversion - i.e. write Lua strings where QString is expected, or numbers instead of QVariant
* optional memory management - you can let the Lua GC destroy objects, or let Qt parent/child management do the work

History
-------

## lqt 0.9

* Public beta, most issues and API stabilized

Building lqt
------------

Pre-compiled Windows binaries of lqt compiled against Qt 4.7 compatible
with Lua for Windows [are available](https://github.com/mkottman/lqt/downloads),
on other systems you need:

* Lua 5.1
* [CMake](http://www.cmake.org/cmake/resources/software.html)
* Qt and headers, on Ubuntu you need to install the `libqt4-dev` package

You can get the latest source of lqt from https://github.com/mkottman/lqt .
When you have the sources, create an out-of-source build directory
(where the binaries will be built, I often use `build` or `/dev/shm`).
Then, use CMake to generate the Makefile and run `make` as usual:

    mkdir build; cd build
    cmake ..
    make -j4 # use parallel build with your number of cores/processors

The generated Lua binding libraries are created in the `lib` directory,
you can copy them to your `LUA_CPATH`.

Usage
-----

A quick example of "Hello World!" button with signal/slot handling:

    require 'qtcore'
    require 'qtgui'

    app = QApplication.new(select('#',...) + 1, {'lua', ...})

    btn = QPushButton.new("Hello World!")
    btn:connect('2pressed()', function(self)
        print("I'm about to close...")
        self:close()
    end)
    btn:setWindowTitle("A great example!")
    btn:resize(300,50)
    btn:show()

    app.exec()

For more examples, check out the `test` folder and the `doc`
folder for documentation on detailed usage - memory management,
signal/slot handling, virtual method overloading, etc. Also, have
a look at the [examples](https://github.com/mkottman/lqt/wiki/Examples)
and feel free to add your own!

License
-------

Copyright (c) 2007-2009 Mauro Iazzi
Copyright (c) 2008-2009 Peter Kümmel
Copyright (c) 2010-2011 Michal Kottman

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
