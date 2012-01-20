#include "lqt_qt.hpp"


int lqtL_qt_metacall (lua_State *L, QObject *self, QObject *acceptor,
        QMetaObject::Call call, const char *name,
        int index, void **args) {
    int callindex = 0, oldtop = 0;
    oldtop = lua_gettop(L);
    lqtL_pushudata(L, self, name); // (1)
    lua_getfield(L, -1, LQT_OBJSIGS); // (2)
    if (lua_isnil(L, -1)) {
        // TODO: determine what is wrong
        lua_settop(L, oldtop);
        QMetaObject::activate(self, self->metaObject(), index, args);
    } else {
        //qDebug() << lua_gettop(L) << luaL_typename(L, -1);
        lua_rawgeti(L, -1, index + 1); // (3)
        if (!lua_isstring(L, -1)) {
            lua_settop(L, oldtop);
            QMetaObject::activate(self, self->metaObject(), index, args);
        } else {
            callindex = acceptor->metaObject()->indexOfSlot(lua_tostring(L, -1));
            // qDebug() << "Found slot" << name << lua_tostring(L,-1) << "on" << acceptor->objectName() << "with index" << callindex;
            lua_pop(L, 2); // (1)
            lua_getfield(L, -1, LQT_OBJSLOTS); // (2)
            lua_rawgeti(L, -1, index+1); // (3)
            lua_remove(L, -2); // (2)
            index = acceptor->qt_metacall(call, callindex, args);
            lua_settop(L, oldtop);
        }
    }
    return -1;
}


const char add_method_func[] =
"return function(qobj, signature, func)\n"
"	local qname = 'LuaObject('..tostring(qobj)..')'\n"
"	local stringdata = qobj['"LQT_OBJMETASTRING"']\n"
"	local data = qobj['"LQT_OBJMETADATA"']\n"
"	local slots = qobj['"LQT_OBJSLOTS"']\n"
"	local sigs = qobj['"LQT_OBJSIGS"']\n"
"	if stringdata==nil then\n"
"		--print'adding a slot!'\n"
"		--initialize\n"
"		stringdata = qname..'\\0'\n"
"		data = setmetatable({}, {__index=table})\n"
"		data:insert(1) -- revision\n"
"		data:insert(0) -- class name\n"
"		data:insert(0) -- class info (1)\n"
"		data:insert(0) -- class info (2)\n"
"		data:insert(0) -- number of methods\n"
"		data:insert(10) -- beginning of methods\n"
"		data:insert(0) -- number of properties\n"
"		data:insert(0) -- beginning of properties\n"
"		data:insert(0) -- number of enums/sets\n"
"		data:insert(0) -- beginning of enums/sets\n"
"		slots = setmetatable({}, {__index=table})\n"
"		sigs = setmetatable({}, {__index=table})\n"
"	end\n"
"	local name, args = string.match(signature, '^(.*)(%b())$')\n"
"	local arg_list = ''\n"
"	if args=='()' then\n"
"		arg_list=''\n"
"	else\n"
"		local argnum = select(2, string.gsub(args, '.+,', ','))+1\n"
"		for i = 1, argnum do\n"
"			if i>1 then arg_list=arg_list..', ' end\n"
"			arg_list = arg_list .. 'arg' .. i\n"
"		end\n"
"	end\n"
"	--print(arg_list, signature)\n"
"	local sig, params = #stringdata + #arg_list + 1, #stringdata -- , ty, tag, flags\n"
"	stringdata = stringdata .. arg_list .. '\\0' .. signature .. '\\0'\n"
"	data:insert(sig) -- print(sig, string.byte(stringdata, sig, sig+4), string.char(string.byte(stringdata, sig+1, sig+6)))\n"
"	data:insert(params) -- print(params, string.char(string.byte(stringdata, params+1, params+10)))\n"
"	data:insert(#stringdata-1) -- print(#stringdata-1, (string.byte(stringdata, #stringdata)))\n"
"	data:insert(#stringdata-1) -- print(#stringdata-1, (string.byte(stringdata, #stringdata)))\n"
"	if func then\n"
"		data:insert(0x0a)\n"
"		slots:insert(func)\n"
"		sigs:insert('__slot'..signature:match'%b()')\n"
"	else\n"
"		data:insert(0x05)\n"
"		slots:insert(false)\n"
"		sigs:insert(false)\n"
"	end\n"
"	data[5] = data[5] + 1\n"
"	qobj['"LQT_OBJMETASTRING"'] = stringdata\n"
"	qobj['"LQT_OBJMETADATA"'] = data\n"
"	qobj['"LQT_OBJSLOTS"'] = slots\n"
"	qobj['"LQT_OBJSIGS"'] = sigs\n"
"end\n";

#include <QMetaObject>
#include <QMetaMethod>

#define CASE(x) case QMetaMethod::x : lua_pushstring(L, " " #x); break
static int lqtL_methods(lua_State *L) {
	QObject* self = static_cast<QObject*>(lqtL_toudata(L, 1, "QObject*"));
	if (self == NULL)
		return luaL_argerror(L, 1, "expecting QObject*");
	const QMetaObject *mo = self->metaObject();
	lua_createtable(L, mo->methodCount(), 0);
	for (int i=0; i < mo->methodCount(); i++) {
		QMetaMethod m = mo->method(i);
		lua_pushstring(L, m.signature());
		switch (m.access()) {
		CASE(Private);
		CASE(Protected);
		CASE(Public);
		}
		switch (m.methodType()) {
		CASE(Method);
		CASE(Signal);
		CASE(Slot);
		CASE(Constructor);
		}
		lua_concat(L, 3);
		lua_rawseti(L, -2, i+1);
	}
	return 1;
}
#undef CASE

static int lqtL_pushqobject(lua_State *L, QObject * object) {
    const QMetaObject * meta = object->metaObject();
    while (meta) {
        QString className = meta->className();
        className += "*";
        char * cname = strdup(qPrintable(className));
        lua_getfield(L, LUA_REGISTRYINDEX, cname);
        int isnil = lua_isnil(L, -1);
        lua_pop(L, 1);
        if (!isnil) {
            lqtL_pushudata(L, object, cname);
            free(cname);
            return 1;
        } else {
            free(cname);
            meta = meta->superClass();
        }
    }
    return 0;
}

static int lqtL_findchild(lua_State *L) {
    QObject* self = static_cast<QObject*>(lqtL_toudata(L, 1, "QObject*"));
    if (self == NULL)
        return luaL_argerror(L, 1, "expecting QObject*");

    QString name = luaL_checkstring(L, 2);
    QObject * child = self->findChild<QObject*>(name);

    if (child) {
        lqtL_pushqobject(L, child);
        return 1;
    } else {
        return 0;
    }
}

static int lqtL_children(lua_State *L) {
    QObject* self = static_cast<QObject*>(lqtL_toudata(L, 1, "QObject*"));
    if (self == NULL)
        return luaL_argerror(L, 1, "expecting QObject*");
    const QObjectList & children = self->children();

    lua_newtable(L);
    for (int i=0; i < children.count(); i++) {
        QObject * object = children[i];
        QString name = object->objectName();
        if (!name.isEmpty() && lqtL_pushqobject(L, object)) {
            lua_setfield(L, -2, qPrintable(name));
        }
    }
    return 1;
}

static int lqtL_connect(lua_State *L) {
    static int methodId = 0;

    QObject* sender = static_cast<QObject*>(lqtL_toudata(L, 1, "QObject*"));
    if (sender == NULL)
        return luaL_argerror(L, 1, "sender not QObject*");

    const char *signal = luaL_checkstring(L, 2);
    const QMetaObject *senderMeta = sender->metaObject();
    int idxS = senderMeta->indexOfSignal(signal + 1);
    if (idxS == -1)
        return luaL_argerror(L, 2, qPrintable(QString("no such sender signal: '%1'").arg(signal + 1)));

    QObject* receiver;
    QString methodName;

    if (lua_type(L, 3) == LUA_TFUNCTION) {
        receiver = sender;

        // simulate sender:__addmethod('LQT_SLOT_X(signature)', function()...end)
        QMetaMethod m = senderMeta->method(idxS);
        methodName = QString(m.signature()).replace(QRegExp("^[^\\(]+"), QString("LQT_SLOT_%1").arg(methodId++));

        lua_getfield(L, 1, "__addmethod");
        lua_pushvalue(L, 1);
        lua_pushstring(L, qPrintable(methodName));
        lua_pushvalue(L, 3);
        lua_call(L, 3, 0);
        
        methodName.prepend("1");
    } else {
        receiver = static_cast<QObject*>(lqtL_toudata(L, 3, "QObject*"));
        if (receiver == NULL)
            return luaL_argerror(L, 3, "receiver not QObject*");
        const char *method = luaL_checkstring(L, 4);
        methodName = method;

        const QMetaObject *receiverMeta = receiver->metaObject();
        int idxR = receiverMeta->indexOfMethod(method + 1);
        if (idxR == -1)
            return luaL_argerror(L, 4, qPrintable(QString("no such receiver method: '%1'").arg(method + 1)));
    }

    bool ok = QObject::connect(sender, signal, receiver, qPrintable(methodName));
    lua_pushboolean(L, ok);
    return 1;
}

void lqtL_qobject_custom (lua_State *L) {
    lua_getfield(L, LUA_REGISTRYINDEX, "QObject*");
    int qobject = lua_gettop(L);

    lua_pushstring(L, "__addmethod");
    luaL_dostring(L, add_method_func);
    lua_rawset(L, qobject);

    lua_pushstring(L, "__methods");
    lua_pushcfunction(L, lqtL_methods);
    lua_rawset(L, qobject);

    lua_pushstring(L, "findChild");
    lua_pushcfunction(L, lqtL_findchild);
    lua_rawset(L, qobject);

    lua_pushstring(L, "children");
    lua_pushcfunction(L, lqtL_children);
    lua_rawset(L, qobject);

    lua_pushstring(L, "connect");
    lua_pushcfunction(L, lqtL_connect);
    lua_rawset(L, qobject);

    // also modify the static QObject::connect function
    lua_getfield(L, LUA_GLOBALSINDEX, "QObject");
    lua_pushcfunction(L, lqtL_connect);
    lua_setfield(L, -2, "connect");
}


QList<QByteArray> lqtL_getStringList(lua_State *L, int i) {
    QList<QByteArray> ret;
    int n = lua_objlen(L, i);
    for (int i=0; i<n; i++) {
        lua_pushnumber(L, i+1);
        lua_gettable(L, i);
        ret[i] = QByteArray(lua_tostring(L, -1));
        lua_pop(L, 1);
    }
    return ret;
}

void lqtL_pushStringList(lua_State *L, const QList<QByteArray> &table) {
    const int n = table.size();
    lua_createtable(L, n, 0);
    for (int i=0; i<n; i++) {
        lua_pushnumber(L, i+1);
        lua_pushstring(L, table[i].data());
        lua_settable(L, -3);
    }
}

#include <QVariant>

#include <QBitArray>
#include <QByteArray>
#include <QChar>
#include <QDate>
#include <QDateTime>
#include <QEasingCurve>
#include <QHash>
#include <QKeySequence>
#include <QLine>
#include <QLineF>
#include <QList>
#include <QLocale>
#include <QMap>
#include <QRect>
#include <QRectF>
#include <QRegExp>
#include <QSize>
#include <QSizeF>
#include <QString>
#include <QStringList>
#include <QTime>
#include <QUrl>

#ifdef MODULE_qtgui
#include <QBitmap>
#include <QBrush>
#include <QColor>
#include <QCursor>
#include <QFont>
#include <QIcon>
#include <QImage>
#include <QMatrix>
#include <QMatrix4x4>
#include <QPalette>
#include <QPen>
#include <QPoint>
#include <QPointF>
#include <QPixmap>
#include <QPolygon>
#include <QQuaternion>
#include <QRegion>
#include <QSizePolicy>
#include <QTextFormat>
#include <QTextLength>
#include <QTransform>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#endif

int lqtL_qvariant_setValue(lua_State *L) {
	QVariant* self = static_cast<QVariant*>(lqtL_toudata(L, 1, "QVariant*"));
	lqtL_selfcheck(L, self, "QVariant");
	/* basic types */
	if (lua_isnil(L, 2)) {
		self->clear();
	} else if (lua_isboolean(L, 2)) {
		self->setValue((bool)lua_toboolean(L, 2));
	} else if (lua_isnumber(L, 2)) {
		self->setValue((double)lua_tonumber(L, 2));
	} else if (lua_isstring(L, 2)) {
		size_t size;
		const char * str = lua_tolstring(L, 2, &size);
		self->setValue(QByteArray(str, size));
	} /* QtCore types */
	else if (lqtL_testudata(L, 2, "QBitArray*")) {
		self->setValue(*(QBitArray*)lqtL_toudata(L, 2, "QBitArray*"));
	} else if (lqtL_testudata(L, 2, "QByteArray*")) {
		self->setValue(*(QByteArray*)lqtL_toudata(L, 2, "QByteArray*"));
	} else if (lqtL_testudata(L, 2, "QChar*")) {
		self->setValue(*(QChar*)lqtL_toudata(L, 2, "QChar*"));
	} else if (lqtL_testudata(L, 2, "QDate*")) {
		self->setValue(*(QDate*)lqtL_toudata(L, 2, "QDate*"));
	} else if (lqtL_testudata(L, 2, "QDateTime*")) {
		self->setValue(*(QDateTime*)lqtL_toudata(L, 2, "QDateTime*"));
	} else if (lqtL_testudata(L, 2, "QKeySequence*")) {
		self->setValue(*(QKeySequence*)lqtL_toudata(L, 2, "QKeySequence*"));
	} else if (lqtL_testudata(L, 2, "QLine*")) {
		self->setValue(*(QLine*)lqtL_toudata(L, 2, "QLine*"));
	} else if (lqtL_testudata(L, 2, "QLineF*")) {
		self->setValue(*(QLineF*)lqtL_toudata(L, 2, "QLineF*"));
	} else if (lqtL_testudata(L, 2, "QList<QVariant>*")) {
		self->setValue(*(QList<QVariant>*)lqtL_toudata(L, 2, "QList<QVariant>*"));
	} else if (lqtL_testudata(L, 2, "QLocale*")) {
		self->setValue(*(QLocale*)lqtL_toudata(L, 2, "QLocale*"));
	} else if (lqtL_testudata(L, 2, "QRect*")) {
		self->setValue(*(QRect*)lqtL_toudata(L, 2, "QRect*"));
	} else if (lqtL_testudata(L, 2, "QRectF*")) {
		self->setValue(*(QRectF*)lqtL_toudata(L, 2, "QRectF*"));
	} else if (lqtL_testudata(L, 2, "QRegExp*")) {
		self->setValue(*(QRegExp*)lqtL_toudata(L, 2, "QRegExp*"));
	} else if (lqtL_testudata(L, 2, "QSize*")) {
		self->setValue(*(QSize*)lqtL_toudata(L, 2, "QSize*"));
	} else if (lqtL_testudata(L, 2, "QSizeF*")) {
		self->setValue(*(QSizeF*)lqtL_toudata(L, 2, "QSizeF*"));
	} else if (lqtL_testudata(L, 2, "QString*")) {
		self->setValue(*(QString*)lqtL_toudata(L, 2, "QString*"));
	} else if (lqtL_testudata(L, 2, "QStringList*")) {
		self->setValue(*(QStringList*)lqtL_toudata(L, 2, "QStringList*"));
	} else if (lqtL_testudata(L, 2, "QTime*")) {
		self->setValue(*(QTime*)lqtL_toudata(L, 2, "QTime*"));
	} else if (lqtL_testudata(L, 2, "QUrl*")) {
		self->setValue(*(QUrl*)lqtL_toudata(L, 2, "QUrl*"));
	} 
#ifdef MODULE_qtgui	
	/* QtGui types */
	else if (lqtL_testudata(L, 2, "QBitmap*")) {
		self->setValue(*(QBitmap*)lqtL_toudata(L, 2, "QBitmap*"));
	} else if (lqtL_testudata(L, 2, "QBrush*")) {
		self->setValue(*(QBrush*)lqtL_toudata(L, 2, "QBrush*"));
	} else if (lqtL_testudata(L, 2, "QColor*")) {
		self->setValue(*(QColor*)lqtL_toudata(L, 2, "QColor*"));
	} else if (lqtL_testudata(L, 2, "QCursor*")) {
		self->setValue(*(QCursor*)lqtL_toudata(L, 2, "QCursor*"));
	} else if (lqtL_testudata(L, 2, "QFont*")) {
		self->setValue(*(QFont*)lqtL_toudata(L, 2, "QFont*"));
	} else if (lqtL_testudata(L, 2, "QIcon*")) {
		self->setValue(*(QIcon*)lqtL_toudata(L, 2, "QIcon*"));
	} else if (lqtL_testudata(L, 2, "QImage*")) {
		self->setValue(*(QImage*)lqtL_toudata(L, 2, "QImage*"));
	} else if (lqtL_testudata(L, 2, "QMatrix*")) {
		self->setValue(*(QMatrix*)lqtL_toudata(L, 2, "QMatrix*"));
	} else if (lqtL_testudata(L, 2, "QMatrix4x4*")) {
		self->setValue(*(QMatrix4x4*)lqtL_toudata(L, 2, "QMatrix4x4*"));
	} else if (lqtL_testudata(L, 2, "QPalette*")) {
		self->setValue(*(QPalette*)lqtL_toudata(L, 2, "QPalette*"));
	} else if (lqtL_testudata(L, 2, "QPen*")) {
		self->setValue(*(QPen*)lqtL_toudata(L, 2, "QPen*"));
	} else if (lqtL_testudata(L, 2, "QPoint*")) {
		self->setValue(*(QPoint*)lqtL_toudata(L, 2, "QPoint*"));
	} else if (lqtL_testudata(L, 2, "QPointF*")) {
		self->setValue(*(QPointF*)lqtL_toudata(L, 2, "QPointF*"));
	} else if (lqtL_testudata(L, 2, "QPixmap*")) {
		self->setValue(*(QPixmap*)lqtL_toudata(L, 2, "QPixmap*"));
	} else if (lqtL_testudata(L, 2, "QPolygon*")) {
		self->setValue(*(QPolygon*)lqtL_toudata(L, 2, "QPolygon*"));
	} else if (lqtL_testudata(L, 2, "QQuaternion*")) {
		self->setValue(*(QQuaternion*)lqtL_toudata(L, 2, "QQuaternion*"));
	} else if (lqtL_testudata(L, 2, "QRegion*")) {
		self->setValue(*(QRegion*)lqtL_toudata(L, 2, "QRegion*"));
	} else if (lqtL_testudata(L, 2, "QSizePolicy*")) {
		self->setValue(*(QSizePolicy*)lqtL_toudata(L, 2, "QSizePolicy*"));
	} else if (lqtL_testudata(L, 2, "QTextFormat*")) {
		self->setValue(*(QTextFormat*)lqtL_toudata(L, 2, "QTextFormat*"));
	} else if (lqtL_testudata(L, 2, "QTextLength*")) {
		self->setValue(*(QTextLength*)lqtL_toudata(L, 2, "QTextLength*"));
	} else if (lqtL_testudata(L, 2, "QTransform*")) {
		self->setValue(*(QTransform*)lqtL_toudata(L, 2, "QTransform*"));
	} else if (lqtL_testudata(L, 2, "QVector2D*")) {
		self->setValue(*(QVector2D*)lqtL_toudata(L, 2, "QVector2D*"));
	} else if (lqtL_testudata(L, 2, "QVector3D*")) {
		self->setValue(*(QVector3D*)lqtL_toudata(L, 2, "QVector3D*"));
	} else if (lqtL_testudata(L, 2, "QVector4D*")) {
		self->setValue(*(QVector4D*)lqtL_toudata(L, 2, "QVector4D*"));
	}
#endif
	return 0;
}

int lqtL_qvariant_value(lua_State *L) {
	QVariant* self = static_cast<QVariant*>(lqtL_toudata(L, 1, "QVariant*"));
	lqtL_selfcheck(L, self, "QVariant");
	QVariant::Type type;
	if (lua_isnoneornil(L, 2)) {
		type = self->type();
	} else {
		type = (QVariant::Type)lqtL_toenum(L, 2, "QVariant.Type");
		const char * currentType = self->typeName();
		if (!self->canConvert(type) || !self->convert(type)) {
			lua_pushnil(L);
			lua_pushfstring(L, "cannot convert %s to %s", currentType, self->typeToName(type));
			return 2;
		}
	}
	switch (self->type()) {
		case QVariant::Invalid: lua_pushnil(L); return 1;
		/* basic types */
		case QVariant::Bool: lua_pushboolean(L, self->toBool()); return 1;
		case QVariant::Double: lua_pushnumber(L, self->toDouble()); return 1;
		case QVariant::Int: lua_pushinteger(L, self->toInt()); return 1;
		case QVariant::UInt: lua_pushinteger(L, self->toUInt()); return 1;
		case QVariant::LongLong: lua_pushnumber(L, self->toLongLong()); return 1;
		case QVariant::ULongLong: lua_pushnumber(L, self->toULongLong()); return 1;
		case QVariant::ByteArray: {
			const QByteArray &ba = self->toByteArray();
			lua_pushlstring(L, ba.data(), ba.size());
			return 1;
		};
		/* QtCore types */
		case QVariant::BitArray: lqtL_passudata(L, new QBitArray(self->value<QBitArray>()), "QBitArray*"); return 1;
		case QVariant::Char: lqtL_passudata(L, new QChar(self->value<QChar>()), "QChar*"); return 1;
		case QVariant::Date: lqtL_passudata(L, new QDate(self->value<QDate>()), "QDate*"); return 1;
		case QVariant::DateTime: lqtL_passudata(L, new QDateTime(self->value<QDateTime>()), "QDateTime*"); return 1;
		case QVariant::KeySequence: lqtL_passudata(L, new QKeySequence(self->value<QKeySequence>()), "QKeySequence*"); return 1;
		case QVariant::Line: lqtL_passudata(L, new QLine(self->value<QLine>()), "QLine*"); return 1;
		case QVariant::LineF: lqtL_passudata(L, new QLineF(self->value<QLineF>()), "QLineF*"); return 1;
		case QVariant::List: lqtL_passudata(L, new QList<QVariant>(self->toList()), "QList<QVariant>*"); return 1;
		case QVariant::Locale: lqtL_passudata(L, new QLocale(self->value<QLocale>()), "QLocale*"); return 1;
		case QVariant::Point: lqtL_passudata(L, new QPoint(self->value<QPoint>()), "QPoint*"); return 1;
		case QVariant::PointF: lqtL_passudata(L, new QPointF(self->value<QPointF>()), "QPointF*"); return 1;
		case QVariant::Rect: lqtL_passudata(L, new QRect(self->value<QRect>()), "QRect*"); return 1;
		case QVariant::RectF: lqtL_passudata(L, new QRectF(self->value<QRectF>()), "QRectF*"); return 1;
		case QVariant::RegExp: lqtL_passudata(L, new QRegExp(self->value<QRegExp>()), "QRegExp*"); return 1;
		case QVariant::Size: lqtL_passudata(L, new QSize(self->value<QSize>()), "QSize*"); return 1;
		case QVariant::SizeF: lqtL_passudata(L, new QSizeF(self->value<QSizeF>()), "QSizeF*"); return 1;
		case QVariant::String: lqtL_passudata(L, new QString(self->value<QString>()), "QString*"); return 1;
		case QVariant::StringList: lqtL_passudata(L, new QStringList(self->value<QStringList>()), "QStringList*"); return 1;
		case QVariant::Time: lqtL_passudata(L, new QTime(self->value<QTime>()), "QTime*"); return 1;
		case QVariant::Url: lqtL_passudata(L, new QUrl(self->value<QUrl>()), "QUrl*"); return 1;
#ifdef MODULE_qtgui
		/* QtGui types */
		case QVariant::Bitmap: lqtL_passudata(L, new QBitmap(self->value<QBitmap>()), "QBitmap*"); return 1;
		case QVariant::Brush: lqtL_passudata(L, new QBrush(self->value<QBrush>()), "QBrush*"); return 1;
		case QVariant::Color: lqtL_passudata(L, new QColor(self->value<QColor>()), "QColor*"); return 1;
		case QVariant::Cursor: lqtL_passudata(L, new QCursor(self->value<QCursor>()), "QCursor*"); return 1;
		case QVariant::Font: lqtL_passudata(L, new QFont(self->value<QFont>()), "QFont*"); return 1;
		case QVariant::Icon: lqtL_passudata(L, new QIcon(self->value<QIcon>()), "QIcon*"); return 1;
		case QVariant::Image: lqtL_passudata(L, new QImage(self->value<QImage>()), "QImage*"); return 1;
		case QVariant::Matrix: lqtL_passudata(L, new QMatrix(self->value<QMatrix>()), "QMatrix*"); return 1;
		case QVariant::Matrix4x4: lqtL_passudata(L, new QMatrix4x4(self->value<QMatrix4x4>()), "QMatrix4x4*"); return 1;
		case QVariant::Palette: lqtL_passudata(L, new QPalette(self->value<QPalette>()), "QPalette*"); return 1;
		case QVariant::Pen: lqtL_passudata(L, new QPen(self->value<QPen>()), "QPen*"); return 1;
		case QVariant::Pixmap: lqtL_passudata(L, new QPixmap(self->value<QPixmap>()), "QPixmap*"); return 1;
		case QVariant::Polygon: lqtL_passudata(L, new QPolygon(self->value<QPolygon>()), "QPolygon*"); return 1;
		case QVariant::Quaternion: lqtL_passudata(L, new QQuaternion(self->value<QQuaternion>()), "QQuaternion*"); return 1;
		case QVariant::Region: lqtL_passudata(L, new QRegion(self->value<QRegion>()), "QRegion*"); return 1;
		case QVariant::SizePolicy: lqtL_passudata(L, new QSizePolicy(self->value<QSizePolicy>()), "QSizePolicy*"); return 1;
		case QVariant::Transform: lqtL_passudata(L, new QTransform(self->value<QTransform>()), "QTransform*"); return 1;
		case QVariant::TextFormat: lqtL_passudata(L, new QTextFormat(self->value<QTextFormat>()), "QTextFormat*"); return 1;
		case QVariant::TextLength: lqtL_passudata(L, new QTextLength(self->value<QTextLength>()), "QTextLength*"); return 1;
		case QVariant::Vector2D: lqtL_passudata(L, new QVector2D(self->value<QVector2D>()), "QVector2D*"); return 1;
		case QVariant::Vector3D: lqtL_passudata(L, new QVector3D(self->value<QVector3D>()), "QVector3D*"); return 1;
		case QVariant::Vector4D: lqtL_passudata(L, new QVector4D(self->value<QVector4D>()), "QVector4D*"); return 1;
#endif
	}
	return 0;
}

#ifndef MODULE_qtgui
void lqtL_qvariant_custom(lua_State *L)
#else
void lqtL_qvariant_custom_qtgui(lua_State *L)
#endif
{
	lua_getfield(L, LUA_REGISTRYINDEX, "QVariant*");
	int qvariant = lua_gettop(L);

	lua_pushliteral(L, "value");
	lua_pushcfunction(L, lqtL_qvariant_value);
	lua_rawset(L, qvariant);

	lua_pushliteral(L, "setValue");
	lua_pushcfunction(L, lqtL_qvariant_setValue);
	lua_rawset(L, qvariant);
}

