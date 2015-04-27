
extern "C"{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include <cstdio>
#include <cstdlib>
#include "fp/fixed_func.h"
#include "fp/fixed_class.h"

#define _METAFPN_      "_METAFPN_XX"

typedef fixedpoint::fixed_point<16> fix16;

#if !defined(_MSV_VER)
#define sprintf_s snprintf
#endif

// Hand-crafted codes.

struct NFix
{
    fix16 val;
};

static void _pushNFix(lua_State *S, fix16 v)
{
    NFix *fp = (NFix*)lua_newuserdata(S, sizeof(NFix));
    luaL_getmetatable(S, _METAFPN_);
    lua_setmetatable(S, -2);
    fp->val = v;
}

#define _BinOp(__OP__, __NAME__)\
static int _fp##__NAME__(lua_State *S)\
{\
    int top = lua_gettop(S);\
    if(top<2){\
        luaL_error(S, "Error parameter count for " #__NAME__);\
    }\
    NFix *p0 = (NFix*)luaL_checkudata(S, 1, _METAFPN_);\
    NFix *p1 = (NFix*)luaL_checkudata(S, 2, _METAFPN_);\
    _pushNFix(S, p0->val __OP__ p1->val);\
    return 1;\
}

#define _OpNone(__NAME__)\
static int _fp##__NAME__(lua_State *S)\
{\
    luaL_error(S, "Not implemented for this type: " #__NAME__);\
    return 0;\
}

static int _fpUnm(lua_State *S){
#if 0
    if(lua_gettop(S)!=2){
				printf("%d\n", lua_gettop(S));
        luaL_error(S, "Error count of parameter for -(unm)");
    }
#endif

#if 0
		do {
			const char *p1 = lua_typename(S, lua_type(S, 1));
			printf("%s\n", (p1 ? p1 :""));
			const char *p2 = lua_typename(S, lua_type(S, 2));
			printf("%s\n", (p2 ? p2 :""));
			//And they are the userdata
			NFix *ptr0 = (NFix*)luaL_checkudata(S, 1, _METAFPN_);
			NFix *ptr1 = (NFix*)luaL_checkudata(S, 2, _METAFPN_);
			printf("%p,  %p\n", ptr0, ptr1);
		}while(0);
#endif 

    NFix *p0 = (NFix*)luaL_checkudata(S, 1, _METAFPN_);
    _pushNFix(S, - p0->val);
    return 1;
}

_BinOp(+, Add)
_BinOp(-, Sub)
_BinOp(*, Mult)
//_BinOp(/, Div)

static int _fpDiv(lua_State *S){
	if(lua_gettop(S)!=2){
		luaL_error(S, "Error parameter count for div");
	}
	NFix* p0 = (NFix*)luaL_checkudata(S, 1, _METAFPN_);
	NFix* p1 = (NFix*)luaL_checkudata(S, 2, _METAFPN_);
	if(0 == float(p1->val)){
		luaL_error(S, "Divisor is zero");
	}
	_pushNFix(S, p0->val / p1->val);
	return 1; 
}

_OpNone(Mod)
_OpNone(Pow)


#define _CmpOp(__OP__, __NAME__)\
static int _fp##__NAME__(lua_State *S)\
{\
    if(lua_gettop(S)!=2){\
        luaL_error(S, "Wrong number for comparison operator");\
    }\
    NFix *p0 = (NFix*)luaL_checkudata(S, 1, _METAFPN_);\
    NFix *p1 = (NFix*)luaL_checkudata(S, 2, _METAFPN_);\
    lua_pushboolean(S,  p0->val __OP__ p1->val );\
    return 1;\
}

_CmpOp(==, Eq)
_CmpOp(< , Lt)
_CmpOp(<=, Le)

_OpNone(Len)

static int _fpToString(lua_State *S)
{
    if(lua_gettop(S) != 1){
        luaL_error(S, "Wrong number for fp-to-string");
    }
    NFix *p0 = (NFix*)luaL_checkudata(S, 1, _METAFPN_);
    char buff[BUFSIZ];
    sprintf_s(buff, sizeof(buff), "fix(%f)", float(p0->val));
    lua_pushstring(S, buff);
    return 1;
}

static int _fpIntValue(lua_State *S){
	if(lua_gettop(S)!=1){
		luaL_error(S, "Error paramter count");
	}
	NFix* p0 = (NFix*)luaL_checkudata(S, 1, _METAFPN_);
	lua_pushinteger(S,p0->val.intValue);
	return 1;
}

static luaL_Reg _NFixEntries[]={
    {"__add", _fpAdd},
    {"__sub", _fpSub},
    {"__mul", _fpMult},
    {"__div", _fpDiv},
    {"__mod", _fpMod},
    {"__unm", _fpUnm},
    {"__pow", _fpPow},
    {"__eq",  _fpEq},
    {"__lt",  _fpLt},
    {"__le",  _fpLe},
    {"__len", _fpLen},
    {"__tostring", _fpToString},
		{"intValue", _fpIntValue},
    {NULL, NULL}
};

static int _createFix(lua_State *S)
{
    int top = lua_gettop(S);
    if(top < 2 ){
        luaL_error(S, "Cannot create with parameter. Need N/D");
    }
    //~ represented as N/D
    int n = luaL_checkint(S, 1);
    int d = luaL_checkint(S, 2);
		if(0==d){
			luaL_error(S, "Denominator cannot be zero");
		}
    fix16 _n = n;
    fix16 _d = d;
    fix16 v = _n / _d;
		_pushNFix(S, v);
    return 1;
}   

static int _createFix0(lua_State *S)
{
	float v = luaL_checknumber(S, 1);
	_pushNFix(S, v);
	return 1;
}

static luaL_Reg _funcs[]={
    {"CreateFix", _createFix},
		{"CreateFix0", _createFix0},
    {NULL, NULL}
};

extern "C" int luaopen_fp(lua_State *S) {
    //The types
		//printf("%d\n", lua_gettop(S));
		const int top = lua_gettop(S);
    luaL_newmetatable(S, _METAFPN_);
    lua_pushvalue(S, -1);
    lua_setfield(S, -2, "__index");
    luaL_register(S, NULL, _NFixEntries);
		lua_pop(S, 1);   // Pop the meta out
		//printf("%d\n", lua_gettop(S));
		if(lua_gettop(S) != top){
			fprintf(stderr, "Error stack balance is compromised\n");
			luaL_error(S, "Error stack corrupted");
		}
    //~ And the libs
    luaL_register(S, "func", _funcs);
    return 1;       //Registered done.
}
