#include "GrammarAnalyzer.h"

#include "symbol.h"

using namespace std;

Factor* GrammarAnalyzer::constructFactor(const SymSet &delimiter){
    Factor *factor = NULL;

    // const_factor
    if(*lex == sym::PLUS
        || *lex == sym::MINUS
        || *lex == sym::UNSIGNED_INTEGER
        || *lex == sym::CHARACTER){
        const string ehd = "const_factor: ";
        SymSet idel = delimiter;
        idel.insert(sym::PLUS);
        idel.insert(sym::MINUS);
        idel.insert(sym::UNSIGNED_INTEGER);
        idel.insert(sym::CHARACTER);

        ConstFactor *const_factor = new ConstFactor();

        if(*lex == sym::CHARACTER){
            // char const
            const_factor->is_char = true;
            const_factor->char_value = lex.getCharValue();
            lex.nextSymbol();
        }
        else{
            // int const
            const_factor->is_char = false;
            const_factor->int_value = constructInteger(idel);
            if(const_factor->int_value == NULL){
                delete const_factor;
                const_factor = NULL;
            }
        }

        if(const_factor != NULL)
            factor = static_cast<Factor*>(const_factor);

        #if HW
        if(const_factor)
            log::hw << "const_factor";
        #endif// HW
    }
    // TODO
    return factor;
}

Item* GrammarAnalyzer::constructItem(const SymSet &delimiter){
    const string ehd = "item: ";
    SymSet idel = delimiter;
    // TODO insert Factor's head
    idel.insert(sym::MULTI);
    idel.insert(sym::DIV);

    Item *item = new Item();

    do{
        if(*lex == sym::MULTI || *lex == sym::DIV){
            item->op_list.push_back(*lex);
            lex.nextSymbol();
        }

        Factor *factor = constructFactor(idel);
        if(factor != NULL)
            item->factor_list.push_back(factor);
        // TODO consider to jump a op, if factor construct fail.
    }while(*lex == sym::MULTI || *lex == sym::DIV);

    if(item->factor_list.size() < 1){
        delete item;
        item = NULL;
    }

#if HW
    log::hw << "item";
#endif// HW

    return item;
}

Expression* GrammarAnalyzer::constructExpression(const SymSet &delimter){
    const string ehd = "expression: ";
    SymSet idel = delimter;
    idel.insert(sym::PLUS);
    idel.insert(sym::MINUS);
    // TODO insert item's head

    Expression *exp = new Expression();

    if(*lex == sym::PLUS || *lex == sym::MINUS){
        exp->minus = *lex == sym::MINUS;
        lex.nextSymbol();
    }

    do{
        if(*lex == sym::PLUS || *lex == sym::MINUS){
            exp->op_list.push_back(*lex);
            lex.nextSymbol();
        }

        Item *item = constructItem(idel);
        if(item != NULL)
            exp->item_list.push_back(item);
        // TODO consider jump a op if item construct fail
    }while(*lex == sym::PLUS || *lex == sym::MINUS);

    if(exp->item_list.size() < 1){
        delete exp;
        exp = NULL;
    }

#if HW
    log::hw << "expression";
#endif// HW

    return exp;
}
