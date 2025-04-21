#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <unordered_map>

using namespace std;

vector<string> tokens;

class DFA
{
    string input;
    vector<string> states;
    string startState;
    vector<string> finalStates;
    unordered_map<string, unordered_map<char, string>> transitionTable;
    char separator = ',';

public:

    void readInput(string path)
    {
        ifstream inputFile(path);
        if (!inputFile) {
            cerr << "Error opening file: " << path << endl;
            exit(1);
        }
        getline(inputFile, input);
    }

    vector<string> split(string str, char separator)
    {
        vector<string> tokens;
        stringstream ss(str);
        string token;
        while (getline(ss, token, separator))
        {
            tokens.push_back(token);
        }
        return tokens;
    }

    void readStates(string path)
    {
        ifstream statesFile(path);
        if (!statesFile) {
            cerr << "Error opening file: " << path << endl;
            exit(1);
        }

        string line;
        getline(statesFile, line);
        states = split(line, separator);

        getline(statesFile, startState);

        getline(statesFile, line);
        finalStates = split(line, separator);
    }

    void readTransitionTable(string path)
    {
        ifstream transitionFile(path);
        if (!transitionFile) {
            cerr << "Error opening file: " << path << endl;
            exit(1);
        }

        string currentState, nextState;
        char acceptedChar;
        while (transitionFile >> currentState >> acceptedChar >> nextState)
        {
            transitionTable[currentState][acceptedChar] = nextState;
        }
    }

    void Scan(string path)
    {
        ofstream tokensFile(path);
        if (!tokensFile) {
            cerr << "Error opening file: " << path << endl;
            exit(1);
        }

        string currentState = startState;
        string value = "";

        for (int i = 0; i < input.size(); i++)
        {
            char c = input[i];
            if (transitionTable[currentState].count(c))
            {
                currentState = transitionTable[currentState][c];
                value += c;
            }
            else
            {
                if (checkCurrentState(currentState))
                {
                    string token = getCorrespondingToken(value);
                    tokens.push_back(token);
                    tokensFile << value + " => " + token << endl;
                    currentState = startState;
                    value.clear();
                    i--;
                    continue;
                }
                else
                {
                    tokens.push_back("InvalidToken");
                    tokensFile << value + " => InvalidToken" << endl;
                    currentState = startState;
                    value.clear();
                    continue;
                }
            }
        }

        if (checkCurrentState(currentState))
        {
            string token = getCorrespondingToken(value);
            tokens.push_back(token);
            tokensFile << value + " => " + token << endl;
        }
        else if (!value.empty())
        {
            tokens.push_back("InvalidToken");
            tokensFile << value + " => InvalidToken" << endl;
        }
    }

    bool checkCurrentState(string state)
    {
        return find(finalStates.begin(), finalStates.end(), state) != finalStates.end();
    }

    string getCorrespondingToken(string value)
    {
        if (value == "(") return "Open Parenthesis";
        if (value == ")") return "Close Parenthesis";
        if (value == ",") return "Comma";

        if (all_of(value.begin(), value.end(), ::isdigit))
            return "Number";
        return "Color";
    }
};

int main()
{
        DFA dfa;
    dfa.readInput("input.txt");
    dfa.readStates("states.txt");
    dfa.readTransitionTable("transition function.txt");
    dfa.Scan("token.txt");

    if (tokens.empty()) {
        cout << "No tokens generated. Exiting..." << endl;
        return 1;
    }
    return 0;
}
