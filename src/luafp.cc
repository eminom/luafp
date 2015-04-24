


extern "C"{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include <cstdio>
#include <cstdlib>
#include "fp/fixed_func.h"
#include "fp/fixed_class.h"

#define MetaFixPointNumber      "MetaFixPointNumberXX"

typedef fixedpoint::fixed_point<16> fix16;

#define sprintf_s snprintf

// Hand-crafted codes.

struct FixNumber
{
    fix16 value;
};

static void _pushFixNumber(lua_State *S, fix16 v)
{
    FixNumber *fp = (FixNumber*)lua_newuserdata(S, sizeof(FixNumber));
    luaL_getmetatable(S, MetaFixPointNumber);
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
    FixNumber *p0 = (FixNumber*)luaL_checkudata(S, 1, MetaFixPointNumber);\
    FixNumber *p1 = (FixNumber*)luaL_checkudata(S, 2, MetaFixPointNumber);\
    _pushFixNumber(S, p0->value __OP__ p1->value);\
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
    FixNumber *p0 = (FixNumber*)luaL_checkudata(S, 1, MetaFixPointNumber);
    _pushFixNumber(S, - p0->value);
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
    FixNumber *p0 = (FixNumber*)luaL_checkudata(S, 1, MetaFixPointNumber);\
    FixNumber *p1 = (FixNumber*)luaL_checkudata(S, 2, MetaFixPointNumber);\
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
    FixNumber *p0 = (FixNumber*)luaL_checkudata(S, 1, MetaFixPointNumber);
    char buff[BUFSIZ];
    sprintf_s(buff, sizeof(buff), "fix(%f)", float(p0->value));
    lua_pushstring(S, buff);
    return 1;
}

static luaL_Reg _FixNumberEntries[]={
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
    FixNumber *fp = (FixNumber*)lua_newuserdata(S, sizeof(FixNumber));
    luaL_getmetatable(S, MetaFixPointNumber);
    if(lua_isnil(S, -1)){
        luaL_error(S, "No such metatable:%s", MetaFixPointNumber);
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
    luaL_newmetatable(S, MetaFixPointNumber);
    lua_pushvalue(S, -1);
    lua_setfield(S, -2, "__index");
    luaL_register(S, NULL, _FixNumberEntries);

    //~ And the libs
    luaL_register(S, "func", _funcs);
    return 1;       //Registered done.
}
