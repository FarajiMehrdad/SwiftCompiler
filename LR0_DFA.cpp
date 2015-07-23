#include "LR0_DFA.hpp"
#include "Grammar.hpp"


LR0_DFAState_Row::LR0_DFAState_Row(Element* variable,
								   Rule* rule,
								   Element* lookAhead,
								   bool isCore) {
	this->variable = variable;
	this->rule = rule;
	this->lookAhead = lookAhead;
	this->isCore = isCore;
	this->nextState = nullptr;
}

LR0_DFAState_Row& LR0_DFAState_Row::operator=(const LR0_DFAState_Row& row) {
	this->variable = row.variable;
	this->rule = row.rule;
	this->lookAhead = row.lookAhead;
	this->isCore = row.isCore;
	this->nextState = row.nextState;
	
	return *this;
}

bool LR0_DFAState_Row::operator==(const LR0_DFAState_Row& row) {
	if (this->isCore == row.isCore &&
		this->lookAhead == row.lookAhead &&
		this->rule->ruleID == row.rule->ruleID) {
		return true;
	}
	return false;
}

bool LR0_DFAState_Row::operator!=(const LR0_DFAState_Row& row) {
	return !(*this == row);
}










unsigned int LR0_DFAState::stateCount = 0;
unsigned int LR0_DFAState::stateUniqueIDs = 0;
Grammar* LR0_DFAState::grammar = nullptr;



LR0_DFAState::LR0_DFAState() {
	this->stateID = ++stateUniqueIDs;
	++stateCount;
}

LR0_DFAState::LR0_DFAState(Grammar* grammar) {
	LR0_DFAState::grammar = grammar;
	this->stateID = ++stateUniqueIDs;
	++stateCount;
}

LR0_DFAState::~LR0_DFAState() {
	--stateCount;
}

bool LR0_DFAState::rowExists(const LR0_DFAState_Row& rowToFind) {
	list<LR0_DFAState_Row> rows = coreRows;
	rows.insert(rows.end(), minorRows.begin(), minorRows.end());
	
	for (auto listIterator = rows.begin();
		 listIterator != rows.end();
		 listIterator++) {
		if (*listIterator == rowToFind) {
			return true;
		}
	}
	return false;
}

void LR0_DFAState::addCoreRow(const LR0_DFAState_Row& rowToAdd) {
	if (!rowExists(rowToAdd)) {
		coreRows.push_back(rowToAdd);
		if (rowToAdd.lookAhead == nullptr) {
			return;
		}
		moves[rowToAdd.lookAhead] = nullptr;
		expandRow(rowToAdd);
	}
}

void LR0_DFAState::expandRow(const LR0_DFAState_Row& rowToExpand) {
	if (rowToExpand.lookAhead == nullptr) {
		return;
	}
	
	moves[rowToExpand.lookAhead] = nullptr;
	
	if (rowToExpand.lookAhead->isTerminal) {
		return;
	}
	
	for (auto grammarIterator = grammar->grammar.begin();
		 grammarIterator != grammar->grammar.end();
		 grammarIterator++) {
		if (rowToExpand.lookAhead->name == grammarIterator->first.name) {
			for (auto ruleIterator = grammarIterator->second.rules.begin();
				 ruleIterator != grammarIterator->second.rules.end();
				 ruleIterator++) {
				Element* newLookAhead = *ruleIterator->elements.begin();
				if (ruleIterator->elements.size() == 1 && (*ruleIterator->elements.begin())->isEpsilon) {
					newLookAhead = nullptr;
				}
				LR0_DFAState_Row newRow(rowToExpand.lookAhead,
										&(*ruleIterator),
										newLookAhead,
										false);
				if (!rowExists(newRow)) {
					minorRows.push_back(newRow);
					expandRow(newRow);
				}
			}
			break;
		}
	}
}











LR0_DFA::LR0_DFA(Grammar* grammar) {
	LR0_DFAState* startState = new LR0_DFAState(grammar);
	for (auto grammarIterator = grammar->startVariableIterator->second.rules.begin();
		 grammarIterator != grammar->startVariableIterator->second.rules.end();
		 grammarIterator++) {
		Element* startVariable = const_cast<Element*>(&grammar->startVariableIterator->first);
		Element* lookAhead = *grammarIterator->elements.begin();
		LR0_DFAState_Row newCoreRow(startVariable,
									&(*grammarIterator),
									lookAhead,
									true);
		startState->addCoreRow(newCoreRow);
	}
	dfaStates.push_back(startState);
	addNewState(startState);
	
	for (auto statesIterator = dfaStates.begin();
		 statesIterator != dfaStates.end();
		 statesIterator++) {
		addNewState(*statesIterator);
	}
}

void LR0_DFA::insertState(LR0_DFAState* stateToInsert) {
	for (auto stateIterator = dfaStates.begin();
		 stateIterator != dfaStates.end();
		 stateIterator++) {
		if (stateToInsert->stateID == (*stateIterator)->stateID) {
			return;
		} else if (listsAreMatch(stateToInsert->coreRows, (*stateIterator)->coreRows)) {
//			cout << stateToInsert->stateID << " IS IDENTICAL TO " << (*stateIterator)->stateID << endl;
			stateToInsert->stateID = (*stateIterator)->stateID;
			stateToInsert = *stateIterator;
			return;
		}
	}
	dfaStates.push_back(stateToInsert);
}

bool LR0_DFA::listsAreMatch(list<LR0_DFAState_Row> first, list<LR0_DFAState_Row> second) {
	if (first.size() != second.size()) {
		return false;
	}
	bool previousWasMatched=true;
	for (auto firstRowIterator = first.begin();
		 firstRowIterator != first.end();
		 firstRowIterator++) {
		if (!previousWasMatched) {
			return false;
		}
		previousWasMatched = false;
		auto secondRowIterator = second.begin();
		while (secondRowIterator != second.end()) {
			if (*firstRowIterator == *secondRowIterator) {
				previousWasMatched = true;
				break;
			}
			secondRowIterator++;
		}
	}
	if (!previousWasMatched) {
		return false;
	}
	return true;
}

void LR0_DFA::addNewState(LR0_DFAState* stateToMove) {
	for (auto movesIterator = stateToMove->moves.begin();
		 movesIterator != stateToMove->moves.end();
		 movesIterator++) {
		
		list<LR0_DFAState_Row> rows = stateToMove->coreRows;
		rows.insert(rows.end(), stateToMove->minorRows.begin(), stateToMove->minorRows.end());
		
		for (auto rowsIterator = rows.begin();
			 rowsIterator != rows.end();
			 rowsIterator++) {
			if (rowsIterator->lookAhead != nullptr) {
				if (movesIterator->first != rowsIterator->lookAhead ||
					rowsIterator->nextState != nullptr) {
					continue;
				}
				
				rowsIterator->nextState = &movesIterator->second;
				
				if (movesIterator->second == nullptr) {
					movesIterator->second = new LR0_DFAState();
				}
				Element* nextLookAhead = findNextLookAhead(&(*rowsIterator));
				LR0_DFAState_Row newCoreRow(rowsIterator->variable,
											rowsIterator->rule,
											nextLookAhead,
											true);
				movesIterator->second->addCoreRow(newCoreRow);
				
			}
		}
		insertState(movesIterator->second);
	}
}

Element* LR0_DFA::findNextLookAhead(const LR0_DFAState_Row* row) {
	if (row->rule->elements.size() == 1 && (*row->rule->elements.begin())->isEpsilon) {
		cout << (*row->rule->elements.begin())->name << endl;
		return nullptr;
	}
	for (auto ruleIterator = row->rule->elements.begin();
		 ruleIterator != row->rule->elements.end();
		 ruleIterator++) {
		if (row->lookAhead == *ruleIterator) {
			ruleIterator++;
			if (ruleIterator != row->rule->elements.end()) {
				return *ruleIterator;
			} else {
				return nullptr;
			}
		}
	}
	return nullptr;
}



ostream& operator<< (ostream& output, const LR0_DFAState_Row& row) {
	string rowString="";
	bool lookAheadIsFound=false;
	
	for (auto ruleIterator = row.rule->elements.begin();
		 ruleIterator != row.rule->elements.end();
		 ruleIterator++) {
		if (!lookAheadIsFound && (*ruleIterator) == row.lookAhead) {
			rowString += " ^" + row.lookAhead->name;
			lookAheadIsFound = true;
		} else {
			rowString += " " + (*ruleIterator)->name;
		}
	}
	if (!lookAheadIsFound) {
		rowString += "^";
	}
	output << row.variable->name << " :" << rowString << " ;\t(" << row.isCore << ")\t" << row.nextState << endl;
	return output;
}

ostream& operator<< (ostream& output, const LR0_DFAState& state) {
	output << "\n============================";
	output << "STATE #" << state.stateID;
	output << "================================" << endl;
	for (auto rowIterator = state.coreRows.begin();
		 rowIterator != state.coreRows.end();
		 rowIterator++) {
		string ruleString="";
		bool lookAheadIsFound=false;
		for (auto ruleIterator = rowIterator->rule->elements.begin();
			 ruleIterator != rowIterator->rule->elements.end();
			 ruleIterator++) {
			if (!lookAheadIsFound &&
				(*ruleIterator) == rowIterator->lookAhead) {
				ruleString += " ^" + rowIterator->lookAhead->name;
				lookAheadIsFound = true;
			} else {
				ruleString += " " + (*ruleIterator)->name;
			}
		}
		if (!lookAheadIsFound) {
			ruleString += "^";
		}
		output << rowIterator->variable->name << " :" << ruleString << endl;
	}
	output << "--------------------------------------------------------------------" << endl;
	for (auto rowIterator = state.minorRows.begin();
		 rowIterator != state.minorRows.end();
		 rowIterator++) {
		string ruleString="";
		bool lookAheadIsFound=false;
		for (auto ruleIterator = rowIterator->rule->elements.begin();
			 ruleIterator != rowIterator->rule->elements.end();
			 ruleIterator++) {
			if (!lookAheadIsFound &&
				(*ruleIterator) == rowIterator->lookAhead) {
				ruleString += " ^" + rowIterator->lookAhead->name;
				lookAheadIsFound = true;
			} else {
				ruleString += " " + (*ruleIterator)->name;
			}
		}
		if (!lookAheadIsFound) {
			ruleString += "^";
		}
		output << rowIterator->variable->name << " :" << ruleString << endl;
	}
	output << "____________________________________________________________________" << endl;
	for (auto moveIterator = state.moves.begin();
		 moveIterator != state.moves.end();
		 moveIterator++) {
		output << moveIterator->first->name << " -> #";
		if (moveIterator->second != nullptr) {
			output << moveIterator->second->stateID << endl;
		} else {
			output << "NULL" << endl;
		}
	}
	output << "====================================================================\n" << endl;
	
	return output;
}


ostream& operator<< (ostream& output, LR0_DFA dfa) {
	output << "Total States: " << dfa.dfaStates.size() << endl;
	for (auto stateIterator = dfa.dfaStates.begin();
		 stateIterator != dfa.dfaStates.end();
		 stateIterator++) {
		output << *(*stateIterator) << endl;
	}
	return output;
}






