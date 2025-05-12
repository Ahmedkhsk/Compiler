#include<iostream>
#include<string>
#include<map>
#include<vector>
#include<fstream>
#include<sstream>

#include "utils.h"
using namespace std;


class GrammarReader
{
	// Hashmap to store the production rules
	// keys are the LHS non terminals
	// value is the vector hold all possible RHS production rule result 
	// for example S -> A B | t its value would be vector of vectors, the first vector hold two string A, B 
	// the other vector holds only one string which is t
	map<string,vector<vector<string>>> GrammarRules;

public :
	void ReadGrammer(string filePath)
	{
		ifstream file(filePath);
		string line;
		stringstream ss;

		if (!file)
		{
			cout << "Error opening " << filePath << " " << endl;
			system("pause");
			exit(1);
		}

		while (getline(file, line))
		{
			// Example line: "S -> A B | t"

			// split function is defined in utils.h, 
			// it return vector of string that was splitted with the provided delimeter
			// so it split the line to get the LHS and RHS parts
			vector<string> splitLine = split(line, "->");
			string lhs = splitLine[0];
			string rhsPart = splitLine[1];

			// split RHS into alternatives around "|"
			vector<string> rhsAlternatives = split(rhsPart, "|");
			vector<vector<string>> parsedAlternatives;

			for (int i = 0; i < rhsAlternatives.size(); ++i)
			{
				vector<string> symbols = split(rhsAlternatives[i], " ");
				parsedAlternatives.push_back(symbols);
			}

			GrammarRules[lhs]= parsedAlternatives;
		}
	}


	void printGrammer()
	{
		for (auto it = GrammarRules.begin(); it != GrammarRules.end(); ++it)
		{
			cout << it->first << " -> ";
			const vector<vector<string>> rules = it->second;

			for (int i = 0; i < rules.size(); ++i)
			{
				const vector<string> rule = rules[i];
				for (int j = 0; j < rule.size(); ++j)
				{
					cout << rule[j];
					if (j < rule.size() - 1)
						cout << " ";
				}
				if (i < rules.size() - 1)
					cout << " | ";
			}

			cout << endl;
		}
	}

};

void main()
{

	GrammarReader gr;
	gr.ReadGrammer("Grammar.txt");
	gr.printGrammer();
	system("pause");
}