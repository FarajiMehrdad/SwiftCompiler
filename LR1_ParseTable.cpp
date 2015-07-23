#define COL_WIDTH 8
#include "TablePrinter.hpp"
#include "LR1_ParseTable.hpp"

ElementConstPtrList globalGrammarTerminals;
ElementConstPtrList globalGrammarVariables;

bool operator<(const LR1_DFAState& rhs, const LR1_DFAState& lhs) {
	return rhs.stateID < lhs.stateID;
}

LR1_ParseTableRow::LR1_ParseTableRow() {
	
}

LR1_ParseTableRow::LR1_ParseTableRow(LR1_DFAState* statePointer) {
	this->statePointer = statePointer;
	
	terminalCells = new list<LR1_ParseTableTerminalCell>[globalGrammarTerminals.size()];
	variableCells = new list<LR1_ParseTableVariableCell>[globalGrammarVariables.size()];
	auto elementIterator = globalGrammarTerminals.begin();
	int elementCounter = 0;
	while (elementCounter < globalGrammarTerminals.size() &&
		   elementIterator != globalGrammarTerminals.end()) {
		list<LR1_ParseTableTerminalCell>* currentCell = terminalCells + elementCounter;
		for (auto rowsIterator = statePointer->coreRows.begin();
			 rowsIterator != statePointer->coreRows.end();
			 rowsIterator++) {
			if (rowsIterator->lookAhead == rowsIterator->rule->elements.end()) {
				if (rowsIterator->variable->isStartVariable) {
					LR1_ParseTableTerminalCell acceptCell;
					acceptCell.action = Accept;
					currentCell->push_back(acceptCell);
					return;
				} else {
					for (auto followIterator = rowsIterator->followList.begin();
						 followIterator != rowsIterator->followList.end();
						 followIterator++) {
						if (*elementIterator == *followIterator) {
							LR1_ParseTableTerminalCell newTerminalCell;
							newTerminalCell.action = Reduce;
							newTerminalCell.reducingRule = rowsIterator->rule;
							currentCell->push_back(newTerminalCell);
						}
					}
				}
			}
		}
		if (currentCell->size() > 1) {
			cout << "Warning! This Language Doesn't Support LR1-Parser." << endl;
		}
		elementIterator++;
		elementCounter++;
	}
	
	elementIterator = globalGrammarTerminals.begin();
	elementCounter = 0;
	while (elementCounter < globalGrammarTerminals.size() &&
		   elementIterator != globalGrammarTerminals.end()) {
		list<LR1_ParseTableTerminalCell>* currentCell = terminalCells + elementCounter;
		for (auto movesIterator = statePointer->moves.begin();
			 movesIterator != statePointer->moves.end();
			 movesIterator++) {
			if ((*movesIterator->first)->isTerminal) {
				if (*elementIterator == *movesIterator->first) {
					LR1_ParseTableTerminalCell newTermianlCell;
					newTermianlCell.action = Shift;
					newTermianlCell.movingRow = movesIterator->second;
					currentCell->push_back(newTermianlCell);
				}
			}
		}
		if (currentCell->size() > 1) {
			cout << "Warning! This Language Doesn't Support LR1-Parser." << endl;
		}
		elementIterator++;
		elementCounter++;
	}
	
	elementIterator = globalGrammarVariables.begin();
	elementCounter = 0;
	while (elementCounter < globalGrammarVariables.size() &&
		   elementIterator != globalGrammarVariables.end()) {
		list<LR1_ParseTableVariableCell>* currentCell = variableCells + elementCounter;
		for (auto movesIterator = statePointer->moves.begin();
			 movesIterator != statePointer->moves.end();
			 movesIterator++) {
			if (!(*movesIterator->first)->isTerminal) {
				if (*elementIterator == *movesIterator->first) {
					LR1_ParseTableVariableCell newVariableCell;
					newVariableCell.movingRow = movesIterator->second;
					currentCell->push_back(newVariableCell);
				}
			}
		}
		if (currentCell->size() > 1) {
			cout << "Warning! This Language Doesn't Support LR1-Parser." << endl;
		}
		elementIterator++;
		elementCounter++;
	}
}

LR1_ParseTableRow& LR1_ParseTableRow::operator=(const LR1_ParseTableRow& row) {
	this->statePointer = row.statePointer;
	this->terminalCells = row.terminalCells;
	this->variableCells = row.variableCells;
	return *this;
}

LR1_ParseTable::LR1_ParseTable(const LR1_DFA& dfa) {
	this->dfa = &dfa;
	
	auto stateIterator = dfa.dfaStates.begin();
	for (auto grammarIterator = (*stateIterator)->grammar->grammar.begin();
		 grammarIterator != (*stateIterator)->grammar->grammar.end();
		 grammarIterator++) {
		if (grammarIterator->first.isTerminal && !grammarIterator->first.isEpsilon) {
			globalGrammarTerminals.push_back(&grammarIterator->first);
		} else if (!grammarIterator->first.isTerminal) {
			globalGrammarVariables.push_back(&grammarIterator->first);
		}
	}
	for (auto terminalIterator = globalGrammarTerminals.begin();
		 terminalIterator != globalGrammarTerminals.end();
		 terminalIterator++) {
		this->grammarTerminals.push_back(*terminalIterator);
	}
	for (auto variableIterator = globalGrammarVariables.begin();
		 variableIterator != globalGrammarVariables.end();
		 variableIterator++) {
		this->grammarVariables.push_back(*variableIterator);
	}
	
	for (stateIterator = dfa.dfaStates.begin();
		 stateIterator != dfa.dfaStates.end();
		 stateIterator++) {
		LR1_ParseTableRow newRow(*stateIterator);
		table[*stateIterator] = newRow;
	}
}

void LR1_ParseTableRow::display(TablePrinter& tableOut) {
	tableOut.Print(this->statePointer->stateID);
	
	for (int terminalCounter = 0;
		 terminalCounter < globalGrammarTerminals.size();
		 terminalCounter++) {
		list<LR1_ParseTableTerminalCell>* currentCell = terminalCells + terminalCounter;
		string cellString="";
		for (auto cellIterator = currentCell->begin();
			 cellIterator != currentCell->end();
			 cellIterator++) {
			if (cellIterator == ++currentCell->begin()) {
				cellString += " ";
			}
			switch (cellIterator->action) {
				case Shift:
					cellString += "S," + to_string(cellIterator->movingRow->stateID);
					break;
				case Reduce:
					cellString += "r," + to_string(cellIterator->reducingRule->ruleID);
					break;
				case Accept:
					cellString += "Acc";
					break;
				default:
					cellString += "Err";
					break;
			}
		}
		tableOut.Print(cellString);
	}
	tableOut.Print("~~~");
	for (int variableCounter = 0;
		 variableCounter < globalGrammarVariables.size();
		 variableCounter++) {
		list<LR1_ParseTableVariableCell>* currentCell = variableCells + variableCounter;
		if (currentCell->begin()->movingRow != nullptr) {
			tableOut.Print(currentCell->begin()->movingRow->stateID);
		} else {
			tableOut.Print("");
		}
	}
}

void LR1_ParseTable::display() {
	int columnSize = 1 + int(globalGrammarTerminals.size()) + 1 + int(globalGrammarVariables.size());
	int columnCounter=1;
	TablePrinter tableOut("ParseTable", int(this->dfa->dfaStates.size()), columnSize, "table_out.txt");
	tableOut.SetColHeader(0, "StateID");
	for (auto terminalIterator = globalGrammarTerminals.begin();
		 terminalIterator != globalGrammarTerminals.end() && columnCounter <= columnSize;
		 terminalIterator++) {
		tableOut.SetColHeader(columnCounter++, (*terminalIterator)->name);
	}
	tableOut.SetColHeader(columnCounter++, "~~~");
	for (auto variableIterator = globalGrammarVariables.begin();
		 variableIterator != globalGrammarVariables.end() && columnCounter <= columnSize;
		 variableIterator++) {
		tableOut.SetColHeader(columnCounter++, (*variableIterator)->name);
	}
	tableOut.SetColWidth(COL_WIDTH);
	tableOut.SetHaveColDiv(true);
	tableOut.SetHaveLineDiv(true);
	tableOut.SetHaveLineNumber(false);
	for (auto rowIterator = this->table.begin();
		 rowIterator != this->table.end();
		 rowIterator++) {
		rowIterator->second.display(tableOut);
	}
}

