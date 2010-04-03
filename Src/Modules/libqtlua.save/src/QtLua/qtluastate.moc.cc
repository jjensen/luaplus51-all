/****************************************************************************
** Meta object code from reading C++ file 'qtluastate.hh'
**
** Created: Sun May 24 00:34:22 2009
**      by: The Qt Meta Object Compiler version 61 (Qt 4.5.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "qtluastate.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qtluastate.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 61
#error "This file was generated using the moc from 4.5.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QtLua__State[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   12, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

 // signals: signature, parameters, type, tag, flags
      18,   14,   13,   13, 0x05,

 // slots: signature, parameters, type, tag, flags
      45,   34,   13,   13, 0x0a,
      85,   59,   13,   13, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_QtLua__State[] = {
    "QtLua::State\0\0str\0output(QString)\0"
    "statements\0exec(QString)\0"
    "prefix,list,cursor_offset\0"
    "fill_completion_list(QString,QStringList&,int&)\0"
};

const QMetaObject QtLua::State::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_QtLua__State,
      qt_meta_data_QtLua__State, 0 }
};

const QMetaObject *QtLua::State::metaObject() const
{
    return &staticMetaObject;
}

void *QtLua::State::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtLua__State))
        return static_cast<void*>(const_cast< State*>(this));
    return QObject::qt_metacast(_clname);
}

int QtLua::State::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: output((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: exec((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: fill_completion_list((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< QStringList(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        default: ;
        }
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void QtLua::State::output(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
