#include "lqt_qt.hpp"

// #include <QThread>

#include <QMetaObject>
#include <QMetaMethod>

// #define VERBOSE_BUILD

#define CASE(x) case QMetaMethod::x : lua_pushstring(L, " " #x); break

static int lqtL_methods(lua_State *L) {
	QObject* self = static_cast<QObject*>(lqtL_toudata(L, 1, "QObject*"));
	if (self == NULL)
		return luaL_argerror(L, 1, "expecting QObject*");
	const QMetaObject *mo = self->metaObject();
	lua_createtable(L, mo->methodCount(), 0);
	for (int i=0; i < mo->methodCount(); i++) {
		QMetaMethod m = mo->method(i);
		lua_pushstring(L, m.methodSignature());
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
		lua_rawseti(L, -2, m.methodIndex());
	}
	return 1;
}
#undef CASE

int lqtL_pushqobject(lua_State *L, QObject * object) {
    if (object == nullptr) {
        lua_pushnil(L);
        return 0;
    }
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
    QString className = meta->className();
    luaL_error(L, "QObject `%s` not registered!", className.toStdString().c_str());
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
	int idxS = senderMeta->indexOfSignal(signal);
	if (idxS == -1)
		return luaL_argerror(L, 2, qPrintable(QString("no such sender signal: '%1'").arg(signal)));
	QString signalName = signal;

	QObject* receiver;
	QString methodName;

	if (lua_type(L, 3) == LUA_TFUNCTION) {
		receiver = sender;

		// simulate sender:__addmethod('LQT_SLOT_X(signature)', function()...end)
		QMetaMethod m = senderMeta->method(idxS);
		methodName = QString(m.methodSignature()).replace(QRegExp("^[^\\(]+"), QString("LQT_SLOT_%1").arg(methodId++));

		lua_getfield(L, 1, "__addslot");
		lua_pushvalue(L, 1);
		lua_pushstring(L, qPrintable(methodName));
		lua_pushvalue(L, 3);

		if (lqtL_pcall(L, 3, 0, 0) != 0)
			lua_error(L);

#ifdef VERBOSE_BUILD
		printf("Connect method (%p) %d(`%s`) to lua-method `%s`\n"
			, receiver
			, idxS
			, signal
			, methodName.toStdString().c_str()
		);
#endif
	}
	else {
		receiver = static_cast<QObject*>(lqtL_toudata(L, 3, "QObject*"));
		if (receiver == NULL)
			return luaL_argerror(L, 3, "receiver not QObject*");
		const char *method = luaL_checkstring(L, 4);
		methodName = method;

		const QMetaObject *receiverMeta = receiver->metaObject();
		int idxR = receiverMeta->indexOfMethod(method);
		if (idxR == -1)
			return luaL_argerror(L, 4, qPrintable(QString("no such receiver method: '%1'").arg(method)));

#ifdef VERBOSE_BUILD
		printf("Connect method (%p) %d(`%s`) to method (%p) %d(`%s`)\n"
			, sender
			, idxS
			, signal
			, receiver
			, idxR
			, method
		);
#endif
	}

	signalName.prepend("2");
	methodName.prepend("1");
	bool ok = QObject::connect(sender, qPrintable(signalName), receiver, qPrintable(methodName));
	lua_pushboolean(L, ok);
	return 1;
}

static int lqtL_metaObject(lua_State *L) {

    QObject* self = static_cast<QObject*>(lqtL_toudata(L, 1, "QObject*"));
    if (self == NULL)
        return luaL_argerror(L, 1, "expecting QObject*");

	lqtL_pushudata(L, self->metaObject(), "QMetaObject*");
	return 1;
}

int lqtL_setErrorHandler(lua_State *L);

void lqtL_qobject_custom (lua_State *L) {
    lua_getfield(L, LUA_REGISTRYINDEX, "QObject*");
    int qobject = lua_gettop(L);

    lqtL_embed(L);

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

    lua_pushstring(L, "metaObject");
    lua_pushcfunction(L, lqtL_metaObject);
    lua_rawset(L, qobject);

    // also modify the static QObject::connect function
    lua_getfield(L, -2, "QObject");
    lua_pushcfunction(L, lqtL_connect);
    lua_setfield(L, -2, "connect");
    // set QtCore.setErrorHandler function
    lua_pushcfunction(L, lqtL_setErrorHandler);
    lua_setfield(L, -4, "setErrorHandler");

	reg_qt_function(L, "qt.inherit", lua_class_inherit);
	reg_qt_function(L, "qt.cast", lua_class_cast);
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


//qtlua------------------------------------------------------------------------------

static int lua_class_cast(lua_State *L)
{

	/* stack: userdata, string */
	if (lua_gettop(L) < 2) {
		lua_pushliteral(L, "Miss arguments to iskindof.");
		lua_error(L);
		return 0;
	}
	if (!lua_isuserdata(L, 1)) {
		lua_pushliteral(L, "Invalid argument #1 to qt.cast: userdata expected.");
		lua_error(L);
		return 0;
	}
	if (!lua_isstring(L, 2)) {
		lua_pushliteral(L, "Invalid argument #2 to qt.cast: string expected.");
		lua_error(L);
		return 0;
	}
	const char *name = lua_tostring(L, 2);
	lua_pushvalue(L, 1);
	luaL_getmetatable(L, name);
	lua_setmetatable(L, -2);

	return 1;
}

static int lua_class_inherit(lua_State *L)
{
	/*stack: name, base*/
	if (lua_gettop(L) < 2) {
		lua_pushliteral(L, "Mess arguments to newclass.");
		lua_error(L);
		return 0;
	}

	const char *name = luaL_checkstring(L, 1);
	if (!name) {
		lua_pushliteral(L, "Invalid argument #1 to inherit: string expected.");
		lua_error(L);
		return 0;
	}

	luaL_getmetatable(L, name);						/*stack: mt*/
	if (!lua_isnil(L, -1))
	{
		lua_pop(L, 1);								/*stack: */
		luaL_error(L, "qt.inherit try to create an existed class [%s].", name);
		return 0;
	}
	lua_pop(L, 1);									/*stack: */

	if (!lua_getmetatable(L, 2)) {
		lua_pushliteral(L, "Invalid argument #2 to inherit: class expected.");
		lua_error(L);
		return 0;
	}

	lua_gettable(L, LUA_REGISTRYINDEX);
	const char *base = luaL_checkstring(L, -1);

	if (!base) {
		lua_pop(L, 1);
		lua_pushliteral(L, "Invalid argument #2 to inherit: class expected.");
		lua_error(L);
		return 0;
	}
	lua_pop(L, 1);

	lqt_Base lqt_base[] = {
		{base, 0},
		{NULL, 0}
	};
	lqtL_createclass(L, name, NULL, NULL, NULL, NULL, lqt_base);

	return 1;
}

static void reg_qt_function(lua_State* L, const char *name, lua_CFunction f)
{
	set_global(L, name, f);
}

static void set_global(lua_State* L, const QtLua::String &name, lua_CFunction f)
{
#if LUA_VERSION_NUM < 502
set_global_r(L, name, f, LUA_GLOBALSINDEX);
#else
try {
	lua_pushglobaltable(L);
	set_global_r(L, name, f, lua_gettop(L));
	lua_pop(L, 1);
}
catch (...) {
	lua_pop(L, 1);
	throw;
}
#endif
}

static void set_global_r(lua_State* L, const QtLua::String &name, lua_CFunction f, int tblidx)
{
	int len = name.indexOf('.', 0);

	if (len < 0)
	{
		// set function in table if last
		lua_pushstring(L, name.constData());
		lua_pushcfunction(L, f);

		try {
			lua_psettable(L, tblidx);
		}
		catch (...) {
			lua_pop(L, 2);
			throw;
		}
	}
	else
	{
		// find intermediate value in path
		QtLua::String prefix(name.mid(0, len));
		lua_pushstring(L, prefix.constData());

		try {
			lua_pgettable(L, tblidx);
		}
		catch (...) {
			lua_pop(L, 1);
			throw;
		}

		if (lua_isnil(L, -1))
		{
			// create intermediate table
			lua_pop(L, 1);
			lua_pushstring(L, prefix.constData());
			lua_newtable(L);

			try {
				set_global_r(L, name.mid(len + 1), f, lua_gettop(L));
				lua_psettable(L, tblidx);
			}
			catch (...) {
				lua_pop(L, 2);
				throw;
			}
		}
		else if (lua_istable(L, -1))
		{
			// use existing intermediate table
			try {
				set_global_r(L, name.mid(len + 1), f, lua_gettop(L));
				lua_pop(L, 1);
			}
			catch (...) {
				lua_pop(L, 1);
				throw;
			}
		}
		else
		{
			// bad existing intermediate value
			lua_pop(L, 1);
			QTLUA_THROW(QtLua::State, "Can not set the global, the `%' key already exists.", .arg(name));
		}
	}
}

static int lua_settable_wrapper(lua_State *st)
{
	lua_settable(st, 1);
	return 0;
}

static void lua_psettable(lua_State *st, int index)
{
	if (lua_type(st, index) == LUA_TTABLE)
	{
		if (!lua_getmetatable(st, index))
			return lua_rawset(st, index);
		lua_pop(st, 1);
	}

	lua_pushcfunction(st, lua_settable_wrapper);
	if (index < 0
#if LUA_VERSION_NUM < 502
		&& index != LUA_GLOBALSINDEX
#endif
		)
		index--;
	lua_pushvalue(st, index);  // table
	lua_pushvalue(st, -4);     // key
	lua_pushvalue(st, -4);     // value
	if (lua_pcall(st, 3, 0, 0))
	{
		QtLua::String err(lua_tostring(st, -1));
		lua_pop(st, 1);
		throw err;
	}
	lua_pop(st, 2);     // remove key/value
}

static int lua_gettable_wrapper(lua_State *st)
{
	lua_gettable(st, 1);
	return 1;
}

static void lua_pgettable(lua_State *st, int index)
{
	if (lua_type(st, index) == LUA_TTABLE)
	{
		if (!lua_getmetatable(st, index)) {
			lua_rawget(st, index);
			return;
		}
		lua_pop(st, 1);
	}

	lua_pushcfunction(st, lua_gettable_wrapper);
	if (index < 0
#if LUA_VERSION_NUM < 502
		&& index != LUA_GLOBALSINDEX
#endif
		)
		index--;
	lua_pushvalue(st, index);  // table
	lua_pushvalue(st, -3);     // key
	if (lua_pcall(st, 2, 1, 0))
	{
		QtLua::String err(lua_tostring(st, -1));
		lua_pop(st, 1);
		throw err;
	}
	lua_remove(st, -2);     // replace key by value
}