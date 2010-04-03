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


#ifndef QTLUABITMAPLISTMODEL_HH_
#define QTLUABITMAPLISTMODEL_HH_

#include <QAbstractItemModel>

#include "qtluaitem.hh"

namespace QtLua {

  /**
   * @short Qt Model/View model class
   * @header QtLua/ItemModel
   * @module {Model/View}
   *
   * This class together with the @ref ListItem and @ref Item classes
   * enable easy use of list or hierarchical data structures that can be
   * viewed and modified from lua script, Qt view widgets and C++
   * code.
   *
   * The hierarchical data structure is accessible from lua as nested
   * tables. Data are not duplicated and any change to the data from
   * lua immediately update to the Qt view widget.
   *
   * Usage example:
   * @example examples/cpp/mvc/treeview.cc:1
   */

class ItemModel : public QAbstractItemModel
{
  friend class Item;

  Q_OBJECT;

public:
  /** Create a new item model with pointer to root item */
  ItemModel(Item::ptr root, QObject *parent = 0);

  ~ItemModel();

  /** Get pointer to Item from QT model index. */
  static Item::ptr get_item(const QModelIndex &index);

private:
  QVariant	data(const QModelIndex &index, int role) const;
  Qt::ItemFlags	flags(const QModelIndex &index) const;
  QVariant	headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  QModelIndex	index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
  QModelIndex	parent(const QModelIndex &index) const;
  int		rowCount(const QModelIndex &parent = QModelIndex()) const;
  int		columnCount(const QModelIndex &parent = QModelIndex()) const;
  bool		setData(const QModelIndex & index, const QVariant & value, int role);
  bool		insertRows(int row, int count, const QModelIndex & parent);
  bool		removeRows(int row, int count, const QModelIndex & parent);

private:
  Item::ptr _root;
};

}

#endif

