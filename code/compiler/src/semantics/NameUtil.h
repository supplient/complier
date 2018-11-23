#ifndef NAME_UTIL_H
#define NAME_UTIL_H

#include <string>

using namespace std;

class NameTableEntry;

class NameUtil
{
public:
    static string genFuncLabel(const string &func_name){
        return "$" + func_name;
    }

    static string genEntryName(const NameTableEntry *entry);

    static string genTempVarName(int index){
        return "#" + to_string(index);
    }
};

#endif//NAME_UTIL_H