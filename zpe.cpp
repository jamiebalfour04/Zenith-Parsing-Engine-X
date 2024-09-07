#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <regex>
#include <sstream>

using namespace std;

// Class to represent the Tokenizer (you'll need to implement it)
class Tokeniser {
public:
    vector<string> listOfComments() {
        return {};
    }

    vector<string> quoteTypes() {
        return {};
    }

    vector<string> listOfBoundWords() {
        return {};
    }

    vector<string> listOfSubsequentCharacters() {
        return {};
    }

    vector<string> listOfWhitespaces() {
        return { " ", "\t", "\n" };
    }

    char stringToByteCode(const string& s) {
        return s[0];  // Dummy implementation
    }
};

// ZenithParsingEngine class
class ZenithParsingEngine {
private:
    class PeekPosition {
    public:
        string w;
        char b;
        int p;

        PeekPosition(string w, char b, int p) : w(w), b(b), p(p) {}

        string toString() {
            return w + " " + to_string(b) + " " + to_string(p);
        }
    };

    static const string VERSION;
    static const char CR = 0x0D;
    static const char LF = 0x0A;
    static const string CRLF;

    Tokeniser* tokens;
    vector<string> word_list;
    vector<char> byte_list;
    vector<PeekPosition> peekPositionList;
    map<string, string> replaceables;
    string program;
    char current_symbol = -128;
    string current_word;
    int current_line = 0;
    char previous_symbol;
    int program_counter = 0;
    bool case_insensitive = false;
    string whitespaces = "";
    vector<string> comments;
    bool ignore_quotes = false;
    bool ignoreEscapes = false;
    string program_so_far;

    static char TAB;
    static char NL;
    static char CRR;
    static int ASCII_ZERO;
    static int ASCII_NINE;

public:
    ZenithParsingEngine(string program_code, bool caseInsensitive, Tokeniser* t) {
        TAB = '\t';
        NL = '\n';
        CRR = '\r';

        // Store the ASCII values of '0' and '9'
        ASCII_ZERO = (int)'0';
        ASCII_NINE = (int)'9';

        this->case_insensitive = caseInsensitive;
        this->tokens = t;

        if (!tokens->listOfComments().empty())
            this->comments = tokens->listOfComments();

        this->program = program_code;

        program_so_far = "";

        // Prepare the program string
        program = prepareString(program);
    }

    static string prepareString(string s) {
        // Prepare a program for actually being parsed
        stringstream out;
        stringstream scanner(s);

        string ln;
        while (getline(scanner, ln)) {
            out << ln << " " << endl;
        }

        return out.str();
    }

    static bool isNumeric(char c) {
        return (int)c >= ASCII_ZERO && (int)c <= ASCII_NINE;
    }

    static bool isNumeric(const string& s) {
        return regex_match(s, regex("(-)?[0-9]+(\\.[0-9]+)?(E(-?)[0-9]+)?"));
    }

    void ignoreQuotes(bool v) {
        this->ignore_quotes = v;
    }

    void skipSymbol(char s) {
        if (this->getCurrentSymbol() == s) {
            this->getNextSymbol();
        }
    }

    string getProgramTo(int point) {
        if (point > program.length()) {
            throw out_of_range("Index out of bounds");
        }
        return program.substr(0, point);
    }

    char stringToByte(const string& s) {
        return tokens->stringToByteCode(s);
    }

    void insertProgramAtCounter(const string& p) {
        string start = this->program.substr(0, program_counter);
        string end = this->program.substr(program_counter + 1);
        this->program = start + prepareString(p) + end;
    }

    string getProgram_so_far() {
        return program_so_far;
    }

    vector<char> getByteList() {
        return byte_list;
    }

    vector<string> getWordList() {
        return word_list;
    }

    string getProgram() {
        return program;
    }

    void setProgram(const string& p) {
        this->program = p;
    }

    int getProgramCounter() {
        return program_counter;
    }

    char getCurrentSymbol() {
        return current_symbol;
    }

    void setCurrentSymbol(char s) {
        this->current_symbol = s;
    }

    int getCurrentLine() {
        return current_line;
    }

    void addConstant(const string& n, const string& v) {
        replaceables[n] = v;
    }

    string getWhitespace() {
        return whitespaces;
    }

    char getNextChar() {
        program_counter++;
        return program[program_counter];
    }

    string parseTo(char symb) {
        string output;
        next(false);

        while (program_counter < program.length()) {
            if (current_symbol == symb) {
                return output;
            } else {
                output += whitespaces + current_word;
                next(false);
            }
        }

        return output;
    }

    string getCurrentWord() {
        return getCurrentWord(true);
    }

    void setCurrentWord(const string& s) {
        this->current_word = s;
    }

    string getCurrentWord(bool replaceQuotes) {
        string s = current_word;

        if (replaceQuotes) {
            if (!s.empty()) {
                char currChar = s[0];
                if (!tokens->quoteTypes().empty() && tokens->quoteTypes().find(string(1, currChar)) != string::npos) {
                    s = s.substr(1);
                    if (s.back() == currChar) {
                        s = s.substr(0, s.length() - 1);
                    }
                }
            }
        }

        if (replaceables.find(s) != replaceables.end()) {
            current_word = replaceables[s];
            s = current_word;
        }

        return s;
    }

    bool hasNext() {
        return current_symbol != -2;
    }

    char getNextSymbol() {
        current_symbol = next(false);
        word_list.push_back(current_word);
        byte_list.push_back(current_symbol);
        return current_symbol;
    }

    int getPreviousSymbol() {
        return previous_symbol;
    }

    bool checkProgramCounter() {
        return program_counter < program.length();
    }

    char peekAhead() {
        return peekAhead(1);
    }

    char peekAhead(int n) {
        int tempPc = program_counter;
        char curSymb = current_symbol;
        char curPrevSymb = previous_symbol;
        int cline = current_line;
        string curWord = current_word;
        string cWhitespace = whitespaces;

        for (int q = 0; q < n; ++q) {
            next(true);
        }

        program_counter = tempPc;
        current_symbol = curSymb;
        previous_symbol = curPrevSymb;
        current_word = curWord;
        current_line = cline;
        whitespaces = cWhitespace;

        return current_symbol;
    }

private:
    char next(bool peeking) {
        whitespaces = "";

        if (program_counter >= program.length())
            return -2;

        previous_symbol = current_symbol;
        string word = getNextWord(peeking);

        if (word.empty()) {
            return -2;
        }

        current_word = word;

        if (!peeking) {
            program_so_far += word;
        }

        current_symbol = case_insensitive ? tokens->stringToByteCode(word) : tokens->stringToByteCode(word);
        return current_symbol;
    }

    string getNextWord(bool peeking) {
        string word;

        whitespaces += parser_eat_whitespace();

        if (program_counter >= program.length()) {
            return "";
        }

        char ch = program[program_counter];

        if (ch == '\0') {
            return "";
        }

        while (program_counter < program.length()) {
            ch = program[program_counter];

            for (const auto& s : tokens->listOfSubsequentCharacters()) {
                if (program.substr(program_counter, s.length()) == s) {
                    if (!word.empty()) {
                        return word;
                    }
                    program_counter += s.length();
                    return s;
                }
            }

            if (program_counter + 1 < program.length()) {
                string double_char(1, ch);
                double_char += program[program_counter + 1];

                for (const auto& s : tokens->listOfSubsequentCharacters()) {
                    if (s.substr(0, 2) == double_char) {
                        if (s.length() > 2) {
                            string new_word = double_char;
                            int count = 2;

                            while (program_counter + count < program.length() && count < s.length()) {
                                new_word += program[program_counter + count];
                                ++count;
                            }

                            program_counter += new_word.length();
                            return new_word;
                        } else {
                            program_counter += 2;
                            return double_char;
                        }
                    }
                }
            }

            word += ch;
            ++program_counter;
        }

        return word;
    }
    string parser_eat_whitespace() {
        string output;
        bool terminate = false;

        while (program_counter < program.length() && !terminate) {
            string ws;

            for (const auto& w : tokens->listOfWhitespaces()) {
                if (program.substr(program_counter, w.length()) == w) {
                    ws = w;
                    break;
                }
            }

            if (!ws.empty()) {
                output += ws;
                program_counter += ws.length();
            } else {
                terminate = true;
            }
        }

        return output;
    }
};
