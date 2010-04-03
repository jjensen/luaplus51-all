
/* anchor 1 */
#include <QtLua/Function>
#include <QtLua/Plugin>

#include "plugin.hh"

Q_EXPORT_PLUGIN2(example, ExamplePlugin)

const QtLua::String & ExamplePlugin::get_name() const
{
  static const QtLua::String s("example");
  return s;
}

const QtLua::String & ExamplePlugin::get_description() const
{
  static const QtLua::String s("This is just an example plugin");
  return s;
}

void ExamplePlugin::register_members(QtLua::Plugin &plugin)
{

  static class : public QtLua::Function
  {
    QtLua::Value::List meta_call(QtLua::State &ls, const QtLua::Value::List &args)
    {
      return QtLua::Value(ls, "foo");
    }

  } foo;

  foo.register_(plugin, "foo");
}
/* anchor end */

struct LoadUnload
{
  LoadUnload()
  {
    qDebug("example plugin loaded");
  }

  ~LoadUnload()
  {
    qDebug("example plugin unloaded");
  }
} load_unload;

