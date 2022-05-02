#include "token.h"
#include "data.h"
#include "symbol.h"
#include <string.h>
#include <stdlib.h>

/**
 * This pointer is set when by parser() func and used by printParsedToken() func.
 *
 * You are not required to use it anywhere. The implemented part of the skeleton
 * handles the printing. Instead, You could use the supplied helper functions to
 * manipulate the output file.
 * */
FILE* _out;

/**
 * Token list iterator used by the parser. It will be set once entered to parser()
 * and reset before exiting parser().
 *
 * It is better to use the given helper functions to make use of token list iterator.
 * */
TokenListIterator _token_list_it;

/**
 * Current level.
 * */
unsigned int currentLevel;

/**
 * Symbol table.
 * */
SymbolTable symbolTable;

/**
 * Returns the current token using the token list iterator.
 * If it is the end of tokens, returns token with id nulsym.
 * */
Token getCurrentToken();

/**
 * Returns the type of the current token. Returns nulsym if it is the end of tokens.
 * */
int getCurrentTokenType();

/**
 * Prints the given token on _out by applying required formatting.
 * */
void printCurrentToken();

/**
 * Advances the position of TokenListIterator by incrementing the current token
 * index by one.
 * */
void nextToken();

/**
 * Given an entry from non-terminal enumaration, prints it.
 * */
void printNonTerminal(NonTerminal nonTerminal);

/**
 * Functions used for non-terminals of the grammar
 * */
int program();
int block();
int const_declaration();
int var_declaration();
int proc_declaration();
int statement();
int condition();
int relop();
int expression();
int term();
int factor();

Token getCurrentToken()
{
    return getCurrentTokenFromIterator(_token_list_it);
}

int getCurrentTokenType()
{
    return getCurrentToken().id;
}

void printCurrentToken()
{
    fprintf(_out, "%8s <%s, '%s'>\n", "TOKEN  :", tokenNames[getCurrentToken().id], getCurrentToken().lexeme);
}

void nextToken()
{
    _token_list_it.currentTokenInd++;
}

void printNonTerminal(NonTerminal nonTerminal)
{
    fprintf(_out, "%8s %s\n", "NONTERM:", nonTerminalNames[nonTerminal]);
}

/**
 * Given the parser error code, prints error message on file by applying
 * required formatting.
 * */
void printParserErr(int errCode, FILE* fp)
{
    if(!fp) return;

    if(!errCode)
        fprintf(fp, "\nPARSING WAS SUCCESSFUL.\n");
    else
        fprintf(fp, "\nPARSING ERROR[%d]: %s.\n", errCode, parserErrorMsg[errCode]);
}

/**
 * Advertised parser function. Given token list, which is possibly the output of
 * the lexer, parses the tokens. If encountered, return the error code.
 *
 * Returning 0 signals successful parsing.
 * Otherwise, returns a non-zero parser error code.
 * */
int parser(TokenList tokenList, FILE* out)
{
    // Set output file pointer
    _out = out;

    /**
     * Create a token list iterator, which helps to keep track of the current
     * token being parsed.
     * */
    _token_list_it = getTokenListIterator(&tokenList);

    // Initialize current level to 0, which is the global level
    currentLevel = 0;

    // Initialize symbol table
    initSymbolTable(&symbolTable);

    // Write parsing history header
    fprintf(_out, "Parsing History\n===============\n");

    // Start parsing by parsing program as the grammar suggests.
    int err = program();

    // Print symbol table - if no error occured
    if(!err)
    {
        fprintf(_out, "\n\n");
        printSymbolTable(&symbolTable, _out);
    }

    // Reset output file pointer
    _out = NULL;

    // Reset the global TokenListIterator
    _token_list_it.currentTokenInd = 0;
    _token_list_it.tokenList = NULL;

    // Delete symbol table
    deleteSymbolTable(&symbolTable);

    // Return err code - which is 0 if parsing was successful
    return err;
}

int program()
{
    printNonTerminal(PROGRAM);

    int err = block();
    if (err) return err;

    if(getCurrentTokenType() != periodsym){
        //No period, therefore return error
        return 6;
    }

    printCurrentToken();
    nextToken();

    return 0;
}

int block()
{
    printNonTerminal(BLOCK);

    int err = const_declaration();
    if (err) return err;

    err = var_declaration();
    if (err) return err;

    err = proc_declaration();
    if (err) return err;

    err = statement();
    if (err) return err;

    return 0;
}

int const_declaration()
{
    printNonTerminal(CONST_DECLARATION);
    Symbol currentSymbol;

    if(getCurrentTokenType() == constsym){
        currentSymbol.type = CONST;
        currentSymbol.level = currentLevel;

        do{
            printCurrentToken();
            nextToken();

            if(getCurrentTokenType() != identsym){
                return 3;
            }

            strcpy(currentSymbol.name, getCurrentToken().lexeme);

            printCurrentToken();
            nextToken();

            if(getCurrentTokenType() != eqsym){
                return 2;
            }

            printCurrentToken();
            nextToken();

            if(getCurrentTokenType() != numbersym){
                return 1;
            }

            currentSymbol.value = strtod(getCurrentToken().lexeme, NULL);
            addSymbol(&symbolTable, currentSymbol);

            printCurrentToken();
            nextToken();
        }

        while(getCurrentTokenType() == commasym);

        if(getCurrentTokenType() != semicolonsym)
            return 4;

        printCurrentToken();
        nextToken();
    }

    return 0;
}

int var_declaration()
{
    printNonTerminal(VAR_DECLARATION);
    Symbol currentSymbol;

    if(getCurrentTokenType() == varsym){
        currentSymbol.type = VAR;
        currentSymbol.level = currentLevel;

        do{
            printCurrentToken();
            nextToken();

            if(getCurrentTokenType() != identsym){
                return 3;
            }

            strcpy(currentSymbol.name, getCurrentToken().lexeme);
            addSymbol(&symbolTable, currentSymbol);

            printCurrentToken();
            nextToken();
        }while(getCurrentTokenType() == commasym);


        if (getCurrentTokenType() != semicolonsym){
            return 4;
        }

        printCurrentToken();
        nextToken();
    }

    return 0;
}

int proc_declaration()
{
    printNonTerminal(PROC_DECLARATION);
    Symbol currentSymbol;

    while(getCurrentTokenType() == procsym){
        printCurrentToken();
        nextToken();

        currentSymbol.type = PROC;
        currentSymbol.level = currentLevel;

        if(getCurrentTokenType() != identsym)
            return 3;

        strcpy(currentSymbol.name, getCurrentToken().lexeme);
        addSymbol(&symbolTable, currentSymbol);

        printCurrentToken();
        nextToken();

        if(getCurrentTokenType() != semicolonsym)
            return 5;

        printCurrentToken();
        nextToken();

        currentLevel++;

        int err = block();
        if (err) return err;

        currentLevel--;

        if(getCurrentTokenType() != semicolonsym)
            return 5;

        printCurrentToken();
        nextToken();
    }

    return 0;
}

int statement()
{
    printNonTerminal(STATEMENT);

    if (getCurrentTokenType() == identsym)
    {
        printCurrentToken();
        nextToken();

        if (getCurrentTokenType() != becomessym)
            return 7;

        printCurrentToken();
        nextToken();

        int err = expression();
        if (err) return err;
    }
    else if (getCurrentTokenType() == callsym)
    {
        printCurrentToken();
        nextToken();

        if (getCurrentTokenType() != identsym)
            return 8;

        printCurrentToken();
        nextToken();
    }
    else if (getCurrentTokenType() == beginsym)
    {
        printCurrentToken();
        nextToken();

        int err = statement();
        if (err) return err;

        while (getCurrentTokenType() == semicolonsym)
        {
            printCurrentToken();
            nextToken();

            int err = statement();
            if (err) return err;
        }

        if (getCurrentTokenType() != endsym)
            return 10;

        printCurrentToken();
        nextToken();
    }
    else if (getCurrentTokenType() == ifsym)
    {
        printCurrentToken();
        nextToken();

        int err = condition();
        if (err) return err;

        if (getCurrentTokenType() != thensym)
            return 9;

        printCurrentToken();
        nextToken();

        err = statement();
        if (err) return err;

        if (getCurrentTokenType() == elsesym)
        {
            printCurrentToken();
            nextToken();

            err = statement();
            if (err) return err;
        }
    }
    else if (getCurrentTokenType() == whilesym)
    {
        printCurrentToken();
        nextToken();

        int err = condition();
        if (err) return err;

        if (getCurrentTokenType() != dosym)
            return 11;


        printCurrentToken();
        nextToken();

        err = statement();
        if (err) return err;
    }
    else if (getCurrentTokenType() == writesym)
    {

        printCurrentToken();
        nextToken();


        if (getCurrentTokenType() != identsym)
            return 3;

        printCurrentToken();
        nextToken();
    }
    else if (getCurrentTokenType() == readsym)
    {
        printCurrentToken();
        nextToken();

        if (getCurrentTokenType() != identsym)
            return 3;

        printCurrentToken();
        nextToken();
    }

    return 0;
}

int condition()
{
    printNonTerminal(CONDITION);

    if(getCurrentTokenType() == oddsym){
        printCurrentToken();
        nextToken();

        int err = expression();
        if (err) return err;
    }
    else{
        int err = expression();
        if (err) return err;

        err = relop();
        if (err) return err;

        err = expression();
        if (err) return err;
    }
    return 0;
}

int relop()
{
    printNonTerminal(REL_OP);

    if(getCurrentTokenType() == eqsym || getCurrentTokenType() == neqsym || getCurrentTokenType() == lessym ||
       getCurrentTokenType() == leqsym || getCurrentTokenType() == gtrsym || getCurrentTokenType() == geqsym)
    {
        printCurrentToken();
        nextToken();

        return 0;
    }
    else
        return 12;

    return 0;
}

int expression()
{
    printNonTerminal(EXPRESSION);

    if(getCurrentTokenType() == plussym || getCurrentTokenType() == minussym){
        printCurrentToken();
        nextToken();
    }

    int err = term();
    if (err) return err;

    while(getCurrentTokenType() == plussym || getCurrentTokenType() == minussym){
        printCurrentToken();
        nextToken();

        err = term();
        if (err) return err;
    }
    return 0;
}

int term()
{
    printNonTerminal(TERM);

    int err = factor();
    if (err) return err;


    while(getCurrentTokenType() == multsym || getCurrentTokenType() == slashsym){
        printCurrentToken();
        nextToken();

        err = factor();
        if (err) return err;
    }

    return 0;
}

/**
 * The below function is left fully-implemented as a hint.
 * */
int factor()
{
    printNonTerminal(FACTOR);

    /**
     * There are three possibilities for factor:
     * 1) ident
     * 2) number
     * 3) '(' expression ')'
     * */

    // Is the current token a identsym?
    if(getCurrentTokenType() == identsym)
    {
        // Consume identsym
        printCurrentToken(); // Printing the token is essential!
        nextToken(); // Go to the next token..

        // Success
        return 0;
    }
    // Is that a numbersym?
    else if(getCurrentTokenType() == numbersym)
    {
        // Consume numbersym
        printCurrentToken(); // Printing the token is essential!
        nextToken(); // Go to the next token..

        // Success
        return 0;
    }
    // Is that a lparentsym?
    else if(getCurrentTokenType() == lparentsym)
    {
        // Consume lparentsym
        printCurrentToken(); // Printing the token is essential!
        nextToken(); // Go to the next token..

        // Continue by parsing expression.
        int err = expression();

        /**
         * If parsing of expression was not successful, immediately stop parsing
         * and propagate the same error code by returning it.
         * */

        if(err) return err;

        // After expression, right-parenthesis should come
        if(getCurrentTokenType() != rparentsym)
        {
            /**
             * Error code 13: Right parenthesis missing.
             * Stop parsing and return error code 13.
             * */
            return 13;
        }

        // It was a rparentsym. Consume rparentsym.
        printCurrentToken(); // Printing the token is essential!
        nextToken(); // Go to the next token..
    }
    else
    {
        /**
          * Error code 24: The preceding factor cannot begin with this symbol.
          * Stop parsing and return error code 24.
          * */
        return 14;
    }

    return 0;
}
