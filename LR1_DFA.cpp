#define COL_WIDTH 17
#include "TablePrinter.hpp"
#include "LR1_DFA.hpp"
#include "Grammar.hpp"


bool operator<(const ElementPtrList::iterator& rhs, const ElementPtrList::iterator& lhs)
{
	return (*rhs)->name < (*lhs)->name;
}

void unionList(ElementPtrList* firstList, const ElementPtrList& secondList) {
	if (secondList.empty()) {
		return;
	}
	firstList->insert(firstList->end() ,secondList.begin(), secondList.end());
	firstList->sort();
	firstList->erase(unique(firstList->begin(), firstList->end()), firstList->end());
}

bool listsAreMatch(ElementPtrList first, ElementPtrList second) {
	if (first.size() != second.size()) {
		return false;
	}
	bool previousWasWatched=true;
	for (auto firstIterator = first.begin();
		 firstIterator != first.end();
		 firstIterator++) {
		if (!previousWasWatched) {
			return false;
		}
		previousWasWatched = false;
		auto secondIterator = second.begin();
		while (secondIterator != second.end()) {
			if (*firstIterator == *secondIterator) {
				previousWasWatched = true;
				break;
			}
			secondIterator++;
		}
	}
	if (!previousWasWatched) {
		return false;
	}
	return true;
}

bool listsAreMatch(list<LR1_DFAState_Row> first, list<LR1_DFAState_Row> second) {
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


ElementPtrList::iterator findNextLookAhead(LR1_DFAState_Row row) {
	if (row.rule->elements.size() == 1 && (*row.rule->elements.begin())->isEpsilon) {
		return row.rule->elements.end();
	}
	for (auto ruleIterator = row.rule->elements.begin();
		 ruleIterator != row.rule->elements.end();
		 ruleIterator++) {
		if (row.lookAhead == ruleIterator) {
			ruleIterator++;
			if (ruleIterator != row.rule->elements.end()) {
				return ruleIterator;
			} else {
				return row.rule->elements.end();
			}
		}
	}
	return row.rule->elements.end();
}

void eliminateEpsilon(ElementPtrList* list) {
	list->sort();
	for (auto listIterator = list->begin();
		 listIterator != list->end();
		 listIterator++) {
		if ((*(*listIterator)).isEpsilon) {
			list->erase(listIterator);
		}
	}
}






LR1_DFAState_Row::LR1_DFAState_Row(Element* variable,
								   Rule* rule,
								   ElementPtrList::iterator lookAhead,
								   bool isCore,
								   ElementPtrList followList) {
	this->variable = variable;
	this->rule = rule;
	this->lookAhead = lookAhead;
	this->isCore = isCore;
	this->nextState = nullptr;
	this->followList = followList;
}

LR1_DFAState_Row& LR1_DFAState_Row::operator=(const LR1_DFAState_Row& row) {
	this->variable = row.variable;
	this->rule = row.rule;
	this->lookAhead = row.lookAhead;
	this->isCore = row.isCore;
	this->nextState = row.nextState;
	this->followList = row.followList;
	
	return *this;
}

bool LR1_DFAState_Row::operator==(const LR1_DFAState_Row& row) {
//	if (this->isCore == row.isCore &&
//		this->lookAhead == row.lookAhead &&
//		this->rule->ruleID == row.rule->ruleID &&
//		listsAreMatch(this->followList, row.followList)) {
	if (this->isCore == row.isCore &&
		this->lookAhead == row.lookAhead &&
		this->rule->ruleID == row.rule->ruleID) {
		if (!listsAreMatch(this->followList, row.followList)) {
			unionList(&this->followList, row.followList);
		}
		return true;
	}
	return false;
}

bool LR1_DFAState_Row::operator!=(const LR1_DFAState_Row& row) {
	return !(*this == row);
}










unsigned int LR1_DFAState::stateCount = 0;
unsigned int LR1_DFAState::stateUniqueIDs = 0;
Grammar* LR1_DFAState::grammar = nullptr;



LR1_DFAState::LR1_DFAState() {
	this->stateID = ++stateUniqueIDs;
	++stateCount;
}

LR1_DFAState::LR1_DFAState(Grammar* grammar) {
	LR1_DFAState::grammar = grammar;
	this->stateID = ++stateUniqueIDs;
	++stateCount;
}

LR1_DFAState::~LR1_DFAState() {
	--stateCount;
}

bool LR1_DFAState::rowExists(const LR1_DFAState_Row& rowToFind) {
	for (auto listIterator = coreRows.begin();
		 listIterator != coreRows.end();
		 listIterator++) {
		if (*listIterator == rowToFind) {
			return true;
		}
	}
	if (rowToFind.isCore) {
		return false;
	}
	for (auto listIterator = minorRows.begin();
		 listIterator != minorRows.end();
		 listIterator++) {
		if (*listIterator == rowToFind) {
			return true;
		}
	}
	return false;
}

void LR1_DFAState::addCoreRow(const LR1_DFAState_Row& rowToAdd) {
	if (!rowExists(rowToAdd)) {
		coreRows.push_back(rowToAdd);
		if (rowToAdd.lookAhead == rowToAdd.rule->elements.end()) {
			return;
		}
		moves[rowToAdd.lookAhead] = nullptr;
		expandRow(rowToAdd);
	}
}

void LR1_DFAState::expandRow(const LR1_DFAState_Row& rowToExpand) {
	if (rowToExpand.lookAhead == rowToExpand.rule->elements.end()) {
		return;
	}
	
	moves[rowToExpand.lookAhead] = nullptr;
	
	if ((*rowToExpand.lookAhead)->isTerminal) {
		return;
	}
	
	for (auto grammarIterator = grammar->grammar.begin();
		 grammarIterator != grammar->grammar.end();
		 grammarIterator++) {
		if ((*rowToExpand.lookAhead)->name == grammarIterator->first.name) {
			for (auto ruleIterator = grammarIterator->second.rules.begin();
				 ruleIterator != grammarIterator->second.rules.end();
				 ruleIterator++) {
				ElementPtrList::iterator newLookAhead = ruleIterator->elements.begin();
				if (ruleIterator->elements.size() == 1 && (*ruleIterator->elements.begin())->isEpsilon) {
					newLookAhead = ruleIterator->elements.end();
				}
				
				ElementPtrList followList;
				ElementPtrList::iterator nextLookAhead = findNextLookAhead(rowToExpand);
				
				if (nextLookAhead == rowToExpand.rule->elements.end()) {
					followList = rowToExpand.followList;
				} else {
					followList.insert(followList.begin(), (*nextLookAhead)->firstList.begin(), (*nextLookAhead)->firstList.end());
					eliminateEpsilon(&followList);
					// TO-DO: check for epsilon condition
				}
				
				
				LR1_DFAState_Row newRow((*rowToExpand.lookAhead),
										&(*ruleIterator),
										newLookAhead,
										false,
										followList);
				if (!rowExists(newRow)) {
					minorRows.push_back(newRow);
					expandRow(newRow);
				}
			}
			break;
		}
	}
}









LR1_DFA::LR1_DFA(Grammar* grammar) {
	LR1_DFAState* startState = new LR1_DFAState(grammar);	
	for (auto grammarIterator = grammar->startVariableIterator->second.rules.begin();
		 grammarIterator != grammar->startVariableIterator->second.rules.end();
		 grammarIterator++) {
		Element* startVariable = const_cast<Element*>(&grammar->startVariableIterator->first);
		ElementPtrList::iterator lookAhead = grammarIterator->elements.begin();
		ElementPtrList followList;
		followList.insert(followList.begin(), startVariable->followList.begin(), startVariable->followList.end());
		LR1_DFAState_Row newCoreRow(startVariable,
									&(*grammarIterator),
									lookAhead,
									true,
									followList);
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

void LR1_DFA::insertState(LR1_DFAState* stateToInsert) {
	for (auto stateIterator = dfaStates.begin();
		 stateIterator != dfaStates.end();
		 stateIterator++) {
		if (stateToInsert->stateID == (*stateIterator)->stateID) {
			return;
		} else if (listsAreMatch(stateToInsert->coreRows, (*stateIterator)->coreRows)) {
			//cout << stateToInsert->stateID << " IS IDENTICAL TO " << (*stateIterator)->stateID << endl;
			stateToInsert->stateID = (*stateIterator)->stateID;
			stateToInsert = *stateIterator;
			return;
		}
	}
	dfaStates.push_back(stateToInsert);
}

void LR1_DFA::addNewState(LR1_DFAState* stateToMove) {
	for (auto movesIterator = stateToMove->moves.begin();
		 movesIterator != stateToMove->moves.end();
		 movesIterator++) {
		
		list<LR1_DFAState_Row> rows = stateToMove->coreRows;
		rows.insert(rows.end(), stateToMove->minorRows.begin(), stateToMove->minorRows.end());
		
		for (auto rowsIterator = rows.begin();
			 rowsIterator != rows.end();
			 rowsIterator++) {
			if (rowsIterator->lookAhead != rowsIterator->rule->elements.end()) {
				if (movesIterator->first != rowsIterator->lookAhead ||
					rowsIterator->nextState != nullptr) {
					continue;
				}
				
				rowsIterator->nextState = &movesIterator->second;
				
				if (movesIterator->second == nullptr) {
					movesIterator->second = new LR1_DFAState();
				}
				ElementPtrList::iterator nextLookAhead = findNextLookAhead(*rowsIterator);

				LR1_DFAState_Row newCoreRow(rowsIterator->variable,
											rowsIterator->rule,
											nextLookAhead,
											true,
											rowsIterator->followList);
				movesIterator->second->addCoreRow(newCoreRow);
			}
		}
		insertState(movesIterator->second);
	}
}

void LR1_DFAState_Row::display(TablePrinter& tableOut) {
	string cellString = this->variable->name + " :";
	bool lookAheadIsFound=false;
	for (auto ruleIterator = this->rule->elements.begin();
		 ruleIterator != this->rule->elements.end();
		 ruleIterator++) {
		if (!lookAheadIsFound && ruleIterator == this->lookAhead) {
			cellString += " ^" + (*this->lookAhead)->name;
			lookAheadIsFound = true;
		} else {
			cellString += " " + (*ruleIterator)->name;
		}
	}
	if (!lookAheadIsFound) {
		cellString += "^";
	}
	tableOut.Print(cellString);
	cellString = "{ ";
	if (!this->followList.empty()) {
		auto listIterator = this->followList.begin();
		for (;
			 listIterator != --this->followList.end();
			 listIterator++) {
			cellString += (*listIterator)->name + " , ";
		}
		cellString += (*listIterator)->name;
	}
	cellString += " }";
	tableOut.Print(cellString);
}

void LR1_DFAState::display() {
	string stateString = "State ##" + to_string(this->stateID);
	int rowSize = int(this->coreRows.size() + this->minorRows.size() + this->moves.size())+2;
	TablePrinter tableOut(stateString, rowSize, 2, "dfa_out.txt");
	tableOut.SetHaveColDiv(true);
	tableOut.SetHaveColNumber(false);
	tableOut.SetColHeader(0, "Rule");
	tableOut.SetColHeader(1, "Follow");
	tableOut.SetHaveLineNumber(false);
	tableOut.SetColWidth(COL_WIDTH);
	for (auto rowIterator = this->coreRows.begin();
		 rowIterator != this->coreRows.end();
		 rowIterator++) {
		tableOut.SetHaveLineDiv(false);
		rowIterator->display(tableOut);
	}
	tableOut.SetHaveLineDiv(true);
	tableOut.Print("");
	tableOut.Print("");
	for (auto rowIterator = this->minorRows.begin();
		 rowIterator != this->minorRows.end();
		 rowIterator++) {
		tableOut.SetHaveLineDiv(false);
		rowIterator->display(tableOut);
	}
	tableOut.SetHaveLineDiv(true);
	tableOut.Print("");
	tableOut.Print("");
	tableOut.SetHaveLineDiv(false);
	for (auto moveIterator = this->moves.begin();
		 moveIterator != this->moves.end();
		 moveIterator++) {
		tableOut.Print((*moveIterator->first)->name);
		if (moveIterator->second != nullptr) {
			stateString = "#" + to_string(moveIterator->second->stateID);
			tableOut.Print(stateString);
		} else {
			tableOut.Print("NULL");
		}
	}
}

void LR1_DFA::display() {
	fstream dfaOutFile;
	dfaOutFile.open("dfa_out.txt");
	dfaOutFile << "Total States: " << this->dfaStates.size() << endl;
	for (auto stateIterator = this->dfaStates.begin();
		 stateIterator != this->dfaStates.end();
		 stateIterator++) {
		(*stateIterator)->display();
	}
}
