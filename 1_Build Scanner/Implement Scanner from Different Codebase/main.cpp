#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <sstream>
using namespace std;

struct Automaton {
    int startState;
    vector<int> finalStates;
    map<pair<int, char>, int> transitionFunction;
    map<int, string> tokenTypes;
};

vector<char> alphabets;
vector<string> ReversedWords;
Automaton automaton;

bool isSpecialChar(char c) {
    return c == '(' || c == ')' || c == ',';
}

int getState(int state, char c) {
    auto it = automaton.transitionFunction.find({ state, c });
    return (it != automaton.transitionFunction.end()) ? it->second : -1;
}

bool isFinal(int state) {
    return find(automaton.finalStates.begin(), automaton.finalStates.end(), state) != automaton.finalStates.end();
}

bool isExactMatch(const string& word) {
    return find(ReversedWords.begin(), ReversedWords.end(), word) != ReversedWords.end();
}

string getTokenType(const string& lexeme, int state) {
    if (lexeme == "(") return "Open";
    if (lexeme == ")") return "Close";
    if (lexeme == ",") return "Comma";

    if (!isFinal(state)) return "Invalid_Token";

    if (isExactMatch(lexeme)) return "Color";
    if (state == 4) return "Invalid_Token";

    auto it = automaton.tokenTypes.find(state);
    return (it != automaton.tokenTypes.end()) ? it->second : "Invalid_Token";
}

void loadAlphabet() {
    ifstream input("Alphabet.txt");
    char c;
    while (input >> c) alphabets.push_back(c);
}

void loadAutomaton() {
    ifstream automatonFile("Automaton.txt");
    int numStates, numFinal;
    automatonFile >> numStates >> automaton.startState >> numFinal;

    automaton.finalStates.resize(numFinal);
    for (int& state : automaton.finalStates)
        automatonFile >> state;

    ifstream transitions("Transition.txt");
    int from, to;
    char c;
    while (transitions >> from >> c >> to)
        automaton.transitionFunction[{from, c}] = to;

    ifstream tokenTypes("TokenTypes.txt");
    string line;
    int index = 0;
    while (getline(tokenTypes, line) && index < automaton.finalStates.size())
        automaton.tokenTypes[automaton.finalStates[index++]] = line;
}

void loadReversedWords() {
    ifstream input("ReversedWords.txt");
    string line;
    while (getline(input, line)) ReversedWords.push_back(line);
}

void scan(const vector<char>& input, ofstream& out) {
    int state = automaton.startState;
    string lexeme;
    size_t i = 0;
    while (i <= input.size()) {
        char c = (i < input.size()) ? input[i] : ' ';

        if ((isSpecialChar(c) || i == input.size()) && !lexeme.empty()) {
            out << lexeme << " - " << getTokenType(lexeme, state) << endl;
            lexeme.clear();
            state = automaton.startState;
        }

        if (i < input.size() && isSpecialChar(c)) {
            out << c << " - " << getTokenType(string(1, c), -1) << endl;
            i++;
            continue;
        }

        if (i < input.size()) {
            int nextState = getState(state, c);

            if (nextState != -1) {
                lexeme += c;
                state = nextState;
            }
            else {
                if (!lexeme.empty()) {
                    out << lexeme << " - " << getTokenType(lexeme, state) << endl;
                    lexeme.clear();
                    state = automaton.startState;
                    continue;
                }

                out << c << " - Invalid_Token" << endl;
                lexeme.clear();
                state = automaton.startState;
            }
        }
        i++;
    }
}

int main() {
    loadAlphabet();
    loadAutomaton();
    loadReversedWords();

    string line;
    cout << "Enter Input: ";
    getline(cin, line);

    ofstream out("Tokens.txt");
    scan(vector<char>(line.begin(), line.end()), out);
    cout << "Tokenization complete. Check Tokens.txt." << endl;
    return 0;
}