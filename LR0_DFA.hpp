#ifndef LR0_DFA_cpp
#define LR0_DFA_cpp

#include "Grammar.hpp"
#include <iostream>

using namespace std;

typedef class LR0_DFAState LR0_DFAState;

class LR0_DFAState_Row {
public:
	LR0_DFAState_Row(Element* variable,
					 Rule* rule,
					 Element* lookAhead,
					 bool isCore);
	
	Element* variable;
	Rule* rule;
	Element* lookAhead;
	bool isCore;
	LR0_DFAState** nextState;
	
	LR0_DFAState_Row& operator=(const LR0_DFAState_Row& row);
	bool operator==(const LR0_DFAState_Row& row);
	bool operator!=(const LR0_DFAState_Row& row);
	
	friend ostream& operator<<(ostream& output, const LR0_DFAState_Row& row);
};


class LR0_DFAState {
public:
	LR0_DFAState();
	LR0_DFAState(Grammar* grammar);
	~LR0_DFAState();
	
	static unsigned int stateCount;
	static unsigned int stateUniqueIDs;
	static Grammar* grammar;
	unsigned int stateID;
	list<LR0_DFAState_Row> coreRows;
	list<LR0_DFAState_Row> minorRows;
	map<Element*, LR0_DFAState*> moves;
	
	bool rowExists(const LR0_DFAState_Row& rowToFind);
	void addCoreRow(const LR0_DFAState_Row& rowToAdd);
	void expandRow(const LR0_DFAState_Row& rowToExpand);
	
	friend ostream& operator<<(ostream& output, const LR0_DFAState& state);
};

class LR0_DFA {
public:
	LR0_DFA();
	LR0_DFA(Grammar* grammar);
	
	list<LR0_DFAState*> dfaStates;
	
	void insertState(LR0_DFAState* stateToInsert);
	bool listsAreMatch(list<LR0_DFAState_Row> first, list<LR0_DFAState_Row> second);
	LR0_DFAState* stateExists(LR0_DFAState* stateToFind);
	void addNewState(LR0_DFAState* stateToMove);
	Element* findNextLookAhead(const LR0_DFAState_Row* row);
	
	friend ostream& operator<< (ostream& output, LR0_DFA dfa);
};

#endif
