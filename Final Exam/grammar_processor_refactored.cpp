#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <sstream>
#include <algorithm>
#include <map>

using namespace std;

// Define a structure for a production rule (can be a nested struct in the class or outside)
struct Production {
    char non_terminal;
    vector<string> rhs_alternatives;
};

class GrammarProcessor {
public:
    // Constructor
    GrammarProcessor(const vector<string>& initial_rules_str, char start_sym) : start_symbol(start_sym) {
        for (const string& rule_str : initial_rules_str) {
            Production p = parse_production_rule(rule_str);
            if (p.non_terminal == 0) { // Parsing failed
                cerr << "Skipping invalid rule: " << rule_str << endl;
                continue;
            }
            // Aggregate rules for the same non-terminal
            if (grammar_map.count(p.non_terminal)) {
                grammar_map[p.non_terminal].rhs_alternatives.insert(
                    grammar_map[p.non_terminal].rhs_alternatives.end(),
                    p.rhs_alternatives.begin(),
                    p.rhs_alternatives.end()
                );
                // Sort and unique alternatives for consistency
                sort(grammar_map[p.non_terminal].rhs_alternatives.begin(), grammar_map[p.non_terminal].rhs_alternatives.end());
                grammar_map[p.non_terminal].rhs_alternatives.erase(
                    unique(grammar_map[p.non_terminal].rhs_alternatives.begin(), grammar_map[p.non_terminal].rhs_alternatives.end()),
                    grammar_map[p.non_terminal].rhs_alternatives.end());
            } else {
                grammar_map[p.non_terminal] = p;
            }
        }
    }

    // Main method to perform epsilon elimination
    map<char, Production> eliminate_epsilon_productions() {
        set<char> nullable_s = find_nullable_symbols_internal();
        cout << "Nullable symbols: ";
        for (char s_char : nullable_s) cout << s_char << " ";
        cout << endl;

        map<char, Production> new_productions_map;
        for (const auto& pair : grammar_map) {
            char A = pair.first;
            const Production& prod_obj = pair.second;
            set<string> resulting_rhs_set;

            for (const string& rhs_alternative : prod_obj.rhs_alternatives) {
                if (rhs_alternative == "#") { // Epsilon production
                    continue; // Skip original epsilon productions for now
                }
                
                vector<string> combinations = generate_rhs_combinations_internal(rhs_alternative, nullable_s);
                for (const string& comb : combinations) {
                    if (!comb.empty()) {
                        resulting_rhs_set.insert(comb);
                    } else { // An empty combination was generated
                        // Add epsilon only if A is the start symbol and A is nullable
                        // Or if the original language contained epsilon through A
                        if (A == start_symbol && nullable_s.count(A)) {
                             resulting_rhs_set.insert("#");
                        }
                    }
                }
            }
            if (!resulting_rhs_set.empty()) {
                Production new_p;
                new_p.non_terminal = A;
                new_p.rhs_alternatives.assign(resulting_rhs_set.begin(), resulting_rhs_set.end());
                sort(new_p.rhs_alternatives.begin(), new_p.rhs_alternatives.end());
                new_productions_map[A] = new_p;
            }
        }
        
        // If start symbol S is nullable, ensure S -> # is in the new grammar
        if (nullable_s.count(start_symbol)) {
            bool s_has_epsilon = false;
            if (new_productions_map.count(start_symbol)) {
                for(const auto& alt : new_productions_map[start_symbol].rhs_alternatives) {
                    if (alt == "#") {
                        s_has_epsilon = true;
                        break;
                    }
                }
            }
            if (!s_has_epsilon) {
                if (new_productions_map.find(start_symbol) == new_productions_map.end()) {
                     Production p_s; p_s.non_terminal = start_symbol;
                     new_productions_map[start_symbol] = p_s; 
                }
                new_productions_map[start_symbol].rhs_alternatives.push_back("#");
                sort(new_productions_map[start_symbol].rhs_alternatives.begin(), 
                          new_productions_map[start_symbol].rhs_alternatives.end());
                new_productions_map[start_symbol].rhs_alternatives.erase(
                    unique(new_productions_map[start_symbol].rhs_alternatives.begin(), 
                                new_productions_map[start_symbol].rhs_alternatives.end()),
                    new_productions_map[start_symbol].rhs_alternatives.end());
            }
        }

        // Final cleanup: remove productions that reference undefined non-terminals
        // This step might be needed if some non-terminals become unreachable or only derive empty
        map<char, Production> final_grammar = cleanup_grammar(new_productions_map, nullable_s);

        return final_grammar;
    }

    // Utility to print grammar
    static void print_grammar_static(const map<char, Production>& gram_map) {
        vector<char> sorted_nts;
        for(const auto& pair : gram_map) {
            sorted_nts.push_back(pair.first);
        }
        sort(sorted_nts.begin(), sorted_nts.end());

        for (char nt_char : sorted_nts) {
            if (gram_map.count(nt_char)) {
                const Production& p = gram_map.at(nt_char);
                if (p.rhs_alternatives.empty()) continue;

                cout << p.non_terminal << " -> ";
                for (size_t i = 0; i < p.rhs_alternatives.size(); ++i) {
                    cout << p.rhs_alternatives[i];
                    if (i < p.rhs_alternatives.size() - 1) {
                        cout << " | ";
                    }
                }
                cout << endl;
            }
        }
    }

    const map<char, Production>& get_original_grammar_map() const {
        return grammar_map;
    }

private:
    map<char, Production> grammar_map;
    char start_symbol;

    // Helper: Trim whitespace from left, right, or both ends of a string
    string ltrim(const string &s) {
        const string WHITESPACE = " \t\n\r\f\v";
        size_t start = s.find_first_not_of(WHITESPACE);
        return (start == string::npos) ? "" : s.substr(start);
    }

    string rtrim(const string &s) {
        const string WHITESPACE = " \t\n\r\f\v";
        size_t end = s.find_last_not_of(WHITESPACE);
        return (end == string::npos) ? "" : s.substr(0, end + 1);
    }

    string fulltrim(const string &s) {
        return rtrim(ltrim(s));
    }

    // Parse a single production rule string
    Production parse_production_rule(const string& rule_str) {
        Production p;
        p.non_terminal = 0; // Initialize to indicate potential failure
        string temp_rule_str = fulltrim(rule_str);
        size_t arrow_pos = temp_rule_str.find("->");

        if (arrow_pos == string::npos) {
            cerr << "Error: Invalid production rule format (missing '->'): " << rule_str << endl;
            return p;
        }

        string nt_str = fulltrim(temp_rule_str.substr(0, arrow_pos));
        if (!nt_str.empty() && nt_str.length() == 1 && isupper(nt_str[0])) {
            p.non_terminal = nt_str[0];
        } else {
            cerr << "Error: Invalid or missing non-terminal in rule: " << rule_str << endl;
            return p;
        }
        
        string rhs_full_str = fulltrim(temp_rule_str.substr(arrow_pos + 2));
        stringstream rhs_ss(rhs_full_str);
        string alternative;

        if (rhs_full_str.empty()) { 
            // Treat as A -> # if this is the intended meaning for an empty RHS by the grammar designer
            // For this implementation, we expect explicit '#' for epsilon.
            // p.rhs_alternatives.push_back("#"); // Or handle as error/warning
        } else {
            while (getline(rhs_ss, alternative, '|')) {
                string trimmed_alt = fulltrim(alternative);
                if (!trimmed_alt.empty()) {
                    p.rhs_alternatives.push_back(trimmed_alt);
                }
            }
        }
        // If after parsing, no alternatives were found but rhs_full_str was not empty (e.g. "S -> | | ")
        // or if it was truly empty (e.g. "S -> ")
        if (p.rhs_alternatives.empty() && rhs_full_str.empty()){
             // This could be an implicit epsilon, but we prefer explicit '#'
        }
        return p;
    }

    // Find nullable symbols
    set<char> find_nullable_symbols_internal() const {
        set<char> nullable_symbols_found;
        bool changed;
        do {
            changed = false;
            for (const auto& pair : grammar_map) {
                const Production& p_rule = pair.second;
                if (nullable_symbols_found.count(p_rule.non_terminal)) {
                    continue; // Already known to be nullable
                }
                for (const auto& alt : p_rule.rhs_alternatives) {
                    if (alt == "#") { // Epsilon production
                        if (nullable_symbols_found.insert(p_rule.non_terminal).second) {
                            changed = true;
                        }
                        break; // This non-terminal is nullable
                    }
                    bool all_in_alt_nullable = true;
                    if (alt.empty()) { // Should not happen if parser filters empty strings and '#' is used
                        all_in_alt_nullable = false;
                    } else {
                        for (char symbol_in_alt : alt) {
                            if (isupper(symbol_in_alt)) { // Non-terminal
                                if (!nullable_symbols_found.count(symbol_in_alt)) {
                                    all_in_alt_nullable = false;
                                    break;
                                }
                            } else { // Terminal
                                all_in_alt_nullable = false;
                                break;
                            }
                        }
                    }
                    if (all_in_alt_nullable) {
                        if (nullable_symbols_found.insert(p_rule.non_terminal).second) {
                            changed = true;
                        }
                        break; // This non-terminal is nullable through this alternative
                    }
                }
            }
        } while (changed);
        return nullable_symbols_found;
    }

    // Generate new productions by removing subsets of nullable symbols
    vector<string> generate_rhs_combinations_internal(const string& rhs, const set<char>& nullable_s) const {
        vector<string> combinations;
        int n = rhs.length();
        vector<int> nullable_indices_in_rhs; // Store indices of nullable non-terminals in rhs

        for (int i = 0; i < n; ++i) {
            if (isupper(rhs[i]) && nullable_s.count(rhs[i])) {
                nullable_indices_in_rhs.push_back(i);
            }
        }

        int num_nullable_in_rhs = nullable_indices_in_rhs.size();
        // Iterate through all subsets of these nullable non-terminals (2^num_nullable_in_rhs possibilities)
        for (int i = 0; i < (1 << num_nullable_in_rhs); ++i) { 
            string current_combination = "";
            set<int> indices_to_remove_from_rhs; // Indices in the original rhs string to be removed for this combination
            
            for (int j = 0; j < num_nullable_in_rhs; ++j) {
                // If the j-th bit is set in i, it means the j-th nullable symbol (from nullable_indices_in_rhs) should be removed
                if ((i >> j) & 1) { 
                    indices_to_remove_from_rhs.insert(nullable_indices_in_rhs[j]);
                }
            }

            // Construct the new RHS string by keeping symbols not marked for removal
            for (int k = 0; k < n; ++k) {
                if (indices_to_remove_from_rhs.find(k) == indices_to_remove_from_rhs.end()) {
                    current_combination += rhs[k];
                }
            }
            combinations.push_back(current_combination); 
        }
        return combinations;
    }

    map<char, Production> cleanup_grammar(const map<char, Production>& current_grammar, const set<char>& nullable_at_start) {
        map<char, Production> cleaned_grammar;
        set<char> defined_nts_in_current_map;
        for(const auto& pair : current_grammar) {
            defined_nts_in_current_map.insert(pair.first);
        }

        for(auto const& [nt_char, prod_obj_original] : current_grammar) {
            Production current_prod_cleaned;
            current_prod_cleaned.non_terminal = nt_char;
            set<string> unique_rhs_alts; // Use a set to handle duplicates from generation
            
            for (const string& rhs_alt : prod_obj_original.rhs_alternatives) {
                if (rhs_alt == "#") {
                    unique_rhs_alts.insert("#");
                    continue;
                }
                bool rhs_is_valid = true;
                for (char symbol_in_rhs : rhs_alt) {
                    if (isupper(symbol_in_rhs)) { // If it's a non-terminal
                        // Check if this non-terminal is defined in the current set of productions
                        // This helps remove rules that refer to non-terminals that might have been entirely eliminated
                        if (defined_nts_in_current_map.find(symbol_in_rhs) == defined_nts_in_current_map.end()) {
                            rhs_is_valid = false;
                            break;
                        }
                    }
                }
                if (rhs_is_valid && !rhs_alt.empty()) { // Also ensure non-empty unless it's explicitly epsilon
                    unique_rhs_alts.insert(rhs_alt);
                }
            }

            if (!unique_rhs_alts.empty()) {
                current_prod_cleaned.rhs_alternatives.assign(unique_rhs_alts.begin(), unique_rhs_alts.end());
                sort(current_prod_cleaned.rhs_alternatives.begin(), current_prod_cleaned.rhs_alternatives.end());
                cleaned_grammar[nt_char] = current_prod_cleaned;
            } else if (nt_char == start_symbol && nullable_at_start.count(start_symbol)) {
                // If it's the start symbol, was nullable, and all its rules vanished, it should at least have S -> #
                Production s_prod; s_prod.non_terminal = start_symbol;
                s_prod.rhs_alternatives.push_back("#");
                cleaned_grammar[start_symbol] = s_prod;
            }
        }
        // Ensure start symbol has an epsilon if it was nullable and all other rules were removed or it wasn't there
         if (nullable_at_start.count(start_symbol)) {
            bool s_in_final = cleaned_grammar.count(start_symbol);
            bool s_has_epsilon_in_final = false;
            if(s_in_final){
                for(const auto& alt : cleaned_grammar[start_symbol].rhs_alternatives) if(alt=="#") s_has_epsilon_in_final = true;
            }

            if (!s_in_final || !s_has_epsilon_in_final) {
                if (!s_in_final) {
                    Production s_prod; s_prod.non_terminal = start_symbol;
                    cleaned_grammar[start_symbol] = s_prod;
                }
                if (!s_has_epsilon_in_final){
                    cleaned_grammar[start_symbol].rhs_alternatives.push_back("#");
                    sort(cleaned_grammar[start_symbol].rhs_alternatives.begin(), cleaned_grammar[start_symbol].rhs_alternatives.end());
                    cleaned_grammar[start_symbol].rhs_alternatives.erase(
                        unique(cleaned_grammar[start_symbol].rhs_alternatives.begin(), cleaned_grammar[start_symbol].rhs_alternatives.end()),
                        cleaned_grammar[start_symbol].rhs_alternatives.end());
                }
            }
        }
        return cleaned_grammar;
    }
};

int main() {
    vector<string> grammar_rules_str = {
        "S -> ABC | D",
        "A -> aA | #",
        "B -> bB | #",
        "C -> c",
        "D -> #"
    };
    char start_symbol = 'S';

    GrammarProcessor processor(grammar_rules_str, start_symbol);

    cout << "Original Grammar:" << endl;
    GrammarProcessor::print_grammar_static(processor.get_original_grammar_map());
    cout << endl;

    map<char, Production> result_grammar = processor.eliminate_epsilon_productions();

    cout << "Grammar after Epsilon Elimination (omega-free):" << endl;
    GrammarProcessor::print_grammar_static(result_grammar);

    return 0;
}

