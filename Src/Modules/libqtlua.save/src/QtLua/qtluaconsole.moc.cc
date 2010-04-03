/****************************************************************************
** Meta object code from reading C++ file 'qtluaconsole.hh'
**
** Created: Sun May 24 00:33:17 2009
**      by: The Qt Meta Object Compiler version 61 (Qt 4.5.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "qtluaconsole.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qtluaconsole.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 61
#error "This file was generated using the moc from 4.5.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QtLua__Console[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   12, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

 // signals: signature, parameters, type, tag, flags
      20,   16,   15,   15, 0x05,
      69,   43,   15,   15, 0x05,

 // slots: signature, parameters, type, tag, flags
     116,   16,   15,   15, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_QtLua__Console[] = {
    "QtLua::Console\0\0str\0line_validate(QString)\0"
    "prefix,list,cursor_offset\0"
    "get_completion_list(QString,QStringList&,int&)\0"
    "print(QString)\0"
};

const QMetaObject QtLua::Console::staticMetaObject = {
    { &QTextEdit::staticMetaObject, qt_meta_stringdata_QtLua__Console,
      qt_meta_data_QtLua__Console, 0 }
};

const QMetaObject *QtLua::Console::metaObject() const
{
    return &staticMetaObject;
}

void *QtLua::Console::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtLua__Console))
        return static_cast<void*>(const_cast< Console*>(this));
    return QTextEdit::qt_metacast(_clname);
}

int QtLua::Console::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTextEdit::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: line_validate((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: get_completion_list((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< QStringList(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 2: print((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void QtLua::Console::line_validate(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QtLua::Console::get_completion_list(const QString & _t1, QStringList & _t2, int & _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
