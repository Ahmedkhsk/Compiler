#include <bits/stdc++.h>
#include "utils.h"
using namespace std;
class GrammarReader
{
    // Hashmap to store the production rules
    // keys are the LHS non terminals
    // value is the vector hold all possible RHS production rule result
    // for example S -> A B | t its value would be vector of vectors, the first vector hold two string A, B
    // the other vector holds only one string which is t
    map<string, vector<vector<string>>> GrammarRules;
    map<string, vector<vector<string>>> grammarAfterRemoveEpso;
    map<string, vector<vector<string>>> grammarAfterLastEdit;

public :
    set<string> N;
    string startSymbol;

    void ReadGrammer(string filePath)
    {
        ifstream file(filePath);
        string line;
        stringstream ss;
        bool isStart = true;

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


            if(isStart)
            {
                startSymbol = lhs;
                isStart = false;
            }


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

    bool findEpso(vector<string> rule)
    {
        for(auto x : rule)
            if(x == "e")
                return false;

        return true;
    }

    bool isNullSymbol(string s)
    {
        for(auto symbol : N)
            if(symbol == s)
                return true;

        return false;
    }

    void removeEpso()
    {
        for(auto prod : GrammarRules)
        {
            string nonTermnal = prod.first;

            vector<vector<string>> setofRules;
            for(auto rule : prod.second)
            {
                if(findEpso(rule))
                {
                    vector<string> setofSymbols;
                    for(auto symbol : rule)
                        setofSymbols.push_back(symbol);

                    setofRules.push_back(setofSymbols);
                }
                else
                {
                    N.insert(nonTermnal);
                }
            }
            N.insert(startSymbol);
            grammarAfterRemoveEpso[nonTermnal] = setofRules;
        }
    }

    void LastEditofGrmmar()
    {
        for(auto prod : grammarAfterRemoveEpso)
        {
            string nonTerminal;
            vector<vector<string>> setofRules;
            for(auto rule : prod.second)
            {

                vector<string> setofSymbols;

                for(int i = 0 ; i < rule.size() ; i++)
                {
                    setofSymbols.push_back(rule[i]);
                }

                for(int i = 0 ; i < rule.size() ; i++)
                {
                    if(isNullSymbol(rule[i]))
                    {



                    }
                }
            }

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

    void printGrammerWithoutE()
    {
        for (auto it = grammarAfterRemoveEpso.begin(); it != grammarAfterRemoveEpso.end(); ++it)
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

int main()
{
    GrammarReader gr;
    gr.ReadGrammer("Grammar.txt");

    cout<<"Before Remove e:"<<endl;
    gr.printGrammer();

    gr.removeEpso();

    cout<<endl<<"After Remove e:"<<endl;
    gr.printGrammerWithoutE();

    cout<<endl<<"all nullable symbols [ ";

    for(auto n : gr.N)
        cout<<n<<" ";

    cout<<"]"<<endl;


    return 0;
}
