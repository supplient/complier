#include "Command.h"

string DataCmd::toString(){
    string s = label + ": ";
    switch(op){
        case ASCIIZ:
            s += ".asciiz \"" + str_value + "\"";
            break;
        case WORD:
            s += ".word 0"; // We don't need to assign any init value.
            break;
        case SPACE:
            s += ".space " + to_string(size);
            break;
    }
    return s;
}

string InstCmd::OP_NAME[MAX_OP_NUM]={
    "none",
    "nop",
    "la",
    "li",
    "move",
    "neg",
    "add",
    "sub",
    "mul",
    "div",
    "and",
    "sll",
    "sb",
    "lb",
    "sw",
    "lw",
    "j",
    "jal",
    "jr",
    "syscall",
    "slt",
    "sle",
    "sgt",
    "sge",
    "sne",
    "seq",
    "beqz",
    "beq",
};

string InstCmd::toString(){
    string s = "";
    if(has_label){
        s += label + ": ";
    }

    switch(op){
        case NONE:
            break;

        case NOP:
        case SYSCALL:
            s += OP_NAME[op];
            break;

        case ADD:
        case SUB:
        case MUL:
        case DIV:

        case AND:

        case SLT:
        case SLE:
        case SGT:
        case SGE:
        case SNE:
        case SEQ:
        case SLL:
        case BEQ:
            s += OP_NAME[op] + " ";
            s += rts(res_reg) + ", ";
            s += rts(left_reg) + ", ";
            s += rightToString();
            break;

        case MOVE:
        case NEG:
            s += OP_NAME[op] + " ";
            s += rts(res_reg) + ", ";
            s += rts(left_reg) + ", ";
            break;

        case BEQZ:
            s += OP_NAME[op] + " ";
            s += rts(res_reg) + ", ";
            s += right_label;
            break;

        case SB:
        case LB:
        case SW:
        case LW:
        case LA:
            s += OP_NAME[op] + " ";
            s += rts(res_reg) + ", ";
            switch(right_type){
                case IMME_TYPE:
                    s += to_string(right_imme); break;
                case LABEL_TYPE:
                    s += right_label; break;
                default:
                    throw string("InstCmd.toString: right_reg is not allowed for cmd like sw/lw.");
            }
            if(left_reg != back::NO_REG)
                s += "(" + rts(left_reg) + ")";
            break;

        case JR:
            s += OP_NAME[op] + " ";
            s += rts(res_reg);
            break;

        case J:
        case JAL:
            s += OP_NAME[op] + " ";
            switch(right_type){
                case IMME_TYPE:
                    s += to_string(right_imme); break;
                case LABEL_TYPE:
                    s += right_label; break;
                default:
                    throw string("InstCmd.toString: right_reg is not allowed for cmd like j.");
            }
            break;

        default:
            throw string("InstCmd.toString: Not implemented OP: " + to_string(op));
    }

    return s;
}
