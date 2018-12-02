#ifndef BASIC_BLOCK_SPLITTER_H
#define BASIC_BLOCK_SPLITTER_H

#include "FuncBlock.h"
#include "FuncTuple.h"

class BasicBlockSplitter
{
public:
    static vector<FuncBlock*> work(vector<FuncTuple*> func_tuples){
        vector<FuncBlock*> func_blocks;
        for(FuncTuple *func_tuple: func_tuples){
            func_blocks.push_back(workOnFunc(func_tuple));
        }
        return func_blocks;
    }

    static FuncBlock* workOnFunc(FuncTuple *func_tuple){
        FuncBlock *func_block = new FuncBlock(func_tuple->func_entry);
        unsigned int index = 0;
        BasicBlock *block = new BasicBlock(index++);
        bool has_started = false;

        func_block->blocks.push_back(block);

        // split and construct basic blocks
        // link normal edge
        for(Tuple *tuple: func_tuple->tuples){
            if(isLabelTuple(tuple) || isFuncTuple(tuple)){
                if(!has_started){
                    block->labels.push_back(tuple->res->str_value);
                    block->tuples.push_back(tuple);
                }
                else{
                    has_started = false;
                    BasicBlock *new_block = new BasicBlock(index++);
                    func_block->blocks.push_back(new_block);
                    new_block->labels.push_back(tuple->res->str_value);
                    new_block->tuples.push_back(tuple);
                    addEdgeBetween(block, new_block, Edge::NORMAL);
                    block = new_block;
                }
                if(isFuncTuple(tuple))
                    func_block->enter_block = block;
                continue;
            }

            has_started = true;

            if(isJumpTuple(tuple) || isBranchTuple(tuple) || isCallTuple(tuple)){
                block->tuples.push_back(tuple);
                has_started = false;
                BasicBlock *new_block = new BasicBlock(index++);
                func_block->blocks.push_back(new_block);
                if(isBranchTuple(tuple) || isCallTuple(tuple))
                    addEdgeBetween(block, new_block, Edge::NORMAL);
                block = new_block;
            }
            else if(isReturnTuple(tuple)){
                block->tuples.push_back(tuple);
                has_started = false;
                BasicBlock *new_block = new BasicBlock(index++);
                func_block->blocks.push_back(new_block);
                func_block->exit_blocks.push_back(block);
                block = new_block;
            }
            else
                block->tuples.push_back(tuple);
        }//for(Tuple *tuple: func_tuple->tuples){

        // Remove the last empty block
        block = func_block->blocks.back();
        delete block;
        func_block->blocks.pop_back();

        // link jmp edges and branch edges
        // TODO

        return func_block;
    }

private:
    static void addEdgeBetween(BasicBlock *from, BasicBlock *to, Edge::TYPE type){
        Edge *edge = new Edge(from, to, type);
        from->out_edges.push_back(edge);
        to->in_edges.push_back(edge);
    }

    static bool isLabelTuple(Tuple *tuple){
        return tuple->op==sem::LABEL;
    }

    static bool isFuncTuple(Tuple *tuple){
        return tuple->op==sem::FUNC;
    }

    static bool isJumpTuple(Tuple *tuple){
        return tuple->op==sem::JMP;
    }

    static bool isBranchTuple(Tuple *tuple){
        switch(tuple->op){
            case sem::BEQ:
            case sem::BEZ:
                return true;
            default:
                return false;
        }
    }

    static bool isCallTuple(Tuple *tuple){
        return tuple->op == sem::CALL;
    }

    static bool isReturnTuple(Tuple *tuple){
        return tuple->op == sem::RET;
    }
};

#endif//BASIC_BLOCK_SPLITTER_H
