#include "FuncDefine.h"

#include <algorithm>

#include "Param.h"
#include "CompoundStatement.h"


Tuples FuncDefine::dump(NameTable &tab){
    Tuples tuples;

    // dump param_list
    // Note: dump in the natual order
    vector<VarEntry*> param_entry_list;
    int param_count = 0;
    for(Param *param: param_list){
        VarEntry *param_entry = param->dump(tab, func_name, param_count);
        if(param_entry != NULL){
            param_entry_list.push_back(param_entry);
            param_count++;
        }
    }

    // create a tuple as the entrance
    Tuple *start_tuple = new Tuple();
    start_tuple->op = sem::FUNC;
    start_tuple->res = new Operand(func_name);
    tuples.insert(tuples.begin(), start_tuple); // should be at the head

    // fill tab
    if(!tab.insertFunc(func_name, return_type, param_entry_list, start_tuple))
        errorRepo("multiple defination for function: " + func_name);

    // dump compound_state
    Tuples state_tuples = compound_state->dump(tab, func_name);
    tuples.insert(tuples.end(), state_tuples.begin(), state_tuples.end());

    // TODO
    // check when this function need return value, if user give a return statement with return value.

    // do return implictly if the last tuple is not RET
    if(tuples.size() < 1 || tuples[tuples.size()-1]->op != sem::RET){
        Tuple *ret_tuple = new Tuple();
        ret_tuple->op = sem::RET;
        tuples.push_back(ret_tuple);
    }

    return tuples;
}
