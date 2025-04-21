#include <bits/stdc++.h>

using namespace std;

class Parser
{
public:
    unordered_map<string, vector<vector<string>>> Productions;
    unordered_map<string, set<string>> firstSets;
    unordered_map<string, set<string>> followSets;
    map<pair<string,string>,string> table;
    string startSymbol;

    void readProductions(string path)
    {
        ifstream productionFile(path);
        string line;
        bool isFirst = true;

        while (getline(productionFile, line))
        {
            if (line.empty()) continue; // تجاهل السطور الفاضية

            stringstream ss(line);
            string nonTerminal, arrow, token;
            ss >> nonTerminal >> arrow;

            if (isFirst)
            {
                startSymbol = nonTerminal;
                isFirst = false;
            }

            vector<vector<string>> rules;
            vector<string> currentRule;

            while (ss >> token)
            {
                if (token == "|")
                {
                    rules.push_back(currentRule);
                    currentRule.clear();
                }
                else
                {
                    currentRule.push_back(token);
                }
            }
            if (!currentRule.empty())
                rules.push_back(currentRule);

            Productions[nonTerminal] = rules;
        }

        productionFile.close();
    }


    set<string> computeFirst(string symbol)
    {
        if (firstSets.count(symbol)) return firstSets[symbol];

        set<string> result;

        if (Productions.find(symbol) == Productions.end())
        {
            result.insert(symbol);
            return result;
        }

        for (auto &rule : Productions[symbol])
        {
            for (auto &token : rule)
            {
                set<string> firstToken = computeFirst(token);
                result.insert(firstToken.begin(), firstToken.end());
                if (firstToken.find("e") == firstToken.end()) break;
            }
        }
        return firstSets[symbol] = result;
    }


    set<string> computeFollow(string symbol, unordered_set<string> visited = {})
    {
        if (followSets.count(symbol)) return followSets[symbol];
        if (visited.count(symbol)) return {};
        visited.insert(symbol);

        set<string> result;


        if (symbol == startSymbol)
            result.insert("$");

        for (auto &prod : Productions)
        {
            for (auto &rule : prod.second)
            {
                for (size_t i = 0; i < rule.size(); i++)
                {
                    if (rule[i] == symbol)
                    {
                        if (i + 1 < rule.size())
                        {
                            set<string> firstNext = computeFirst(rule[i + 1]);
                            for (auto &x : firstNext)
                            {
                                if (x != "e")
                                    result.insert(x);
                            }

                            if (firstNext.count("e"))
                            {
                                set<string> followNext = computeFollow(prod.first, visited);
                                result.insert(followNext.begin(), followNext.end());
                            }
                        }
                        else
                        {
                            set<string> followNext = computeFollow(prod.first, visited);
                            result.insert(followNext.begin(), followNext.end());
                        }
                    }
                }
            }
        }

        visited.erase(symbol);
        followSets[symbol] = result;
        return result;
    }


    void buildTable()
    {
        for (const auto &prod : Productions)
        {
            const string &nonTerminal = prod.first;

            for (const auto &rule : prod.second)
            {
                set<string> firstSet;
                bool hasEpsilon = true;

                for (const auto &symbol : rule)
                {
                    set<string> firstOfSymbol;

                    if (firstSets.count(symbol))
                        firstOfSymbol = firstSets[symbol];
                    else
                        firstOfSymbol.insert(symbol); // terminal

                    firstSet.insert(firstOfSymbol.begin(), firstOfSymbol.end());

                    if (firstOfSymbol.find("e") == firstOfSymbol.end())
                    {
                        hasEpsilon = false;
                        break;
                    }
                }

                for (const auto &terminal : firstSet)
                {
                    if (terminal != "e")
                        table[ {nonTerminal, terminal}] = ruleToString(rule);
                }

                if (hasEpsilon)
                {
                    for (const auto &follow : followSets[nonTerminal])
                    {
                        table[ {nonTerminal, follow}] = "e";
                    }
                }
            }
        }

    }


    void parseInput(const vector<string>& tokens)
    {
        stack<string> st;
        st.push("$");
        st.push(startSymbol);

        vector<string> input = tokens;
        input.push_back("$");

        size_t i = 0;

        while (!st.empty())
        {
            string top = st.top();
            string current = input[i];

            if (top == current)
            {
                st.pop();
                i++;
            }
            else if (Productions.count(top)) // non-terminal
            {
                auto it = table.find({top, current});
                if (it != table.end())
                {
                    st.pop();
                    vector<string> rhs;
                    stringstream ss(it->second);
                    string symbol;
                    while (ss >> symbol)
                    {
                        if (symbol != "e")
                            rhs.push_back(symbol);
                    }
                    // Push RHS in reverse
                    for (auto it = rhs.rbegin(); it != rhs.rend(); ++it)
                        st.push(*it);
                }
                else
                {
                    cout << "Syntax error at token: " << current << " while top of stack is " << top << endl;
                    return;
                }
            }
            else // terminal mismatch
            {
                cout << "Unexpected token: " << current << ", expected: " << top << endl;
                return;
            }
        }

        if (i == input.size())
        {
            cout << "Input is syntactically correct\n";
            return;

        }
        else
        {
            cout << "Extra input after parsing finished\n";
            return;
        }


    }


    void computeAllFirstAndFollow()
    {
        for (const auto &entry : Productions)
            computeFirst(entry.first);

        for (const auto &entry : Productions)
            computeFollow(entry.first);
    }



    string ruleToString(const vector<string> &rule)
    {
        stringstream ss;
        for (const string &s : rule)
            ss << s << " ";
        return ss.str();
    }


    void printFirstAndFollow()
    {
        computeAllFirstAndFollow();
        cout << "First and Follow Sets:\n";
        for (const auto &entry : Productions)
        {
            const string &nonTerminal = entry.first;
            cout << nonTerminal << " -> FIRST: { ";
            for (const auto &value : firstSets[nonTerminal])
            {
                cout << value << " ";
            }
            cout << "} FOLLOW: { ";
            for (const auto &value : followSets[nonTerminal])
            {
                cout << value << " ";
            }
            cout << "}\n";
        }
        cout<<"__________________________________________________________\n\n";
    }


    void printProductions()
    {
        cout << "Productions:\n";
        for (const auto &entry : Productions)
        {
            cout << entry.first << " -> ";
            for (size_t i = 0; i < entry.second.size(); ++i)
            {
                for (const auto &symbol : entry.second[i])
                {
                    cout << symbol << " ";
                }
                if (i != entry.second.size() - 1) cout << "| ";
            }
            cout << "\n";
        }
        cout<<"__________________________________________________________\n\n";
    }


    void printParsingTable()
    {
        // تجميع كل الـ terminals من الجدول
        set<string> terminals;
        for (auto &entry : table)
        {
            terminals.insert(entry.first.second);
        }

        // طباعة العنوان
        cout << "Parsing Table:\n";
        cout << setw(12) << left << "NT\\T";

        for (const auto &t : terminals)
            cout << setw(12) << left << t;
        cout << "\n";

        cout << string(12 + 12 * terminals.size(), '-') << "\n";

        // طباعة الصفوف
        for (const auto &nonTerminal : Productions)
        {
            cout << setw(12) << left << nonTerminal.first;

            for (const auto &terminal : terminals)
            {
                auto key = make_pair(nonTerminal.first, terminal);
                if (table.count(key))
                    cout << setw(12) << left << table[key];
                else
                    cout << setw(12) << left << " ";
            }
            cout << "\n";
        }

        cout<<"__________________________________________________________\n\n";
    }


};

vector<string> readTokens(string path)
{
        ifstream inputFile(path);
        vector<string> Tokens;

        vector<map<string,string>> toke;
        string key , er , value;

        while(inputFile >> key >> er >> value)
            Tokens.push_back(key);


        return Tokens;
        inputFile.close();
}

int main()
{
    vector<string> tokens =  readTokens("Tokens.txt");

    Parser parser;

    parser.readProductions("grammar.txt");

    parser.printProductions();

    parser.computeAllFirstAndFollow();
    parser.printFirstAndFollow();

    parser.buildTable();
    parser.printParsingTable();

    parser.parseInput(tokens);
    return 0;
}
