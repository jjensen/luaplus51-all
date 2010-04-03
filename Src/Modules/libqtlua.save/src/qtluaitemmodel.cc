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


#include <QIcon>

#include <QtLua/Item>
#include <QtLua/ListItem>
#include <QtLua/ItemModel>

namespace QtLua {

ItemModel::ItemModel(Item::ptr root, QObject *parent)
  : QAbstractItemModel(parent),
    _root(root)
{
  assert(!_root->get_model());
  _root->set_model(this);
}

ItemModel::~ItemModel()
{
  _root->set_model(0);
}

Item::ptr ItemModel::get_item(const QModelIndex &index)
{
  return *static_cast<Item*>(index.internalPointer());
}

QVariant ItemModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  Item *item = static_cast<Item*>(index.internalPointer());

  switch (role)
    {
    case Qt::DisplayRole:
      return QVariant(item->get_name());

    case Qt::DecorationRole:
      return item->get_icon();

    default:
      return QVariant();
    }
}

Qt::ItemFlags ItemModel::flags(const QModelIndex &index) const
{
  if (!index.isValid())
    return 0;

  Item *item = static_cast<Item*>(index.internalPointer());

  Qt::ItemFlags res = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

  if (item->is_rename_allowed())
    res |= Qt::ItemIsEditable;

  return res;
}

QVariant ItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  return QVariant();
}

QModelIndex ItemModel::index(int row, int column, const QModelIndex &parent) const
{
  Item *p;

  if (column)
    return QModelIndex();    

  if (!parent.isValid())
    p = _root.ptr();
  else
    p = static_cast<Item*>(parent.internalPointer());

  Item *c = p->get_child_row(row);

  if (c)
    return createIndex(row, column, c);
  else
    return QModelIndex();
}

QModelIndex ItemModel::parent(const QModelIndex &index) const
{
  if (!index.isValid())
    return QModelIndex();

  Item *c = static_cast<Item*>(index.internalPointer());
  Item *p = c->get_parent();

  if (!p || p == _root.ptr())
    return QModelIndex();

  return createIndex(p->get_row(), 0, p);
}

int ItemModel::rowCount(const QModelIndex &parent) const
{
  Item *p;

  if (!parent.isValid())
    p = _root.ptr();
  else
    p = static_cast<Item*>(parent.internalPointer());

  return p->get_child_count();
}

int ItemModel::columnCount(const QModelIndex &parent) const
{
  return 1;
}

bool ItemModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
  if (!index.isValid())
    return false;

  Item *item = static_cast<Item*>(index.internalPointer());

  switch (role)
    {
    case Qt::EditRole:
      item->set_name(value.toString());
      return true;

    default:
      return false;
    }
}

bool ItemModel::insertRows(int row, int count, const QModelIndex & parent)
{
  return false;
  //  beginInsertRows(parent, row, row + count - 1);
  //  endInsertRows();
}

bool ItemModel::removeRows(int row, int count, const QModelIndex & parent)
{
  ListItem *p;

  if (!parent.isValid())
    return false;
  else
    p = static_cast<ListItem*>((Item*)parent.internalPointer());

  beginRemoveRows(parent, row, row + count - 1);

  for (unsigned int i = row; i < row + count; i++)
    {
      Item *item = p->get_child_row(i);
      item->set_model(0);
      p->qtllistitem_remove(item);
    }

  endRemoveRows();

  return true;
}

}

