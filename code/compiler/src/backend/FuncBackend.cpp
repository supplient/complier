#include "FuncBackend.h"

#include <algorithm>

#include "symbol.h"
#include "log.h"

FuncBackend::FuncBackend(NameTable &tab, const FuncTuple *func_tuple)
    :func_tuple(func_tuple)
{
    back::size offset = 0;

    // global regs
    // TODO

    // $ra
    ra_offset = offset;
    offset += SizeUtil::regSize();

    // temp var & local var
    // we treat temp var as local var
    FuncNameTable *func_tab = tab.getFuncNameTable(func_tuple->func_entry->name);
    vector<VarEntry*> lv_list = func_tab->getUnconstVars();
    for(VarEntry *lv: lv_list){
        if(!lv)
            throw string("FuncBackend: NULL local variable!");

        if(lv->is_param)
            continue; // param is not processed here.

        lvo_tab[NameUtil::genEntryName(lv)] = offset;

        // Note: we save char in one word, just the same as int
        if(lv->dim == 0){
            // normal var
            offset += SizeUtil::regSize();
        }
        else{
            // array var
            offset += SizeUtil::regSize() * lv->dim;
        }
    }

    // Save stack size
    stack_size = offset;

    // param
    vector<VarEntry*> param_list = func_tuple->func_entry->param_list;
    for(VarEntry *pv: param_list){
        if(!pv)
            throw string("FuncBackend: NULL param!");

        lvo_tab[NameUtil::genEntryName(pv)] = offset;
        offset += SizeUtil::regSize(); // no param can be an array.
    }

    // DEBUG
    mylog::debug << "Func " + func_tuple->func_entry->name + " 's lvo_tab.";
    for(auto pair: lvo_tab){
        mylog::debug << pair.first + ": " + to_string(pair.second);
    }
}

bool isGlobalVar(const VarEntry *entry){
    return entry->getOwnerName() == sem::GLOBAL_FUNC_NAME;
}

void FuncBackend::writeBackVar(const VarEntry *entry, back::REG reg, vector<InstCmd*> *inst_cmds){
    InstCmd *write_cmd = new InstCmd();
    switch(entry->type){
        case sym::INT:
            write_cmd->op = InstCmd::SW;
            break;
        case sym::CHAR:
            write_cmd->op = InstCmd::SB;
            break;
        default:
            throw string("FuncBackend.writeBackVar: invalid entry's type: " + to_string(entry->type));
    }
    write_cmd->res_reg = reg;
    if(isGlobalVar(entry)){
        write_cmd->left_reg = back::NO_REG;
        write_cmd->right_type = InstCmd::LABEL_TYPE;
        write_cmd->right_label = NameUtil::genEntryName(entry);
    }
    else{
        write_cmd->left_reg = back::sp;
        write_cmd->right_type = InstCmd::IMME_TYPE;
        write_cmd->right_imme = lvo_tab[NameUtil::genEntryName(entry)];
    }

    inst_cmds->push_back(write_cmd);
}

back::REG FuncBackend::registVar(const VarEntry *entry, vector<InstCmd*>* inst_cmds){
    back::REG res;

    // check if has registed
    res = reg_pool.lookUpReg(entry);
    if(res != back::NO_REG)
        return res;

    // ask for a reg
    const VarEntry *out_entry = NULL;
    res = reg_pool.regist(entry, &out_entry);

    if(out_entry){
        // some var has been unregisted
        // write back out_entry
        writeBackVar(out_entry, res, inst_cmds);
    }
    return res;
}

back::REG FuncBackend::registAndLoadVar(const VarEntry *entry, vector<InstCmd*> *inst_cmds){
    back::REG res;

    // check if has registed
    res = reg_pool.lookUpReg(entry);
    if(res != back::NO_REG)
        return res;

    // regist if not registed
    res = registVar(entry, inst_cmds);

    // load var's value into the registed reg.
    InstCmd *read_cmd = new InstCmd();
    switch(entry->type){
        case sym::INT:
            read_cmd->op = InstCmd::LW;
            break;
        case sym::CHAR:
            read_cmd->op = InstCmd::LB;
            break;
        default:
            throw string("FuncBackend.registAndLoadVar: invalid entry's type: " + to_string(entry->type));
    }
    read_cmd->res_reg = res;
    if(isGlobalVar(entry)){
        read_cmd->left_reg = back::NO_REG;
        read_cmd->right_type = InstCmd::LABEL_TYPE;
        read_cmd->right_label = NameUtil::genEntryName(entry);
    }
    else{
        read_cmd->left_reg = back::sp;
        read_cmd->right_type = InstCmd::IMME_TYPE;
        read_cmd->right_imme = lvo_tab[NameUtil::genEntryName(entry)];
    }

    inst_cmds->push_back(read_cmd);

    return res;
}

void FuncBackend::trans(map<string, string> str_tab,
    vector<DataCmd*> *data_cmds, vector<InstCmd*> *inst_cmds){

    for(Tuple *tuple: func_tuple->tuples){
        transTuple(tuple, str_tab, data_cmds, inst_cmds);
    }
}

void FuncBackend::transTuple(Tuple *tuple, map<string, string> str_tab,
    vector<DataCmd*> *data_cmds, vector<InstCmd*> *inst_cmds){
    using namespace sem;

    map<back::REG, const VarEntry*> reg_entry_map;
    InstCmd::OP inst_op;
    back::REG res_reg;
    back::REG left_reg;
    back::REG right_reg;
    switch(tuple->op){
        case ASSIGN:
            res_reg = registVar(tuple->res->entry, inst_cmds);

            if(tuple->left->type == Operand::ENTRY){
                // move
                left_reg = registAndLoadVar(tuple->left->entry, inst_cmds);
                inst_cmds->push_back(
                    new InstCmd(InstCmd::MOVE, res_reg, left_reg)
                );
            }
            else if(tuple->left->type == Operand::INT_CONST){
                // add
                inst_cmds->push_back(
                    new InstCmd(InstCmd::ADD, res_reg, back::zero, tuple->left->int_const)
                );
            }
            else if(tuple->left->type == Operand::CHAR_CONST){
                // add
                inst_cmds->push_back(
                    new InstCmd(InstCmd::ADD, res_reg, back::zero, tuple->left->char_const)
                );
            }
            break;

        case ADD:
        case SUB:
        case MUL:
        case DIV:
        case LESS:
        case LESSOREQUAL:
        case MORE:
        case MOREOREQUAL:
        case NOTEQUAL:
        case EQUAL:
            switch(tuple->op){
                case ADD: inst_op = InstCmd::ADD; break;
                case SUB: inst_op = InstCmd::SUB; break;
                case MUL: inst_op = InstCmd::MUL; break;
                case DIV: inst_op = InstCmd::DIV; break;
                case LESS: inst_op = InstCmd::SLT; break;
                case LESSOREQUAL: inst_op = InstCmd::SLE; break;
                case MORE: inst_op = InstCmd::SGT; break;
                case MOREOREQUAL:  inst_op = InstCmd::SGE; break;
                case NOTEQUAL:  inst_op = InstCmd::SNE; break;
                case EQUAL: inst_op = InstCmd::SEQ; break;
                default: throw string("FuncBackend: something wrong.");
            }
            // add
            left_reg = registAndLoadVar(tuple->left->entry, inst_cmds);
            res_reg = registVar(tuple->res->entry, inst_cmds);
            if(tuple->right->type == Operand::ENTRY){
                right_reg = registAndLoadVar(tuple->right->entry, inst_cmds);
                inst_cmds->push_back(
                    new InstCmd(inst_op, res_reg, left_reg, right_reg)
                );
            }
            else if(tuple->right->type == Operand::INT_CONST){
                inst_cmds->push_back(
                    new InstCmd(inst_op, res_reg, left_reg, tuple->right->int_const)
                );
            }
            else if(tuple->right->type == Operand::CHAR_CONST){
                inst_cmds->push_back(
                    new InstCmd(inst_op, res_reg, left_reg, tuple->right->char_const)
                );
            }
            break;

        case FUNC:
            // func label
            inst_cmds->push_back(new InstCmd(tuple->left->entry->name));
            // adjust $sp
            inst_cmds->push_back(
                new InstCmd(InstCmd::ADD, back::sp, back::sp, -stack_size)
            );
            // save $ra
            inst_cmds->push_back(
                new InstCmd(InstCmd::SW, back::ra, back::sp, ra_offset)
            );
            // save $s?
            // TODO
            break;

        case RET:
            // save all global vars
            reg_entry_map = reg_pool.getAllRegistedVar();
            for(auto pair: reg_entry_map){
                back::REG reg = pair.first;
                const VarEntry *entry = pair.second;
                if(!isGlobalVar(entry))
                    continue;
         
                writeBackVar(entry, reg, inst_cmds);
            }
            // load $s?
            // TODO
            // load $ra
            inst_cmds->push_back(
                new InstCmd(InstCmd::LW, back::ra, back::sp, ra_offset)
            );
            // recover $sp
            inst_cmds->push_back(
                new InstCmd(InstCmd::ADD, back::sp, back::sp, stack_size)
            );
            // jump return
            inst_cmds->push_back(
                new InstCmd(InstCmd::JR, back::ra)
            );
            break;

        default:
            throw string("FuncBackend: Not implemented tuple OP: " + to_string(tuple->op));
    }
}
