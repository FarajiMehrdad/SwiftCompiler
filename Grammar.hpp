#ifndef Grammar_cpp
#define Grammar_cpp

#include <iostream>
#include <string>
#include <list>
#include <map>

using namespace std;

typedef class Element Element;
typedef list<Element*> ElementPtrList;

class Element {
public:
	string name="";
	bool isTerminal=false;
	bool isEpsilon=false;
	bool isEpsilonable=false;
	bool isStartVariable=false;
	
	ElementPtrList firstList;
	ElementPtrList dependencyList;
	ElementPtrList recursiveDependencyList;
	bool isFirstCompleted=false;
	
	ElementPtrList followList;
	bool isFollowCompleted=false;
	
	void addFirst(Element* elementPointer);
	bool operator<(const Element& element) const;
};

struct Rule {
	ElementPtrList elements;
	unsigned int ruleID=0;
};

struct Productions {
	list<Rule> rules;
};

class Grammar {
public:
	Grammar();
	void readGrammar(ifstream& input);
	void addToGrammar(const Element& variableElement, const Productions& rules);
	void initFirst();
	void initFollow();
	Element* initElement(const string& name, const bool& isTerminal, const bool& isEpsilonable);
	ElementPtrList firstFinder(Element* elementPointerToFind);
	ElementPtrList followFinder(Element* elementPointerToFind);
	void unionList(ElementPtrList* firstList, const ElementPtrList& secondList);
	Element* findElement(const Element* elementPointerToFind, const ElementPtrList& list);
	map<Element, Productions>::iterator findElementInGrammar(const string& name);
	void eliminateEpsilon(ElementPtrList* list);
	bool isEpsilon(const string& name);
	void skip(const string& str,int& iterator, const char& toSkip);
	
	friend ostream& operator<<(ostream& output, Rule rule);
	friend ostream& operator<<(ostream& output, Grammar grammar);
	friend ostream& operator<<(ostream& output, Productions rules);
	
	map<Element, Productions> grammar;
	unsigned long int rulesCount;
	map<Element, Productions>::iterator startVariableIterator=grammar.begin();
	bool isStartVariableInitiated=false;
};

#endif
