#include "Grammar.hpp"
#include <fstream>


void Element::addFirst(Element* elementPointer) {
	firstList.push_back(elementPointer);
}

bool Element::operator<(const Element& element) const {
	return name < element.name;
}



Grammar::Grammar(){
	rulesCount = 0;
}

void Grammar::readGrammar(ifstream& input) {
	string line, variableName="", elementName="";
	int charIterator=0, skipped=0, getLines=0;;
	bool isTerminal=false;
	bool isEpsilonable=false;
	Element element;
	Element* elementPointer;
	Element* variablePointer;
	Rule rule;
	Productions rules;
	
	while (!input.eof()) {
		getline(input, line);
		getLines++;
		charIterator = 0;
		if (charIterator<line.size() && (line.at(0) == '/' || line.at(0) == '\n')) {
			skipped++;
			continue;
		}
		while(charIterator<line.size() && line.at(charIterator)!=' ') {
			if (line.at(charIterator)=='\n' ||
				line.at(charIterator)=='\t' ||
				line.at(charIterator)==' '  ||
				line.empty()) {
				continue;
			}
			
			variableName += line.at(charIterator);
			charIterator++;
		}
		// var_name has been read
		
		if (variableName == "") {
			continue;
		}
		
		skip(line, charIterator, ' ');
		skip(line, charIterator, ':');
		skip(line, charIterator, ' ');
		
		while (charIterator<line.size() && line.at(charIterator)!=';') {
			if (charIterator<line.size() && line.at(charIterator)=='|') {
				rule.ruleID++;
				rules.rules.push_back(rule);
				rule.elements.clear();
				
				skip(line, charIterator, '|');
				skip(line, charIterator, ' ');
			}
			
			if (charIterator<line.size() && line.at(charIterator) == '\'') {
				isTerminal = true;
				skip(line, charIterator, '\'');
				while (charIterator<line.size() && line.at(charIterator)!='\'') {
					elementName += line.at(charIterator);
					charIterator++;
				}
				skip(line, charIterator, '\'');
			} else {
				while (charIterator<line.size() && line.at(charIterator)!=' ') {
					elementName += line.at(charIterator);
					charIterator++;
				}
			}
			
			if (elementName == "") {
				continue;
			}
			if (isEpsilon(elementName)) {
				isEpsilonable = true;
			}
			elementPointer = initElement(elementName, isTerminal, isEpsilonable);
			element = *elementPointer;
			
			rule.elements.push_back(elementPointer);
			elementName = "";
			isTerminal = false;
			
			skip(line, charIterator, ' ');
		}
		
		rule.ruleID++;
		rules.rules.push_back(rule);
		rule.elements.clear();
		
		variablePointer = initElement(variableName, isTerminal, isEpsilonable);
		if (isEpsilonable) {
			variablePointer->isEpsilonable = true;
		}
		
		addToGrammar(*variablePointer, rules);
		
		if (!isStartVariableInitiated) {
			startVariableIterator = findElementInGrammar(variableName);
			isStartVariableInitiated = true;
			Element* eof = initElement("^$", true, false);
			Element* startVariable = const_cast<Element*>(&startVariableIterator->first);
			startVariable->isStartVariable = true;
			startVariable->followList.push_back(eof);
		}
		
		rules.rules.clear();
		variableName = "";
		isEpsilonable = false;
	}
	cout << getLines << " lines read." << endl;
	cout << skipped << " lines skipped." << endl;
}

void Grammar::addToGrammar(const Element& variableElement, const Productions& rules) {
	auto i=grammar.begin();
	for (;
		 i!=grammar.end();
		 i++) {
		if (i->first.name == variableElement.name) {
			grammar.at(variableElement).rules.insert(grammar.at(variableElement).rules.end(), rules.rules.begin(), rules.rules.end());
			return;
		}
	}
	grammar[variableElement] = rules;
}

void Grammar::initFirst() {
	for (auto grammarIterator = grammar.begin();
		 grammarIterator != grammar.end();
		 grammarIterator++) {
		if (grammarIterator->first.isTerminal) {
			Element* elementPointer = const_cast<Element*>(&grammarIterator->first);
			elementPointer->addFirst(elementPointer);
			elementPointer->isFirstCompleted = true;
		}
	}
	for (auto grammarIterator = grammar.begin();
		 grammarIterator != grammar.end();
		 grammarIterator++) {
		if (!grammarIterator->first.isTerminal) {
			Element* elementPointer = const_cast<Element*>(&grammarIterator->first);
			unionList(&elementPointer->firstList, firstFinder(elementPointer));
		}
	}
	for (auto grammarIterator = grammar.begin();
		 grammarIterator != grammar.end();
		 grammarIterator++) {
		Element* elementPointer = const_cast<Element*>(&grammarIterator->first);
		for (auto recursiveDependencyListIterator = elementPointer->recursiveDependencyList.begin();
			 recursiveDependencyListIterator != elementPointer->recursiveDependencyList.end();
			 recursiveDependencyListIterator++) {
			unionList(&elementPointer->firstList, (*recursiveDependencyListIterator)->firstList);
		}
	}
	for (auto grammarIterator = grammar.begin();
		 grammarIterator != grammar.end();
		 grammarIterator++) {
		Element* elementPointer = const_cast<Element*>(&grammarIterator->first);
		for (auto rulesIterator = grammarIterator->second.rules.begin();
			 rulesIterator != grammarIterator->second.rules.end();
			 rulesIterator++) {
			if (rulesIterator->elements.size() == 1 && (*rulesIterator->elements.begin())->isEpsilonable) {
				elementPointer->isEpsilonable = true;
				break;
			}
		}
		if (!elementPointer->isEpsilonable) {
			eliminateEpsilon(&elementPointer->firstList);
		}
		elementPointer->dependencyList.clear();
		elementPointer->recursiveDependencyList.clear();
	}
}

void Grammar::initFollow() {
	for (auto grammarIterator = grammar.begin();
		 grammarIterator != grammar.end();
		 grammarIterator++) {
		if (grammarIterator->first.isTerminal) {
			continue;
		}
		Element* elementPointer = const_cast<Element*>(&grammarIterator->first);
		unionList(&elementPointer->followList, followFinder(elementPointer));
	}
	for (auto grammarIterator = grammar.begin();
		 grammarIterator != grammar.end();
		 grammarIterator++) {
		if (grammarIterator->first.isTerminal) {
			continue;
		}
		Element* elementPointer = const_cast<Element*>(&grammarIterator->first);
		for (auto recursiveDependencyListIterator = elementPointer->recursiveDependencyList.begin();
			 recursiveDependencyListIterator != elementPointer->recursiveDependencyList.end();
			 recursiveDependencyListIterator++) {
			unionList(&elementPointer->followList, (*recursiveDependencyListIterator)->followList);
		}
	}
	for (auto grammarIterator = grammar.begin();
		 grammarIterator != grammar.end();
		 grammarIterator++) {
		Element* elementPointer = const_cast<Element*>(&grammarIterator->first);
		eliminateEpsilon(&elementPointer->followList);
		elementPointer->dependencyList.clear();
		elementPointer->recursiveDependencyList.clear();
	}
}


Element* Grammar::initElement(const string& name, const bool& isTerminal, const bool& isEpsilonable) {
	auto grammarIterator = grammar.begin();
	for (;
		 grammarIterator != grammar.end();
		 grammarIterator++) {
		if (grammarIterator->first.name == name) {
			Element* elementPointer = const_cast<Element*>(&grammarIterator->first);
			return elementPointer;
		}
	}
	
	Element* elementPointer = new Element;
	elementPointer->name = name;
	elementPointer->isTerminal = isTerminal;
	elementPointer->isEpsilonable = isEpsilonable;
	if (isEpsilon(name)) {
		elementPointer->isEpsilon = true;
		elementPointer->isEpsilonable = true;
	} else {
		elementPointer->isEpsilon = false;
	}
	
	grammar[*elementPointer] = *new Productions;
	for (grammarIterator = grammar.begin();
		 grammarIterator != grammar.end();
		 grammarIterator++) {
		if (grammarIterator->first.name == name) {
			elementPointer = const_cast<Element*>(&grammarIterator->first);
			return elementPointer;
		}
	}
	return elementPointer;
}

ElementPtrList Grammar::firstFinder(Element* elementPointerToFind) {
	if (elementPointerToFind->isFirstCompleted) {
		return elementPointerToFind->firstList;
	}
	for (auto grammarIterator = grammar.begin();
		 grammarIterator != grammar.end();
		 grammarIterator++) {
		if (grammarIterator->first.name == elementPointerToFind->name) {
			for (auto rulesIterator = grammarIterator->second.rules.begin();
				 rulesIterator != grammarIterator->second.rules.end();
				 rulesIterator++) {
				Element* elementPointer = const_cast<Element*>(*rulesIterator->elements.begin());
				if (elementPointerToFind->name == elementPointer->name) {
					continue;
				} else if (elementPointer->isTerminal) {
					unionList(&elementPointerToFind->firstList, (*rulesIterator->elements.begin())->firstList);
				} else {
					if (findElement(elementPointer, elementPointerToFind->recursiveDependencyList) == (*elementPointerToFind->recursiveDependencyList.end())) {
						if (findElement(elementPointer, elementPointerToFind->dependencyList) == (*elementPointerToFind->dependencyList.end())) {
							elementPointerToFind->dependencyList.push_back(elementPointer);
						} else {
							elementPointerToFind->dependencyList.remove(elementPointer);
							elementPointerToFind->recursiveDependencyList.push_back(elementPointer);
							if (elementPointer->isEpsilonable) {
								auto ruleIterator = rulesIterator->elements.begin();
								ruleIterator++;
								if (ruleIterator != rulesIterator->elements.end()) {
									elementPointer = const_cast<Element*>((*ruleIterator));
									if (elementPointerToFind->name == elementPointer->name) {
										continue;
									}
									unionList(&elementPointerToFind->firstList, firstFinder(elementPointer));
								}
							}
							continue;
						}
					} else {
						if (elementPointer->isEpsilonable) {
							auto ruleIterator = rulesIterator->elements.begin();
							ruleIterator++;
							if (ruleIterator != rulesIterator->elements.end()) {
								elementPointer = const_cast<Element*>((*ruleIterator));
								if (elementPointerToFind->name == elementPointer->name) {
									continue;
								}
								unionList(&elementPointerToFind->firstList, firstFinder(elementPointer));
							}
						}
						continue;
					}
					unionList(&elementPointerToFind->firstList, firstFinder(elementPointer));
					if (elementPointer->isEpsilonable) {
						auto ruleIterator = rulesIterator->elements.begin();
						ruleIterator++;
						if (ruleIterator != rulesIterator->elements.end()) {
							elementPointer = const_cast<Element*>((*ruleIterator));
							if (elementPointerToFind->name == elementPointer->name) {
								continue;
							}
							unionList(&elementPointerToFind->firstList, firstFinder(elementPointer));
						}
					}
				}
			}
			elementPointerToFind->isFirstCompleted = true;
			return elementPointerToFind->firstList;
		}
	}
	return elementPointerToFind->firstList;
}

ElementPtrList Grammar::followFinder(Element* elementPointerToFind) {
	if (elementPointerToFind->isFollowCompleted) {
		return elementPointerToFind->followList;
	}
	for (auto grammarIterator = grammar.begin();
		 grammarIterator != grammar.end();
		 grammarIterator++) {
		if (grammarIterator->first.isTerminal) continue;
		for (auto rulesIterator = grammarIterator->second.rules.begin();
			 rulesIterator != grammarIterator->second.rules.end();
			 rulesIterator++) {
			for (auto ruleIterator = rulesIterator->elements.begin();
				 ruleIterator != rulesIterator->elements.end();
				 ruleIterator++) {
				if ((*ruleIterator)->isTerminal) continue;
				if ((*ruleIterator)->name == elementPointerToFind->name) {
					Element* elementPointer = const_cast<Element*>(&grammarIterator->first);
					if (ruleIterator == --rulesIterator->elements.end()) {
						if (findElement(elementPointer, elementPointerToFind->recursiveDependencyList) == *elementPointerToFind->recursiveDependencyList.end()) {
							if (findElement(elementPointer, elementPointerToFind->dependencyList) == *elementPointerToFind->dependencyList.end()) {
								elementPointerToFind->dependencyList.push_back(elementPointer);
							} else {
								elementPointerToFind->dependencyList.remove(elementPointer);
								elementPointerToFind->recursiveDependencyList.push_back(elementPointer);
								continue;
							}
						} else {
							continue;
						}
						unionList(&elementPointerToFind->followList, followFinder(elementPointer));
					} else {
						unionList(&elementPointerToFind->followList, (*(++ruleIterator))->firstList);
						if ((*ruleIterator)->isEpsilonable) {
							if (ruleIterator == --rulesIterator->elements.end()) {
								if (findElement(elementPointer, elementPointerToFind->recursiveDependencyList) == *elementPointerToFind->recursiveDependencyList.end()) {
									if (findElement(elementPointer, elementPointerToFind->dependencyList) == *elementPointerToFind->dependencyList.end()) {
										elementPointerToFind->dependencyList.push_back(elementPointer);
									} else {
										elementPointerToFind->dependencyList.remove(elementPointer);
										elementPointerToFind->recursiveDependencyList.push_back(elementPointer);
										continue;
									}
								} else {
									continue;
								}
								unionList(&elementPointerToFind->followList, followFinder(elementPointer));
							} else {
								while (ruleIterator != --rulesIterator->elements.end() && (*ruleIterator)->isEpsilonable) {
									unionList(&elementPointerToFind->followList, (*(++ruleIterator))->firstList);
								}
								unionList(&elementPointerToFind->followList, (*ruleIterator)->firstList);
								if ((*ruleIterator)->isEpsilonable) {
									if (findElement(elementPointer, elementPointerToFind->recursiveDependencyList) == *elementPointerToFind->recursiveDependencyList.end()) {
										if (findElement(elementPointer, elementPointerToFind->dependencyList) == *elementPointerToFind->dependencyList.end()) {
											elementPointerToFind->dependencyList.push_back(elementPointer);
										} else {
											elementPointerToFind->dependencyList.remove(elementPointer);
											elementPointerToFind->recursiveDependencyList.push_back(elementPointer);
											continue;
										}
									} else {
										continue;
									}
									unionList(&elementPointerToFind->followList, followFinder(elementPointer));
								}
							}
						}
					}
				}
			}
		}
	}
	elementPointerToFind->isFollowCompleted = true;
	return elementPointerToFind->followList;
}

void Grammar::unionList(ElementPtrList* firstList, const ElementPtrList& secondList) {
	if (secondList.empty()) {
		return;
	}
	firstList->insert(firstList->end() ,secondList.begin(), secondList.end());
	firstList->sort();
	firstList->erase(unique(firstList->begin(), firstList->end()), firstList->end());
}

Element* Grammar::findElement(const Element* elementPointerToFind, const ElementPtrList& list) {
	for (auto listIterator = list.begin();
		 listIterator != list.end();
		 listIterator++) {
		if (elementPointerToFind->name == (*listIterator)->name) {
			Element* elementPointer = const_cast<Element*>(*listIterator);
			return elementPointer;
		}
	}
	return *list.end();
}

map<Element, Productions>::iterator Grammar::findElementInGrammar(const string& name) {
	for (auto grammarIterator = grammar.begin();
		 grammarIterator != grammar.end();
		 grammarIterator++) {
		if (grammarIterator->first.name == name) {
			return grammarIterator;
		}
	}
	return grammar.end();
}


void Grammar::eliminateEpsilon(ElementPtrList* list) {
	list->sort();
	for (auto listIterator = list->begin();
		 listIterator != list->end();
		 listIterator++) {
		if ((*(*listIterator)).isEpsilon) {
			list->erase(listIterator);
		}
	}
}

bool Grammar::isEpsilon(const string& name) {
	if (name == "epsilon") {
		return true;
	}
	return false;
}

void Grammar::skip(const string& str, int& iterator, const char& toSkip) {
	while (iterator<str.size() && str.at(iterator)==toSkip) {
		iterator++;
	}
}


ostream& operator<<(ostream& output, Rule rhs) {
	if (rhs.elements.empty()) return output;
	auto ruleIterator = rhs.elements.begin();
	for (;
		 ruleIterator != rhs.elements.end();
		 ruleIterator++) {
		if ((*ruleIterator)->isTerminal) {
			output << '\'' << (*ruleIterator)->name << "\' ";
		} else {
			output << (*ruleIterator)->name << ' ';
		}
	}
	output << "(" << rhs.ruleID << ") ";
	return output;
}

ostream& operator<<(ostream& output, Productions rhs) {
	if (rhs.rules.empty()) {
		return output;
	}
	auto rulesIterator = rhs.rules.begin();
	for (;
		 rulesIterator != --rhs.rules.end();
		 rulesIterator++) {
		output << *rulesIterator << "| ";
	}
	output << *rulesIterator;
	return output;
}

ostream& operator<<(ostream& output, Grammar rhs) {
	string terminalOut="@TERMINAL Set: { ";
	for (auto grammarIterator = rhs.grammar.begin();
		 grammarIterator != rhs.grammar.end();
		 grammarIterator++) {
		if (grammarIterator->first.isTerminal) {
			if (grammarIterator == --rhs.grammar.end()) {
				terminalOut += grammarIterator->first.name;
				continue;
			}
			terminalOut += grammarIterator->first.name + " , ";
			continue;
		}
		output << grammarIterator->first.name << " : " << grammarIterator->second << ";" << endl;
		if (!grammarIterator->first.firstList.empty()) {
			output << "\tFirst Set = { ";
			auto listIterator = grammarIterator->first.firstList.begin();
			for (;
				 listIterator != --grammarIterator->first.firstList.end();
				 listIterator++) {
				output << (*listIterator)->name << " , ";
			}
			output << (*listIterator)->name << " }" << endl;
		}
		if (!grammarIterator->first.followList.empty()) {
			output << "\tFollow Set = { ";
			auto listIterator = grammarIterator->first.followList.begin();
			for (;
				 listIterator != --grammarIterator->first.followList.end();
				 listIterator++) {
				output << (*listIterator)->name << " , ";
			}
			output << (*listIterator)->name << " }" << endl;
		} else if (grammarIterator->first.isStartVariable) {
			output << "\t***Start Variable***" << endl;
		} else {
			output << "\tWARNING! Useless Rule!" << endl;
		}
		output << endl;
	}
	terminalOut += " }";
	output << terminalOut;
	return output;
}