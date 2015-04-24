


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

#define sprintf_s snprintf

// Hand-crafted codes.

struct NFix
{
    fix16 value;
};

static void _pushNFix(lua_State *S, fix16 v)
{
    NFix *fp = (NFix*)lua_newuserdata(S, sizeof(NFix));
    luaL_getmetatable(S, _METAFPN_);
    lua_setmetatable(S, -2);
    fp->value = v;
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
    _pushNFix(S, p0->value __OP__ p1->value);\
    return 1;\
}

#define _OpNone(__NAME__)\
static int _fp##__NAME__(lua_State *S)\
{\
    luaL_error(S, "Not implemented for this type: " #__NAME__);\
    return 0;\
}

static int _fpUnm(lua_State *S)
{
    if(lua_gettop(S)!=1){
        luaL_error(S, "Error count of parameter for -");
    }
    NFix *p0 = (NFix*)luaL_checkudata(S, 1, _METAFPN_);
    _pushNFix(S, - p0->value);
    return 1;
}

_BinOp(+, Add)
_BinOp(-, Sub)
_BinOp(*, Mult)
_BinOp(/, Div)
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
    lua_pushboolean(S,  p0->value __OP__ p1->value );\
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
    sprintf_s(buff, sizeof(buff), "fix(%f)", float(p0->value));
    lua_pushstring(S, buff);
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
    fix16 _n = n;
    fix16 _d = d;
    fix16 v = _n / _d;
    NFix *fp = (NFix*)lua_newuserdata(S, sizeof(NFix));
    luaL_getmetatable(S, _METAFPN_);
    if(lua_isnil(S, -1)){
        luaL_error(S, "No such metatable:%s", _METAFPN_);
    }
    lua_setmetatable(S, -2);
    // Assignment
    fp->value = v;
    return 1;
}   

static luaL_Reg _funcs[]={
    {"createFix", _createFix},
    {NULL, NULL}
};

extern "C" int luaopen_fp(lua_State *S)
{
    //The types
    luaL_newmetatable(S, _METAFPN_);
    lua_pushvalue(S, -1);
    lua_setfield(S, -2, "__index");
    luaL_register(S, NULL, _NFixEntries);

    //~ And the libs
    luaL_register(S, "func", _funcs);
    return 1;       //Registered done.
}
