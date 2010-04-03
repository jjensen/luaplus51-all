/****************************************************************************
** Meta object code from reading C++ file 'qtluaqtlib.hh'
**
** Created: Wed May 27 17:57:12 2009
**      by: The Qt Meta Object Compiler version 61 (Qt 4.5.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "qtluaqtlib.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qtluaqtlib.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 61
#error "This file was generated using the moc from 4.5.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QtLua__QFileDialog[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       5,   12, // properties
       0,    0, // enums/sets
       0,    0, // constructors

 // properties: name, type, flags
      27,   19, 0x0a095103,
      49,   37, 0x0b095103,
      57,   37, 0x0b095001,
      71,   19, 0x0a095001,
      90,   37, 0x0b095003,

       0        // eod
};

static const char qt_meta_stringdata_QtLua__QFileDialog[] = {
    "QtLua::QFileDialog\0QString\0directory\0"
    "QStringList\0history\0selectedFiles\0"
    "selectedNameFilter\0nameFilters\0"
};

const QMetaObject QtLua::QFileDialog::staticMetaObject = {
    { &::QFileDialog::staticMetaObject, qt_meta_stringdata_QtLua__QFileDialog,
      qt_meta_data_QtLua__QFileDialog, 0 }
};

const QMetaObject *QtLua::QFileDialog::metaObject() const
{
    return &staticMetaObject;
}

void *QtLua::QFileDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtLua__QFileDialog))
        return static_cast<void*>(const_cast< QFileDialog*>(this));
    typedef ::QFileDialog QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int QtLua::QFileDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef ::QFileDialog QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    
#ifndef QT_NO_PROPERTIES
     if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QString*>(_v) = get_directory(); break;
        case 1: *reinterpret_cast< QStringList*>(_v) = history(); break;
        case 2: *reinterpret_cast< QStringList*>(_v) = selectedFiles(); break;
        case 3: *reinterpret_cast< QString*>(_v) = selectedFilter(); break;
        case 4: *reinterpret_cast< QStringList*>(_v) = filters(); break;
        }
        _id -= 5;
    } else if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: setDirectory(*reinterpret_cast< QString*>(_v)); break;
        case 1: setHistory(*reinterpret_cast< QStringList*>(_v)); break;
        case 4: setFilters(*reinterpret_cast< QStringList*>(_v)); break;
        }
        _id -= 5;
    } else if (_c == QMetaObject::ResetProperty) {
        _id -= 5;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 5;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 5;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 5;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 5;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 5;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}
QT_END_MOC_NAMESPACE
