#ifndef DAG_BUILDER_H
#define DAG_BUILDER_H

#include "Node.h"
#include "log.h"
#include "NameTable.h"

#include <map>

namespace dag{
    class Builder
    {
    public:
        Builder(){
            reset();
        }

        vector<Node*> work(NameTable & tab, Tuples tuples){
            // In: should be tuples in a basic block
            //      without head's label tuples
            //      and tail's jump tuples
            // Out: is the DAG tree's all nodes
            reset();

            Node *left;
            Node *mid;
            Node *right;

            OpNode *op_node;

            // Build DGA graph
            for(Tuple *tuple: tuples){

                switch(tuple->op){
                    case sem::ASSIGN:
                        left = getNodeAndFillTab(tuple->left);
                        assignVarToNode(left, tuple->res->entry);
                        break;

                    case sem::NEG:
                        left = getNodeAndFillTab(tuple->left);
                        op_node = getOpNodeWithOne(tuple->op, left);
                        assignVarToNode(op_node, tuple->res->entry);
                        break;

                    case sem::RARRAY:
                    case sem::SUB:
                    case sem::DIV:
                    case sem::LESS:
                    case sem::LESSOREQUAL:
                    case sem::MORE:
                    case sem::MOREOREQUAL:
                        left = getNodeAndFillTab(tuple->left);
                        right = getNodeAndFillTab(tuple->right);
                        op_node = getOpNodeWithTwo(tuple->op, left, right, true);
                        assignVarToNode(op_node, tuple->res->entry);
                        break;

                    case sem::ADD:
                    case sem::MUL:
                    case sem::NOTEQUAL:
                    case sem::EQUAL:
                        left = getNodeAndFillTab(tuple->left);
                        right = getNodeAndFillTab(tuple->right);
                        op_node = getOpNodeWithTwo(tuple->op, left, right, false);
                        assignVarToNode(op_node, tuple->res->entry);
                        break;

                    case sem::WARRAY:
                        left = getNodeAndFillTab(tuple->left);
                        mid = getNodeAndFillTab(tuple->res);
                        right = getNodeAndFillTab(tuple->right);
                        op_node = getOpNodeWithThree_order(tuple->op, left, mid, right);
                        if(op_node)
                            throw string("dag::Builder.WARRAY: a op_node is found, while it is impossible to find a calculated node for array.");
                        op_node = new OpNode(tuple->op, left, mid, right);
                        nodes.push_back(op_node);
                        op_node->addVar(tuple->res->entry);
                        var_tab[tuple->res->entry] = op_node;
                        break;

                    case sem::PARAM:
                        left = getNodeAndFillTab(tuple->left);
                        mid = getNodeAndFillTab(tuple->res);
                        right = getNodeAndFillTab(tuple->right);
                        op_node = new OpNode(stack_node, param_node, tuple->op, left, mid, right);
                        nodes.push_back(op_node);
                        stack_node = op_node;
                        param_node = op_node;
                        break;

                    case sem::INPUT:
                        op_node = new OpNode(stream_node, tuple->op);
                        nodes.push_back(op_node);
                        assignVarToNode(op_node, tuple->res->entry);
                        stream_node = op_node;
                        break;

                    case sem::OUTPUT:
                        left = NULL;
                        right = NULL;
                        if(tuple->left){
                            left = getNodeAndFillTab(tuple->left);
                        }
                        if(tuple->right){
                            right = getNodeAndFillTab(tuple->right);
                        }
                        op_node = new OpNode(stream_node, param_node, sem::OUTPUT, left, right);
                        nodes.push_back(op_node);
                        stream_node = op_node;
                        param_node = op_node;
                        break;

                    default:
                        throw string("dag::Builder.work: Not Implemented sem::op: " + to_string(tuple->op));
                }

            }

            // DEBUG
            mylog::debug << "Built DAG graph-----";
            for(Node *node: nodes)
                mylog::debug << node->toString();

            return nodes;
        }

    private:
        map<const VarEntry*, Node*> var_tab;
        map<int, Node*> int_tab;
        map<char, Node*> char_tab;
        map<string, Node*> str_tab;

        Node* param_node;
        Node* stack_node;
        Node* stream_node;

        vector<Node*> nodes; // Note: we must ensure node is inserted into nodes in its creating order to maintain old_ref

        void reset(){
            // TODO free memory
            var_tab.clear();
            int_tab.clear();
            char_tab.clear();
            str_tab.clear();

            param_node = new SpecialNode("param");
            stack_node = new SpecialNode("stack");
            stream_node = new SpecialNode("stream");

            nodes.clear();
        }

        void assignVarToNode(Node *node, const VarEntry *target_var){
            // TODO
            VarNode *var_node = dynamic_cast<VarNode*>(node);
            if(!var_node)
                throw string("dag::Builder.assignVarToNode: the node cannot be converted to VarNode.");
            var_node->addVar(target_var);
            var_tab[target_var] = var_node;
        }

        OpNode* getOpNodeWithOne(sem::OP op, Node *left){
            for(OpNode *node: left->fathers){
                if(node->op != op)
                    continue;

                if(node->left != left)
                    continue;

                return node;
            }

            OpNode *res = NULL;
            res = new OpNode(op, left);
            nodes.push_back(res);

            return res;
        }

        OpNode* getOpNodeWithTwo(sem::OP op, Node *left, Node *right, bool order){
            for(OpNode *node: left->fathers){
                if(node->op != op)
                    continue;

                if(order){
                    if(node->left != left)
                        continue;
                    if(node->right != right)
                        continue;
                }
                else{
                    if(node->left == left){
                        if(node->right != right)
                            continue;
                    }
                    else if(node->right == left){
                        if(node->left != right)
                            continue;
                    }
                    else
                        throw string("dag::Builder.getOpNodeWithTwo: though the op node is in left's fathers, left != node.left && left != node.right");
                }

                return node;
            }

            OpNode *res = NULL;
            res = new OpNode(op, left, right);
            nodes.push_back(res);

            return res;
        }

        OpNode* getOpNodeWithThree_order(sem::OP op, Node *left, Node *mid, Node *right){
            for(OpNode *node: left->fathers){
                if(node->op != op)
                    continue;
                if(node->left != left
                    || node->mid != mid
                    || node->right != right
                )
                    continue;

                return node;
            }

            OpNode *res = NULL;
            res = new OpNode(op, left, mid, right);
            nodes.push_back(res);

            return res;
        }

        Node* getNodeAndFillTab(Operand *ord){
            auto var_it = var_tab.end();
            auto int_it = int_tab.end();
            auto char_it = char_tab.end();
            auto str_it = str_tab.end();

            Node *res = NULL;
            string entry_name;

            switch(ord->type){
                case Operand::ENTRY:
                    var_it = var_tab.find(ord->entry);
                    if(var_it == var_tab.end()){
                        res = new VarNode(ord->entry);
                        var_tab[ord->entry] = res;
                        nodes.push_back(res);
                    }
                    else
                        res = var_it->second;
                    break;
                case Operand::INT_CONST:
                    int_it = int_tab.find(ord->int_const);
                    if(int_it == int_tab.end()){
                        res = new ValueNode(ord->int_const);
                        int_tab[ord->int_const] = res;
                        nodes.push_back(res);
                    }
                    else
                        res = int_it->second;
                    break;
                case Operand::CHAR_CONST:
                    char_it = char_tab.find(ord->char_const);
                    if(char_it == char_tab.end()){
                        res = new ValueNode(ord->char_const);
                        char_tab[ord->char_const] = res;
                        nodes.push_back(res);
                    }
                    else
                        res = char_it->second;
                    break;
                case Operand::STRING:
                    str_it = str_tab.find(ord->str_value);
                    if(str_it == str_tab.end()){
                        res = new StringNode(ord->str_value);
                        str_tab[ord->str_value] = res;
                        nodes.push_back(res);
                    }
                    else
                        res = str_it->second;
                    break;
                default:
                    throw string("dag::Builder.getNodeAndFillTab: Invalid Operand.type: " + to_string(ord->type));
            }

            return res;
        }
    };
}
#endif//DAG_BUILDER_H
