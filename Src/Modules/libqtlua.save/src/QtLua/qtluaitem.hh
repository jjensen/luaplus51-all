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


#ifndef QTLUAITEM_HH_
#define QTLUAITEM_HH_

#include <QModelIndex>
#include <QIcon>

#include "qtluastring.hh"
#include "qtluauserdata.hh"
#include "qtluavalue.hh"

namespace QtLua {

class ItemModel;
class ListItem;

  /**
   * @short Qt Model/View item class
   * @header QtLua/Item
   * @module {Model/View}
   *
   * This class together with the @ref ListItem and @ref ItemModel
   * classes enable easy use of list or hierarchical data structures
   * that can be viewed and modified from lua script, Qt view widgets
   * and C++ code.
   *
   * This class implement the generic hierarchical data structure leaf
   * node. It must be used as a base class for objects which may be
   * exposed to Qt views via the @ref ItemModel class.
   *
   * @ref Item objects can be inserted in and removed from @ref
   * ListItem objects from the C++ code with the @ref insert and @ref
   * remove functions. 
   *
   * Each @ref Item object have a node name used for display in Qt
   * views and access from lua script. This name can be accessed from
   * C++ code with the @ref get_name and @ref set_name functions.
   *
   * Each data structure modification by lua script or user view
   * interaction may be allowed or denied by reimplemention of
   * @ref is_move_allowed, @ref is_rename_allowed, @ref
   * is_remove_allowed, and @ref is_replace_allowed functions.
   *
   * See @ref ItemModel for example.
   */

class Item : public UserData
{
  friend class ItemModel;
  friend class ListItem;

public:

  QTLUA_REFTYPE(Item);

  /** Create a new Item with given name */
  Item(const String &name = "");

  /** Create a new Item copy */
  Item(const Item &item);

  ~Item();

  /** Insert this item in parent container */
  void			insert(QtLua::Ref<ListItem> parent);

  /** Remove this item from its container */
  void			remove();

  /** Set item name. Name may be mangled to be a valid lua identifier. */
  void			set_name(const String &name);

  /** Get item name */
  inline const String & get_name() const;

  /** Get pointer to parent container */
  inline ListItem *	get_parent() const;

protected:

  /** Must return icon decoration to use for this node. */
  virtual QIcon &	get_icon() const;

  /** Must return true if item can change parent containers.
      (default is true) */
  virtual bool	is_move_allowed() const;

  /** Must return true if item can renamed.
      (default is true) */
  virtual bool	is_rename_allowed() const;

  /** Must return true if item can be removed from container.
      (default is true) */
  virtual bool	is_remove_allowed() const;

  /** Must return true if item can be removed by replacement by an
      other item (default is is_remove_allowed()) */
  virtual bool	is_replace_allowed() const;

private:
  const Item &operator=(const Item &);

  virtual void			set_model(ItemModel* model);
  inline ItemModel *		get_model() const;
  bool				in_parent_path(Item *item);
  void				rename_insert();
  inline QModelIndex		model_index() const;
  inline int			get_row() const;
  inline void			set_row(int row);
  virtual Item *		get_child_row(int row) const;
  virtual int			get_child_count() const;

  String			_name;
  ListItem			*_parent;
  ItemModel			*_model;
  int				_row;
};

}

#endif

