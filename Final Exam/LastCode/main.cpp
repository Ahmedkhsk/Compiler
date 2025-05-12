#include <bits/stdc++.h>
#include "utils.h" // Assuming utils.h contains split and trim

using namespace std;

class GrammarReader
{
    map<string, vector<vector<string>>> GrammarRules;
    map<string, vector<vector<string>>> grammarAfterRemoveEpso; // Stores grammar with direct A->e rules removed
    map<string, vector<vector<string>>> grammarAfterLastEdit; // Final grammar

public :
    set<string> N; // Set of nullable symbols
    string startSymbol;

    GrammarReader() : startSymbol("") {}

    void ReadGrammer(string filePath)
    {
        ifstream file(filePath);
        string line;
        bool isStart = true;

        if (!file)
        {
            cout << "Error opening " << filePath << endl;
            exit(1);
        }

        GrammarRules.clear();

        while (getline(file, line))
        {
            line = trim(line);
            if (line.empty() || line[0] == '#' || (line.length() >= 2 && line.substr(0,2) == "//")) continue;

            size_t arrow_pos = line.find("->");
            if (arrow_pos == string::npos) continue;

            string lhs = trim(line.substr(0, arrow_pos));
            string rhsPart = trim(line.substr(arrow_pos + 2));

            if (lhs.empty()) continue;

            if(isStart)
            {
                startSymbol = lhs;
                isStart = false;
            }

            vector<string> rhsAlternativesStr = split(rhsPart, '|');
            vector<vector<string>> parsedAlternatives;

            for (string& alt_str : rhsAlternativesStr)
            {
                alt_str = trim(alt_str);
                if (alt_str.empty()) continue;

                vector<string> symbols = split(alt_str, ' ');
                vector<string> final_symbols;
                for(const string& s_token : symbols) {
                    string trimmed_s_token = trim(s_token);
                    if (!trimmed_s_token.empty()) {
                        final_symbols.push_back(trimmed_s_token);
                    }
                }
                if (!final_symbols.empty()) {
                    parsedAlternatives.push_back(final_symbols);
                }
            }

            if (!parsedAlternatives.empty()) {
                GrammarRules[lhs].insert(GrammarRules[lhs].end(), parsedAlternatives.begin(), parsedAlternatives.end());
            }
        }
        file.close();
    }

    void determineNullableSymbols()
    {
        N.clear();
        bool changed = true;

        for(auto const& [nonTerminal, rules] : GrammarRules){
            for(const auto& rule : rules){
                if(rule.size() == 1 && rule[0] == "e"){
                    N.insert(nonTerminal);
                    break;
                }
            }
        }

        while(changed){
            changed = false;
            for(auto const& [nonTerminal, rules] : GrammarRules){
                if(N.count(nonTerminal)) continue;

                for(const auto& rule : rules){
                    if (rule.empty() || (rule.size()==1 && rule[0]== "e")) continue;

                    bool allSymbolsInRuleAreNullable = true;
                    for(const string& symbol : rule){
                        if(!N.count(symbol)){
                            allSymbolsInRuleAreNullable = false;
                            break;
                        }
                    }
                    if(allSymbolsInRuleAreNullable){
                        if(N.insert(nonTerminal).second) {
                            changed = true;
                        }
                        break;
                    }
                }
            }
        }
    }

    void prepareGrammarWithoutDirectEpsilonProductions() {
        grammarAfterRemoveEpso.clear();
        for(auto const& [nonTerminal, rules] : GrammarRules) {
            vector<vector<string>> keptRules;
            for(const auto& rule : rules) {
                if (!(rule.size() == 1 && rule[0] == "e")) {
                    keptRules.push_back(rule);
                }
            }
            if (!keptRules.empty()) {
                grammarAfterRemoveEpso[nonTerminal] = keptRules;
            }
        }
    }

    void generateFinalGrammar()
    {
        grammarAfterLastEdit.clear();

        for (auto const& [nonTerminal, rulesFromPreparedGrammar] : grammarAfterRemoveEpso)
        {
            set<vector<string>> newRuleSetForThisNonTerminal;

            for (const auto& rule : rulesFromPreparedGrammar)
            {
                int n = rule.size();
                vector<int> nullableSymbolIndicesInRule;
                for(int i=0; i<n; ++i){
                    if(N.count(rule[i])){
                        nullableSymbolIndicesInRule.push_back(i);
                    }
                }

                int numNullableSymbolsInThisRule = nullableSymbolIndicesInRule.size();
                for (int i = 0; i < (1 << numNullableSymbolsInThisRule); ++i)
                {
                    vector<string> currentNewRuleRHS;
                    for(int ruleSymbolIdx=0; ruleSymbolIdx<n; ++ruleSymbolIdx) {
                        bool isNullableAtIndex = N.count(rule.at(ruleSymbolIdx));
                        bool removeThisNullable = false;
                        if (isNullableAtIndex) {
                            int nullableInstanceOrder = -1;
                            for(int k=0; k<nullableSymbolIndicesInRule.size(); ++k){
                                if(nullableSymbolIndicesInRule[k] == ruleSymbolIdx){
                                    nullableInstanceOrder = k;
                                    break;
                                }
                            }
                            if (nullableInstanceOrder != -1 && ((i >> nullableInstanceOrder) & 1)) {
                                removeThisNullable = true;
                            }
                        }
                        if (!removeThisNullable) {
                            currentNewRuleRHS.push_back(rule.at(ruleSymbolIdx));
                        }
                    }

                    if (!currentNewRuleRHS.empty()) {
                        newRuleSetForThisNonTerminal.insert(currentNewRuleRHS);
                    }
                }
            }
            if (!newRuleSetForThisNonTerminal.empty()) {
                 grammarAfterLastEdit[nonTerminal].assign(newRuleSetForThisNonTerminal.begin(), newRuleSetForThisNonTerminal.end());
            }
        }

        if (N.count(startSymbol)) {
            bool startSymbolAlreadyHasEpsilonRule = false;
            if(grammarAfterLastEdit.count(startSymbol)){
                for(const auto& r_vec : grammarAfterLastEdit[startSymbol]){
                    if(r_vec.size() == 1 && r_vec[0] == "e"){
                        startSymbolAlreadyHasEpsilonRule = true;
                        break;
                    }
                }
            }
            if(!startSymbolAlreadyHasEpsilonRule){
                 grammarAfterLastEdit[startSymbol].push_back({"e"});
            }
        }

        for (auto& pair : grammarAfterLastEdit) {
            if (!pair.second.empty()) {
                sort(pair.second.begin(), pair.second.end());
                pair.second.erase(unique(pair.second.begin(), pair.second.end()), pair.second.end());
            }
            if (pair.second.empty() && !(pair.first == startSymbol && N.count(startSymbol))) {
            }
        }
    }

    void printSelectedGrammar(const map<string, vector<vector<string>>>& gramToPrint, const string& title) const
    {
        cout << title;
        vector<string> sortedLHSKeys;
        for(auto const& [lhs_key, rules_val] : gramToPrint) sortedLHSKeys.push_back(lhs_key);
        sort(sortedLHSKeys.begin(), sortedLHSKeys.end());

        for(const string& lhs : sortedLHSKeys)
        {
            const vector<vector<string>>& rules = gramToPrint.at(lhs);
            // Corrected char 'e' to string "e" comparison
            bool is_S_with_only_e = (lhs == startSymbol && N.count(startSymbol) && rules.size() == 1 && rules[0].size() == 1 && rules[0][0] == "e");
            if (rules.empty() && !is_S_with_only_e) {
                 continue;
            }

            cout << lhs << " -> ";
            for (size_t i = 0; i < rules.size(); ++i)
            {
                const vector<string>& rule_symbols = rules[i];
                if (rule_symbols.empty()) {
                    cout << "e";
                } else {
                    for (size_t j = 0; j < rule_symbols.size(); ++j)
                    {
                        cout << rule_symbols[j];
                        if (j < rule_symbols.size() - 1)
                            cout << " ";
                    }
                }
                if (i < rules.size() - 1)
                    cout << " | ";
            }
            cout << endl;
        }
    }

    void printOriginalGrammar(const string& title) const {
        cout << title << endl;
        printSelectedGrammar(GrammarRules, "");
    }

    void printFinalGrammar(const string& title) const {
        cout << title << endl;
        printSelectedGrammar(grammarAfterLastEdit, "");
    }

    const set<string>& getNullableSet() const { return N; }

};

int main()
{
    ofstream grammarFile("Grammar.txt");
    if (grammarFile.is_open()) {
        grammarFile << "S -> A B C | D\n";
        grammarFile << "A -> a A | e\n";
        grammarFile << "B -> b B | e\n";
        grammarFile << "C -> c\n";
        grammarFile << "D -> e\n";
        grammarFile.close();
    } else {
        cout << "Failed to create Grammar.txt for testing." << endl;
        return 1;
    }

    GrammarReader gr;
    gr.ReadGrammer("Grammar.txt");

    gr.printOriginalGrammar("Original Grammar:");

    gr.determineNullableSymbols();
    cout << endl << "Nullable symbols identified: [ ";
    for(const auto& n_sym : gr.getNullableSet())
        cout << n_sym << " ";
    cout << "]" << endl;

    gr.prepareGrammarWithoutDirectEpsilonProductions();
    gr.generateFinalGrammar();

    cout << endl;
    gr.printFinalGrammar("Final Grammar (after epsilon elimination):");

    return 0;
}

