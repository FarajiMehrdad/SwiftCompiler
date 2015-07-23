#ifndef LR1_Parser_cpp
#define LR1_Parser_cpp

#include "Grammar.hpp"
#include "LR1_DFA.hpp"
#include "LR1_ParseTable.hpp"
#include <iostream>
#include <fstream>
#include <stack>
#include <list>

using namespace std;

class TokenStream {
public:
	TokenStream(const string& name = "token.txt");
	
	list<string> tokenStream;
};

class LR1_Parser {
public:
	LR1_Parser();
	LR1_Parser(Grammar* grammar);
	
	LR1_DFA* dfa;
	LR1_ParseTable* parseTable;
	stack<LR1_DFAState*> stateStack;
	stack<string> tokenStack;
	
	void parse(TokenStream* tokenStream);
};

#endif
