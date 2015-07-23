#include <iostream>
#include <fstream>

#include "Grammar.hpp"
#include "LR0_DFA.hpp"
#include "LR1_DFA.hpp"
#include "LR1_ParseTable.hpp"
#include "LR1_Parser.hpp"

using namespace std;

int main() {
	ifstream inputFile;
	ofstream grammarOutFile, dfaOutFile, tableOutFile;
	inputFile.open("grammar.txt");
	grammarOutFile.open("grammar_out.txt");
	dfaOutFile.open("dfa_out.txt");
	tableOutFile.open("table_out.txt");
	
	if (!inputFile.is_open() || !grammarOutFile.is_open()) {
		cout << "File Not Found!" << endl;
		return 1;
	}
	
	Grammar swiftGrammar;
	
	swiftGrammar.readGrammar(inputFile);
	swiftGrammar.initFirst();
	swiftGrammar.initFollow();
	
	grammarOutFile << swiftGrammar;
	
	grammarOutFile.close();
	
	LR1_DFA lr1_dfa(&swiftGrammar);
	
	lr1_dfa.display();
	
	LR1_ParseTable lr1_parseTable(lr1_dfa);
	
	lr1_parseTable.display();
	
	inputFile.close();
	dfaOutFile.close();
	tableOutFile.close();
	
	TokenStream str;
	LR1_Parser lr1_parser(&swiftGrammar);
	lr1_parser.parse(&str);
	
	return 0;
}
