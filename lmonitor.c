#include <lua.h>
#include <lauxlib.h>
#include <string.h>
#include <stdlib.h>

struct status {
	int max_depth;
	int depth;
	int calls;
};

static struct status * G = NULL;

static void
monitor_init(struct status * st) {
	memset(st, 0, sizeof(*st));
}

static void 
monitor_depth(lua_State *L, lua_Debug *ar) {
	struct status * s = G;
	switch (ar->event) {
	case LUA_HOOKCALL:
	case LUA_HOOKTAILCALL:
		if (++s->depth > s->max_depth) {
			++s->max_depth;
		}
		break;
	case LUA_HOOKRET:
		--s->depth;
		++s->calls;
		break;
	}
}

static int
ldepth(lua_State *L) {
	monitor_init(G);
	luaL_checktype(L, 1, LUA_TFUNCTION);
	lua_sethook(L, monitor_depth, LUA_MASKCALL | LUA_MASKRET, 0);
	int args = lua_gettop(L) - 1;
	lua_call(L, args, 0);
	lua_sethook(L, NULL, 0 , 0);
	lua_pushinteger(L, G->max_depth);
	lua_pushinteger(L, G->calls);
	return 2;
}

int
luaopen_monitor(lua_State *L) {
	luaL_checkversion(L);
	luaL_Reg l[] = {
		{ "depth", ldepth },
		{ NULL, NULL },
	};
	luaL_newlib(L,l);
	struct status * s = malloc(sizeof(*G));
	G = s;
	return 1;
}
