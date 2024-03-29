#ifndef FACTOR_H
#define FACTOR_H

#include "Element.h"
#include "FuncCallExp.h"

class Expression;
class Integer;

class Factor: public Element
{
public:
    virtual Tuples dump(NameTable &tab, const string &func_name, 
            TempVarPool &tvp, Operand **ret_ord){
        throw string("Not implemented.");
        Tuples tuples;
        return tuples;
    }
};

class VarFactor: public Factor
{
public:
    bool is_array;
    string ident;
    Expression *select;

    virtual Tuples dump(NameTable &tab, const string &func_name, 
            TempVarPool &tvp, Operand **ret_ord);
};

class FuncFactor: public Factor
{
public:
    FuncCallExp *call_exp;

    Tuples dump(NameTable &tab, const string &func_name, 
            TempVarPool &tvp, Operand **ret_ord);
};

class ExpFactor: public Factor
{
public:
    Expression *exp;

    virtual Tuples dump(NameTable &tab, const string &func_name, 
            TempVarPool &tvp, Operand **ret_ord);
};

class ConstFactor: public Factor
{
public:
    bool is_char;
    Integer *int_value;
    char char_value;

    virtual Tuples dump(NameTable &tab, const string &func_name, 
            TempVarPool &tvp, Operand **ret_ord);
};

#endif//FACTOR_H