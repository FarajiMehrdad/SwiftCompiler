#include "LR1_Parser.hpp"

TokenStream::TokenStream(const string& name) {
	ifstream input;
	input.open(name);
	
	char tempChar;
	string tempString = "";
	
	while (!input.eof()) {
		input.get(tempChar);
		
		switch (tempChar) {
			case '<':
				input.get(tempChar);
				while (!input.eof() && tempChar!='>') {
					tempString += tempChar;
					input.get(tempChar);
					if (tempChar == ',') {
						break;
					}
				}
			case ',':
				while (!input.eof() && tempChar!='>') {
					input.get(tempChar);
				}
			case '>':
				if (tempChar == '>' && !input.eof()) {
					input.get(tempChar);
					if (tempChar != '<') {
						tempString += tempChar;
					}
				}
				tokenStream.push_back(tempString);
				tempString = "";
				break;
			default:
				tempString += tempChar;
				break;
		}
	}
	
	auto lastToken = --tokenStream.end();
	*lastToken = lastToken->substr(0, lastToken->size()-1);
	
//	for (auto tokenIterator=tokenStream.begin();
//		 tokenIterator!=tokenStream.end();
//		 tokenIterator++) {
//		cout << *tokenIterator << endl;
//	}
}

LR1_Parser::LR1_Parser() {
	
}

LR1_Parser::LR1_Parser(Grammar* grammar) {
	this->dfa = new LR1_DFA(grammar);
	this->parseTable = new LR1_ParseTable(*dfa);
	LR1_DFAState* startState = nullptr;
	for (auto stateIterator = dfa->dfaStates.begin();
		 stateIterator != dfa->dfaStates.end();
		 stateIterator++) {
		if ((*stateIterator)->stateID == 1) {
			startState = *stateIterator;
			break;
		}
	}
	if (startState != nullptr) {
		stateStack.push(startState);
	} else {
		exit(1);
	}
}


void LR1_Parser::parse(TokenStream* tokenStream) {
	auto tokenIterator = tokenStream->tokenStream.begin();
	auto tokenIteratorLookAhead = tokenIterator;
	if (tokenIteratorLookAhead != tokenStream->tokenStream.end()) tokenIteratorLookAhead++;
	while (tokenIterator != tokenStream->tokenStream.end()) {
		tokenStack.push(*tokenIterator);
		int columnNumber=-1;
		for (auto terminalIterator = parseTable->grammarTerminals.begin();
			 terminalIterator != parseTable->grammarTerminals.end();
			 terminalIterator++) {
			if (*tokenIterator == (*terminalIterator)->name) {
				break;
			}
			columnNumber++;
		}
		
		
		LR1_DFAState* currentState = stateStack.top();
		cout << currentState->stateID << endl;
		cout << parseTable->table[currentState].terminalCells->size() << endl;
		for (auto columnIterator = parseTable->table[currentState].terminalCells->begin();
			 columnIterator != parseTable->table[currentState].terminalCells->end();
			 columnIterator++) {
			if (--columnNumber == 0) {
				LR1_ParseTableTerminalCell* currentCell = &(*columnIterator);
				if (currentCell->action == Error) {
					cout << "Error: No move for <" << *tokenIterator << "> in state #" << currentState->stateID << endl;
				} else if (currentCell->action == Accept) {
					if (stateStack.size() == 1 && tokenStack.size() == 1) {
						cout << "Input Accepted." << endl;
					} else {
						cout << "Though some errors occured, Input Accepted." << endl;
					}
					return;
				} else if (currentCell->action == Shift) {
					stateStack.push(currentCell->movingRow);
					tokenIterator++;
					tokenStack.push(*tokenIterator);
				} else if (currentCell->action == Reduce) {
					for (auto count = currentCell->reducingRule->elements.size();
						 count > 0;
						 count--) {
						stateStack.pop();
						tokenStack.pop();
					}
				}
				break;
			}
		}
		tokenIterator++;
	}
}



















