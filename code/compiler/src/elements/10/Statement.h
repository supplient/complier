#ifndef STATEMENT_H
#define STATEMENT_H

#include "Element.h"
#include "Expression.h"

class StatementList;

class Statement: public Element
{
    public:

};

class IfStatement: public Statement
{

};

class WhileStatement: public Statement
{

};

class FuncCallStatement: public Statement
{

};

class AssignStatement: public Statement
{
public:
    string ident;
    Expression *exp;

    bool is_array;
    Expression *select;
};

class InputStatement: public Statement
{
public:
    vector<string> ident_list;
};

class OutputStatement: public Statement
{
public:
    OutputStatement():Statement(){
        has_string=false;
        }
    bool has_string;
    string str_value;
    Expression *exp_value;
};

class ReturnStatement: public Statement
{
public:
    Expression *exp;
};

class EmptyStatement: public Statement
{

};

class BracedStatement: public Statement
{
public:
    StatementList *state_list;
};

#endif//STATEMENT_H