#include "lqt_qt_utils.hpp"
#include "lqt_function.hpp"

static int qt_slot_from_string (lua_State *L) {
	lua_pushstring(L, "1");
	lua_pushvalue(L, 1);
	lua_concat(L, 2);
	return 1;
}

static int qt_signal (lua_State *L) {
	lua_pushstring(L, "2");
	lua_pushvalue(L, 1);
	lua_concat(L, 2);
	return 1;
}

static int qt_derive (lua_State *L) {
	if (!lua_isuserdata(L, 1) || !lua_getmetatable(L, 1)) {
		lua_pushnil(L);
		lua_pushstring(L, "no userdata or no metatable given");
		return 2;
	}
	lua_getfield(L, -1, "__qtype");
	if (!lua_isstring(L, -1)) {
		lua_pushnil(L);
		lua_pushstring(L, "not a Qt type");
		return 2;
	}
	lua_insert(L, -2);
	lua_newtable(L);
	lua_insert(L, -3);
	lua_settable(L, -3);
	lua_newtable(L);
	lua_insert(L, -2);
	lua_setfield(L, -2, "__base");
	lua_pushcfunction(L, lqtL_index);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, lqtL_newindex);
	lua_setfield(L, -2, "__newindex");
	lua_setmetatable(L, 1);
	return 0;
}

static int qt_slot_from_function (lua_State *L) {
	lua_pushvalue(L, 1);
	LuaFunction *f = new LuaFunction(L);
	f = 0;
	lua_pushstring(L, "1function()");
	return 2;
}

static int qt_slot (lua_State *L) {
	int ret = 0;
	if (lua_type(L, 1)==LUA_TSTRING) {
		ret = qt_slot_from_string(L);
	} else if (lua_type(L, 1)==LUA_TFUNCTION) {
		ret = qt_slot_from_function(L);
	}
	return ret;
}

static luaL_Reg libqt[] = {
	{ "slot", qt_slot },
	{ "signal", qt_signal },
	{ "derive", qt_derive },
	{ 0, 0 },
};

int luaopen_qt (lua_State *L) {
	luaL_register(L, "qt", libqt);
	return 1;
}

