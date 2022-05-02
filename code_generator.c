#include "token.h"
#include "data.h"
#include "symbol.h"
#include <string.h>
#include <stdlib.h>

/**
 * This pointer is set when by codeGenerator() func and used by printEmittedCode() func.
 *
 * You are not required to use it anywhere. The implemented part of the skeleton
 * handles the printing. Instead, you are required to fill the vmCode properly by making
 * use of emit() func.
 * */
FILE* _out;

/**
 * Token list iterator used by the code generator. It will be set once entered to
 * codeGenerator() and reset before exiting codeGenerator().
 *
 * It is better to use the given helper functions to make use of token list iterator.
 * */
TokenListIterator _token_list_it;

/**
 * Current level. Use this to keep track of the current level for the symbol table entries.
 * */
unsigned int currentLevel;

/**
 * Current scope. Use this to keep track of the current scope for the symbol table entries.
 * NULL means global scope.
 * */
Symbol* currentScope;

/**
 * Symbol table.
 * */
SymbolTable symbolTable;

/**
 * The array of instructions that the generated(emitted) code will be held.
 * */
Instruction vmCode[MAX_CODE_LENGTH];

/**
 * The next index in the array of instructions (vmCode) to be filled.
 * */
int nextCodeIndex;

/**
 * The id of the register currently being used.
 * */
int currentReg;

/**
 * Emits the instruction whose fields are given as parameters.
 * Internally, writes the instruction to vmCode[nextCodeIndex] and returns the
 * nextCodeIndex by post-incrementing it.
 * If MAX_CODE_LENGTH is reached, prints an error message on stderr and exits.
 * */
int emit(int OP, int R, int L, int M);

/**
 * Prints the emitted code array (vmCode) to output file.
 *
 * This func is called in the given codeGenerator() function. You are not required
 * to have another call to this function in your code.
 * */
void printEmittedCodes();

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
 * Advances the position of TokenListIterator by incrementing the current token
 * index by one.
 * */
void nextToken();

/**
 * Functions used for non-terminals of the grammar
 *
 * rel-op func is removed on purpose. For code generation, it is easier to parse
 * rel-op as a part of condition.
 * */
int program();
int block();
int const_declaration();
int var_declaration();
int proc_declaration();
int statement();
int condition();
int expression();
int term();
int factor();

/******************************************************************************/
/* Definitions of helper functions starts *************************************/
/******************************************************************************/

Token getCurrentToken()
{
    return getCurrentTokenFromIterator(_token_list_it);
}

int getCurrentTokenType()
{
    return getCurrentToken().id;
}

void nextToken()
{
    _token_list_it.currentTokenInd++;
}

/**
 * Given the code generator error code, prints error message on file by applying
 * required formatting.
 * */
void printCGErr(int errCode, FILE* fp)
{
    if(!fp || !errCode) return;

    fprintf(fp, "CODE GENERATOR ERROR[%d]: %s.\n", errCode, codeGeneratorErrMsg[errCode]);
}

int emit(int OP, int R, int L, int M)
{
    if(nextCodeIndex == MAX_CODE_LENGTH)
    {
        fprintf(stderr, "MAX_CODE_LENGTH(%d) reached. Emit is unsuccessful: terminating code generator..\n", MAX_CODE_LENGTH);
        exit(0);
    }

    vmCode[nextCodeIndex] = (Instruction){ .op = OP, .r = R, .l = L, .m = M};

    return nextCodeIndex++;
}

void printEmittedCodes()
{
    for(int i = 0; i < nextCodeIndex; i++)
    {
        Instruction c = vmCode[i];
        fprintf(_out, "%d %d %d %d\n", c.op, c.r, c.l, c.m);
    }
}

/******************************************************************************/
/* Definitions of helper functions ends ***************************************/
/******************************************************************************/

/**
 * Advertised codeGenerator function. Given token list, which is possibly the
 * output of the lexer, parses a program out of tokens and generates code.
 * If encountered, returns the error code.
 *
 * Returning 0 signals successful code generation.
 * Otherwise, returns a non-zero code generator error code.
 * */
int codeGenerator(TokenList tokenList, FILE* out)
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

    // Initialize current scope to NULL, which is the global scope
    currentScope = NULL;

    // The index on the vmCode array that the next emitted code will be written
    nextCodeIndex = 0;

    // The id of the register currently being used
    currentReg = 0;

    // Initialize symbol table
    initSymbolTable(&symbolTable);

    // Start parsing by parsing program as the grammar suggests.
    int err = program();

    // Print symbol table - if no error occured
    if(!err)
    {
        // Print the emitted codes to the file
        printEmittedCodes();
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

// Already implemented.
int program()
{
	// Generate code for block
    int err = block();
    if(err) return err;

    // After parsing block, periodsym should show up
    if( getCurrentTokenType() == periodsym )
    {
        // Consume token
        nextToken();

        // End of program, emit halt code
        emit(SIO_HALT, 0, 0, 3);

        return 0;
    }
    else
    {
        // Periodsym was expected. Return error code 6.
        return 6;
    }
}

int block()
{
    int err;

    Symbol tempScope;
    tempScope.scope = currentScope;

    if(currentScope != NULL)
        emit(INC,0,0,4);

    err = const_declaration();
    if(err)
        return err;
    err = var_declaration();
    if(err)
        return err;
    err = proc_declaration();
    if(err)
        return err;
    currentScope = &tempScope;
    err = statement();
    if(err)
        return err;
    currentScope = tempScope.scope;
    return 0;
}

int const_declaration()
{

    Symbol symbol;
    Token token;
    symbol.type = CONST;
    symbol.level = currentLevel;

    if(getCurrentTokenType() != constsym)
    {
        return 0;
    }

    nextToken();
    if(getCurrentTokenType() != identsym)
    {
        return 3;
    }

    token = getCurrentToken();
    strcpy(symbol.name, token.lexeme);
    nextToken();

    if(getCurrentTokenType() != eqsym)
    {
        return 2;
    }

    nextToken();
    if(getCurrentTokenType() != numbersym)
    {
        return 1;
    }

    token = getCurrentToken();
    symbol.value = atoi(token.lexeme);

    symbol.scope = currentScope; //add current scope to symbol table
    nextToken();
    addSymbol(&symbolTable,symbol);

    while(getCurrentTokenType() == commasym)
    {

        nextToken();
        if(getCurrentTokenType() != identsym)
            return 3;


        token = getCurrentToken();
        strcpy(symbol.name, token.lexeme);
        nextToken();

        if(getCurrentTokenType() != eqsym)
        {
            return 2;
        }

        nextToken();
        if(getCurrentTokenType() != numbersym)
        {
            return 1;
        }

        token = getCurrentToken();
        symbol.value = atoi(token.lexeme);

        symbol.scope = currentScope;// add current scope to symbol table
        nextToken();
        addSymbol(&symbolTable,symbol);
    }
    if(getCurrentTokenType() != semicolonsym)
        return 4;

    nextToken();

    return 0;
}

int var_declaration()
{
    if(getCurrentTokenType() != varsym)
        return 0;

    Symbol symbol;
    symbol.type = VAR;
    symbol.level = currentLevel;
    symbol.scope = currentScope;
    Token token;
    int varNum = 0;
    emit(INC,0,0,2);

    while(1){
        if(getCurrentTokenType() == varsym || getCurrentTokenType() == commasym){
            varNum++;
            nextToken();
            if(getCurrentTokenType() == identsym){

                token = getCurrentToken();
                strcpy(symbol.name, token.lexeme);
                nextToken();
                if(getCurrentTokenType() == eqsym){

                    nextToken();
                    if(getCurrentTokenType() == numbersym){

                        token = getCurrentToken();
                        symbol.value = atoi(token.lexeme);
                        symbol.address = 4 * currentLevel + varNum;
                        nextToken();
                        if(getCurrentTokenType() == semicolonsym){

                            nextToken();
                            break;
                        }
                        else if(getCurrentTokenType() != commasym && getCurrentTokenType() != semicolonsym){
                            return 4;
                        }
                        addSymbol(&symbolTable,symbol);
                    }
                    else{
                        return 1;
                    }
                }
                else if(getCurrentTokenType() == semicolonsym){

                    nextToken();
                    symbol.address = 4 * currentLevel + varNum;
                    addSymbol(&symbolTable,symbol);
                    break;
                }
                else if(getCurrentTokenType() == commasym){
                    symbol.address = 4 * currentLevel + varNum;
                    addSymbol(&symbolTable,symbol);
                }
                else{
                    return 4;
                }
            }
            else{
                return 3;
            }
        }
        else{
            break;
        }
    }
    return 0;
}

int proc_declaration()
{
    int err;
    Token token;
    Symbol symbol;
    int tmpscope = 0;

    int varNum = 0, procTrue = 0;
    symbol.type = PROC;
    int codeIndex = nextCodeIndex;
    if(getCurrentTokenType() == procsym){
        emit(JMP,0,0,0);
        procTrue = 1;
    }

    while(getCurrentTokenType() == procsym)
    {
        varNum++;
        nextToken();
        if(getCurrentTokenType() != identsym){
            return 3;
        }
        token = getCurrentToken();
        strcpy(symbol.name, token.lexeme);
        nextToken();
        if(getCurrentTokenType() != semicolonsym){
                return 5;
        }
        nextToken();
        symbol.scope = currentScope;
        symbol.level = currentLevel;
        symbol.address = nextCodeIndex;
        addSymbol(&symbolTable,symbol);

        currentScope++;
        currentLevel++;
        err = block();
        currentLevel--;
        currentScope--;

        if(err)
            return err;
        if(getCurrentTokenType() != semicolonsym)
        {
            return 5;
        }
        nextToken();
    }
    if(procTrue == 1)
        emit(RTN,0,0,0);
    vmCode[codeIndex].m = nextCodeIndex;

    return 0;
}

int statement()
{
	int err = 0, jmp, jmp2;
	Symbol* currSym;

    if(getCurrentTokenType() == identsym)
	{
		currSym = findSymbol(&symbolTable, currentScope, getCurrentToken().lexeme);

		if(currSym->scope != currentScope)
			return 15;
		if(currSym->type != VAR)
			return 16;

		nextToken();
		if(getCurrentTokenType() != becomessym)
			return 7;

		// Get next token and pass to expression.
		nextToken();
		err = expression();
		if(err != 0)
			return err;
	}
	// Statement that begins w call symbol.
	else if(getCurrentTokenType() == callsym)
	{
		nextToken();
		if(getCurrentTokenType() != identsym)
			return 8;

		currSym = findSymbol(&symbolTable, currentScope, getCurrentToken().lexeme);

		// Check scope/type of symbol.
		if(currSym->scope != currentScope)
			return 15;
		if(currSym->type == PROC)
			emit(CAL, 0, currentLevel - currSym->level, currSym->address);
		else
			return 17;


		nextToken();
	}

	else if(getCurrentTokenType() == beginsym)
	{
		nextToken();
		err = statement();
		if(err != 0)
			return err;

		while (getCurrentTokenType() == semicolonsym)
		{
			// Get next token and pass to statement.
			nextToken();
			err = statement();
			if(err != 0)
				return err;
		}

		if(getCurrentTokenType() != endsym)
			return 10;
		nextToken();
	}

	else if(getCurrentTokenType() == ifsym)
	{
		nextToken();
		err = condition();
		if(err != 0)
			return err;

		if(getCurrentTokenType() != thensym)
			return 9;

		nextToken();

		jmp = nextCodeIndex;
		emit(JPC, 0, 0, 0);

		err = statement();
		if(err != 0)
			return err;

		vmCode[jmp].m = nextCodeIndex;

		if(getCurrentTokenType() == elsesym)
		{
			jmp2 = nextCodeIndex;
			emit(JMP, 0, 0, 0);

			// Get next token & update  jump address
			nextToken();
			vmCode[jmp2].m = nextCodeIndex;

			err = statement();
			if(err != 0)
				return err;

			vmCode[jmp].m = nextCodeIndex;
		}
	}
	else if(getCurrentTokenType() == whilesym)
	{
		jmp = nextCodeIndex;

		nextToken();
		err = condition();
		if(err != 0)
			return err;

		jmp2 = nextCodeIndex;
		emit(JPC, 0, 0, 0);

		if(getCurrentTokenType() != dosym)
			return 11;

		nextToken();
		err = statement();
		if(err != 0)
			return err;

		emit(JMP, 0, 0, jmp);
		vmCode[jmp2].m = nextCodeIndex;
	}
	else if(getCurrentTokenType() == writesym)
	{

		nextToken();
		if(getCurrentTokenType() != identsym)
			return 3;

		// Get symbol and check scope/type.
		currSym = findSymbol(&symbolTable, currentScope, getCurrentToken().lexeme);
		if(currSym->scope != currentScope)
			return 15;
		if(currSym->type == PROC)
			return 18;

		emit(LOD, 0, currentLevel - currSym->level, currSym->address);
		emit(SIO_WRITE, 0, 0, 0);


		nextToken();
	}

	else if(getCurrentTokenType() == readsym)
	{
		emit(SIO_READ, 0, 0, 0);

		nextToken();
		if(getCurrentTokenType() != identsym)
			return 3;

		// Get symbol and check scope/type.
		currSym = findSymbol(&symbolTable, currentScope, getCurrentToken().lexeme);
		if(currSym->scope != currentScope)
			return 15;
		if(currSym->type != VAR)
			return 19;

		nextToken();
		emit(STO, 0, currentLevel - currSym->level, currSym->address);
	}

    return 0;
}

int condition()
{
    int err;

    if(getCurrentTokenType() == oddsym){

        nextToken();
        err = expression();
        if(err)
            return err;
        emit(ODD,currentReg,0,0);
    }
    else{
        err = expression();
        if(err)
            return err;

        if(getCurrentTokenType() == eqsym)
        {
            emit(EQL,currentReg,0,0);
            nextToken();
            return 0;
        }
        else if(getCurrentTokenType() == neqsym)
        {
            emit(NEQ,currentReg,0,0);
            nextToken();
            return 0;
        }
        else if(getCurrentTokenType() == lessym)
        {
            emit(LSS,currentReg,0,0);
            nextToken();
            return 0;
        }
        else if(getCurrentTokenType() == leqsym)
        {
            emit(LEQ,currentReg,0,0);
            nextToken();
            return 0;
        }
        else if(getCurrentTokenType() == gtrsym)
        {
            emit(GTR,currentReg,0,0);
            nextToken();
            return 0;
        }
        else if(getCurrentTokenType() == geqsym)
        {
            emit(GEQ,currentReg,0,0);
            nextToken();
            return 0;
        }
        else
        {
            return 12;
        }

        err = expression();
        if(err)
            return err;
    }

    return 0;
}

int expression()
{
	int err = 0;
	int op = getCurrentTokenType();


    if(op == plussym || op == minussym)
	{
		nextToken();

		err = term();
		if(err != 0)
			return err;

		if(op == minussym)
			emit(NEG, 0, 0, 0);
	}

	err = term();
	if(err != 0)
		return err;


	// Continue parsing
	while(op == plussym || op == minussym)
	{
		nextToken();

		err = term();
		if(err != 0)
			return err;

		if(op == plussym)
			emit(ADD, 0, 0, 0);
		else
			emit(SUB, 0, 0, 0);
	}

    return 0;
}

int term()
{
	int err = 0;
	int op = getCurrentTokenType();

    err = factor();
	if(err != 0)
		return err;

	// Continue parsing
	while(op == multsym || op == slashsym)
	{
		nextToken();

		err = factor();
		if(err != 0)
			return err;

		if(op == multsym)
			emit(MUL, 0, 0, 0);
		else
			emit(DIV, 0, 0, 0);
	}

    return 0;
}

int factor()
{

	Symbol* currSym = findSymbol(&symbolTable, currentScope, getCurrentToken().lexeme);
	if(currSym == NULL)
		return 15;

    if(getCurrentTokenType() == identsym)
    {
		if(currSym->type == PROC)
			return 14;
		else if(currSym->type == CONST)
			emit(LIT, 0, 0, currSym->value);
		else
			emit(LOD, 0, currentLevel - currSym->level, currSym->address);

        nextToken();

        return 0;
    }
    else if(getCurrentTokenType() == numbersym)
    {
		int value = atoi(getCurrentToken().lexeme);
		emit(LIT, 0, 0, value);

        nextToken();

        return 0;
    }

    else if(getCurrentTokenType() == lparentsym)
    {
        nextToken();

        // Continue parsing expression
        int err = expression();

        if(err) return err;


        if(getCurrentTokenType() != rparentsym)
        {
            return 13;
        }


        nextToken();
    }
    else
    {
        return 24;
    }

    return 0;
}
