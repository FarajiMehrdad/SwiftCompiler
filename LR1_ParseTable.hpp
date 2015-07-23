#ifndef LR0_ParseTable_cpp
#define LR0_ParseTable_cpp

#include "TablePrinter.hpp"
#include "LR1_DFA.hpp"
#include <iostream>
#include <map>

typedef list<const Element*> ElementConstPtrList;

enum ParseTableTermialAction {
	Error=0,
	Accept,
	Shift,
	Reduce
};

typedef class LR1_ParseTableRow LR1_ParseTableRow;

struct LR1_ParseTableTerminalCell {
	ParseTableTermialAction action=Error;
	LR1_DFAState* movingRow=nullptr;
	Rule* reducingRule=nullptr;
	
	bool operator==(const LR1_ParseTableTerminalCell& cell) {
		return (this->action == cell.action);
	}
};

struct LR1_ParseTableVariableCell {
	LR1_DFAState* movingRow=nullptr;
	
	bool operator==(const LR1_ParseTableVariableCell& cell) {
		return (this->movingRow == cell.movingRow);
	}
};

class LR1_ParseTableRow {
public:
	LR1_ParseTableRow();
	LR1_ParseTableRow(LR1_DFAState* statePointer);
	
	LR1_ParseTableRow& operator=(const LR1_ParseTableRow& row);
	
	const LR1_DFAState* statePointer;
	list<LR1_ParseTableTerminalCell>* terminalCells;
	list<LR1_ParseTableVariableCell>* variableCells;
	
	void display(TablePrinter& tableOut);
};

class LR1_ParseTable {
public:
	LR1_ParseTable(const LR1_DFA& dfa);
	
	const LR1_DFA* dfa;
	ElementConstPtrList grammarTerminals;
	ElementConstPtrList grammarVariables;
	map<LR1_DFAState*, LR1_ParseTableRow> table;
	
	void display();
};

#endif
