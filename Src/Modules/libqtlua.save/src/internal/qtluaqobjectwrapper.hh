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


#ifndef QTLUAQOBJECTWRAPPER_HH_
#define QTLUAQOBJECTWRAPPER_HH_

#include <QObject>
#include <QMetaObject>

#include <QtLua/qtluauserdata.hh>

namespace QtLua {

  class QObjectIterator;

/**
 * @short QObject wrapper class (internal)
 * @header internal/QObjectWrapper
 * @module {QObject wrapping}
 * @internal
 *
 * This internal class implements the @ref QObject wrapper decribed in the
 * @xref{QObject wrapping} section.
 */

  class QObjectWrapper : public UserData, public QObject
  {
    friend class QObjectIterator;

  public:
    QTLUA_REFTYPE(QObjectWrapper);

    /** Create or find existing QObjectWrapper associated with given
	QObject. QObject will be deleted when wrapper is destroyed
	only if it flagged as deletable and has no more
	parent. Deletable flag is set on wrapper creation if QObject
	has a parent. */
    static Ref<QObjectWrapper> get_wrapper(State &st, QObject *obj);

    /** Same as get_wrapper() but specify if QObject parent change is
	allowed (default is true) and if QObject must be deleted when
	no references to wrapper remains and it has no parent (default
	is true if QObject had a parent on wrapper creation). */
    static Ref<QObjectWrapper> get_wrapper(State &st, QObject *obj, bool reparent, bool delete_);

    /** Specify if wrapped QObject parent change is allowed */
    inline void set_reparent(bool reparent);
    /** Specify if wrapped QObject must be deleted when no more
	refenrences to wrapper exists and it has no parent. */
    inline void set_delete(bool delete_);

    /** Get reference to wrapped qobject. Throw QPointer has become null. */
    inline QObject & get_object();

    /** Check if wrapped qobject still exist */
    inline bool valid() const;

    /** Get reference to state */
    inline State & get_state();

    /** Return object name, forge a decent one if empty */
    static String qobject_name(QObject &obj);

    /** Find QObject child non-recursively */
    static QObject * get_child(QObject &obj, const String &name);

    // internal use only
    int qt_metacall(QMetaObject::Call c, int id, void **args);
    void _lua_connect(int sigindex, const Value &v);
    bool _lua_disconnect(int sigindex, const Value &v);
    void _lua_disconnect_all(int sigindex);
    void _lua_disconnect_all();

    ~QObjectWrapper();
  private:

    void reparent(QObject *parent);

    QObjectWrapper(State &st, QObject *obj);
    QObjectWrapper(const QObjectWrapper &qow);

    Value meta_index(State &ls, const Value &key);
    void meta_newindex(State &ls, const Value &key, const Value &value);
    Ref<Iterator> new_iterator(State &ls);
    void completion_patch(String &path, String &entry, int &offset);
    String get_type_name() const;
    String get_value_str() const;
    void obj_destroyed();
    void ref_drop(int count);

  private:

    typedef QHash<int, Value> lua_slots_hash_t;

    State &_ls;
    QObject *_obj;
    lua_slots_hash_t _lua_slots;
    int _lua_next_slot;
    bool _reparent;
    bool _delete;
  };

}

#endif

