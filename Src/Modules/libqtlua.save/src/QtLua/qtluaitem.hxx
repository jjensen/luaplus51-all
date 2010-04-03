
#ifndef QTLUAITEM_HXX_
#define QTLUAITEM_HXX_

#include <cassert>

#include "qtluauserdata.hxx"
#include "qtluaitemmodel.hh"

namespace QtLua {

QModelIndex Item::model_index() const
{
  return _model->createIndex(_row, 0, (void*)this);
}

void Item::set_row(int row)
{
  _row = row;
}

int Item::get_row() const
{
  return _row;
}

ListItem * Item::get_parent() const
{
  return _parent;
}

const String & Item::get_name() const
{
  return _name;
}

ItemModel * Item::get_model() const
{
  return _model;
}

}

#endif

