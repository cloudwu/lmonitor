#include <lua.h>
#include <lauxlib.h>
#include <string.h>
#include <stdlib.h>

#define REPORT_MAX (1024*1024)
#define FNAME_MAX 1024

struct status {
	int max_depth;
	int depth;
	int calls;
	int ptr;
	char buffer[REPORT_MAX];
};

static struct status * G = NULL;

static void
monitor_init(struct status * st) {
	st->max_depth = 0;
	st->depth = 0;
	st->calls = 0;
	st->ptr = 0;
}

static void
monitor_cat(struct status * st, const char * str, size_t sz) {
	if (sz + st->ptr > REPORT_MAX) {
		sz = REPORT_MAX - st->ptr;
	}
	memcpy(st->buffer + st->ptr, str, sz);
	st->ptr += sz;
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

static void 
monitor_report(lua_State *L, lua_Debug *ar) {
	char info[FNAME_MAX];
	struct status * s = G;
	int n;
	lua_getinfo(L, "nS", ar);
	switch (ar->event) {
	case LUA_HOOKCALL:
	case LUA_HOOKTAILCALL:
		if (++s->depth > s->max_depth) {
			++s->max_depth;
		}
		if (ar->name != NULL) {
			n = snprintf(info, FNAME_MAX, "%s(", ar->name);
		} else if (ar->linedefined < 0) {
			n = snprintf(info, FNAME_MAX, "(");
		} else {
			n = snprintf(info, FNAME_MAX, "%s:%d(", ar->short_src, ar->linedefined);
		}
		monitor_cat(s, info, n);
		break;
	case LUA_HOOKRET:
		--s->depth;
		++s->calls;
		monitor_cat(s, ")", 1);
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

static int
lreport(lua_State *L) {
	monitor_init(G);
	luaL_checktype(L, 1, LUA_TFUNCTION);
	lua_sethook(L, monitor_report, LUA_MASKCALL | LUA_MASKRET, 0);
	int args = lua_gettop(L) - 1;
	lua_call(L, args, 0);
	lua_sethook(L, NULL, 0 , 0);
	lua_pushinteger(L, G->max_depth);
	lua_pushinteger(L, G->calls);
	lua_pushlstring(L, G->buffer, G->ptr);
	return 3;
}

int
luaopen_monitor(lua_State *L) {
	luaL_checkversion(L);
	luaL_Reg l[] = {
		{ "depth", ldepth },
		{ "report", lreport },
		{ NULL, NULL },
	};
	luaL_newlib(L,l);
	struct status * s = malloc(sizeof(*G));
	G = s;
	return 1;
}
