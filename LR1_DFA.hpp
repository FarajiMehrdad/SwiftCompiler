#ifndef LR1_DFA_cpp
#define LR1_DFA_cpp

#include "TablePrinter.hpp"
#include "Grammar.hpp"
#include <iostream>

using namespace std;

typedef class LR1_DFAState LR1_DFAState;

class LR1_DFAState_Row {
public:
	LR1_DFAState_Row(Element* variable,
					 Rule* rule,
					 ElementPtrList::iterator lookAhead,
					 bool isCore,
					 ElementPtrList followList);
	
	Element* variable;
	Rule* rule;
	ElementPtrList::iterator lookAhead;
	bool isCore;
	LR1_DFAState** nextState;
	ElementPtrList followList;
	
	LR1_DFAState_Row& operator=(const LR1_DFAState_Row& row);
	bool operator==(const LR1_DFAState_Row& row);
	bool operator!=(const LR1_DFAState_Row& row);
	
	void display(TablePrinter& tableOut);
};

class LR1_DFAState {
public:
	LR1_DFAState();
	LR1_DFAState(Grammar* grammar);
	~LR1_DFAState();
	
	static unsigned int stateCount;
	static unsigned int stateUniqueIDs;
	static Grammar* grammar;
	unsigned int stateID;
	list<LR1_DFAState_Row> coreRows;
	list<LR1_DFAState_Row> minorRows;
	map<ElementPtrList::iterator, LR1_DFAState*> moves;
	
	bool rowExists(const LR1_DFAState_Row& rowToFind);
	void addCoreRow(const LR1_DFAState_Row& rowToAdd);
	void expandRow(const LR1_DFAState_Row& rowToExpand);
	
	void display();
};

class LR1_DFA {
public:
	LR1_DFA();
	LR1_DFA(Grammar* grammar);
	
	list<LR1_DFAState*> dfaStates;
	
	void insertState(LR1_DFAState* stateToInsert);
	LR1_DFAState* stateExists(LR1_DFAState* stateToFind);
	void addNewState(LR1_DFAState* stateToMove);
	
	void display();
};

#endif
