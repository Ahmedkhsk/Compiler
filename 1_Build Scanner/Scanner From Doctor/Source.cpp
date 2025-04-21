#include<iostream>
#include<fstream>
#include<string>
#include<sstream>
#include<map>
#include<vector>
#include<set>
using namespace std;

class DFA
{
	private:
	map<pair<string, char>, string> TransitionTable;
	set<string> FinalStates;
	set<string> States;
	public :
	string StartState;
	string ErrorState;
	string CurrentState;
	string PrevoiusState;

	void load_States(string filePath)
	{
		ifstream file(filePath);
		if (!file) {
			cerr << "Error opening states file!\n";
			return;
		}
		string line;
		// Read all states from first line, states separated with space as it is easy so split them with
		if (getline(file, line)) {
			stringstream ss(line);
			string state;
			while (ss >> state) {
				States.insert(state);
			}
		}

		// Read start state from second line
		if (getline(file, StartState)) {
			CurrentState = StartState;
		}
		// Read final states from third line
		if (getline(file, line)) {
			stringstream ss(line);
			string state;
			while (ss >> state) {
				FinalStates.insert(state);
			}
		}

		// Read error state from fourth line
		if (getline(file, ErrorState)) 
		{		
		}


	}

	void loadTransitionTable(string filePath) {
		ifstream file(filePath);
		if (!file) {
			cerr << "Error opening transitions file!\n";
			return;
		}

		string currentState, nextState;
		char input;

		while (file >> currentState >> input >> nextState) {
			TransitionTable[{currentState, input}] = nextState;
		}
		file.close();
	}

	string MoveToNextState(string currentState, char input)
	{
		pair<string, char> key = { currentState,input };
		auto keyValue = TransitionTable.find(key);
		if (keyValue != TransitionTable.end())
		{
			PrevoiusState = CurrentState;
			CurrentState = keyValue->second;
			//return the value of the returned iterator
			return keyValue->second;
		}
		else
		{
			PrevoiusState = CurrentState;
			CurrentState = ErrorState;
			return ErrorState;
		}
	}
	bool isFinalState(string state)
	{
		// if not found, find function return the iterator end of the set
		return FinalStates.find(state) != FinalStates.end();
	}

	void reset()
	{
		CurrentState = StartState;
		PrevoiusState = StartState;
	}

};







class Scanner
{
	DFA dfa;
	map<string, string> FinalStateToToken;
	public:
		Scanner(string statesFile, string TransitionTableFile, string tokenFile)
		{
			load_FinalStatesTokensCorrespondance(tokenFile);
			dfa.load_States(statesFile);
			dfa.loadTransitionTable(TransitionTableFile);
		}
	void load_FinalStatesTokensCorrespondance(string filePath)
	{
		ifstream file(filePath);
		if (!file)
			cerr << "Error openning file" << endl;
		string line, state, token;

		while (getline(file,line))
		{
			stringstream ss(line);
			ss >> state >> token;
			FinalStateToToken[state] = token;
		}
	}
	void scan(string scanningFile, string outputPath)
	{
		
		ifstream file(scanningFile);
		ofstream outPutFile(outputPath);
		if (!file)
			cerr << "Error openning file" << endl;

		char ch;
		string CurrentState = dfa.StartState;
		string PreviousState = dfa.StartState;
		string lexeme = "";
		while (file.get(ch))
		{
			PreviousState = CurrentState;
			CurrentState = dfa.MoveToNextState(CurrentState, ch);
			if(CurrentState != dfa.ErrorState)
				lexeme += ch;
			else //(CurrentState == dfa.ErrorState)
			{
				if (dfa.isFinalState(PreviousState))
				{
					outPutFile << FinalStateToToken[PreviousState] << "\t\t" << lexeme << endl;
					CurrentState = dfa.StartState;
					PreviousState = dfa.StartState;
					lexeme = "";
					//reset the character to be the same, so in the next loop ch is the same character 
					//so it could start with this character input with the start state
					file.unget();
				}
				else
				{
					outPutFile << "Lexical Error :" <<"\t\t"<< ch <<" "<<"Invalid Character" << endl;					
					outPutFile.close();
					exit(1);
				}				
			}

		}
		//scan The last character, because above will break from while loop without scanning the last character if it is valid
		if (dfa.isFinalState(CurrentState))
		{
			// Output the final valid token
			outPutFile << FinalStateToToken[CurrentState] << "\t\t" << lexeme << endl;
		}				
	}
};


int main()
{
	Scanner scanner("automaton.txt", "transition.txt", "tokens.txt");
	scanner.scan("input.txt", "output.txt");
	return 0;
}