#include "stdio.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <regex>
#include <bitset>
#include "../misc.hpp"

using std::string;
using namespace TYPES;
using namespace BITMAN;
using namespace MISC;

enum Tokens {
    Number,
    Equal,
    Coma,
    _OPER,
    Plus,
    And,
    Or,
    _SOPER,
    Minus,
    Tilde,
    _REG,
    A,
    D,
    AM,
    _JMP,
    JEQ,
    JNE,
    JGT,
    JGE,
    JLT,
    JLE,
    JMP,
    __END__
};

struct KVP {
    string key;
    unsigned int value;
};

struct DS {
    string key;
    string value;
};

struct Macro {
    string name;
    std::vector<std::string> instructions;
    bool shouldBeExpanded;
    std::vector<DS> arguments;
};

std::vector<std::string> bits;
unsigned int linen = 1;
string currfile;
bool useDec = false;
std::vector<KVP> labels;
std::vector<KVP> constants;
std::vector<Macro> macros;

template <typename T>
bool isBetween(T min, T x, T max) {
    return (min < x && x < max);
}

template <typename T>
bool vecContains(std::vector<T> v, T x) {
    return (std::find(v.begin(), v.end(), x) != v.end());
}

std::string getSubStrBAChar(std::string const& str, const char chr, bool before) {
    std::string::size_type pos = str.find(chr);
    if (pos != std::string::npos) {
        if (before) {
            return str.substr(0, pos);
        } else {
            return str.substr(pos + 1);
        }
    }
    return str;
}

std::vector<string> cutString(string str, char delim) {
    std::stringstream ss(str);
    std::string segment;
    std::vector<std::string> seglist;

    while(std::getline(ss, segment, delim)) {
        seglist.push_back(segment);
    }
    return seglist;
}

bool replace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

bool labelExists(string label) {
    for (SU i = 0; i < labels.size(); i++) {
        if (labels[i].key == label)
            return true;
    }
    return false;
}

unsigned int getLabel(string label) {
    for (SU i = 0; i < labels.size(); i++) {
        if (labels[i].key == label)
            return labels[i].value;
    }
    return 0;
}

bool constantExists(string constant) {
    for (SU i = 0; i < constants.size(); i++) {
        if (constants[i].key == constant)
            return true;
    }
    return false;
}

unsigned int getConstant(string constant) {
    for (SU i = 0; i < constants.size(); i++) {
        if (constants[i].key == constant)
            return constants[i].value;
    }
    return 0;
}

string getRegex(string pattern, string str) {
    std::regex rx(pattern);
    std::smatch sm;
    if (std::regex_search(str, sm, rx)) {
        if (sm.position() == 0)
            return str.substr(0, sm.length()).c_str();
    }
    return "";
}

int getHBDNum(string str) {
    string ret = getRegex("(0b)[0-1]+", str);
    if (ret != "")
        return stoi(ret.substr(2), (size_t *)nullptr, 2);
    
    ret = getRegex("(0x)[0-9a-fA-F]+", str);
    if (ret != "")
        return stoi(ret.substr(2), (size_t *)nullptr, 16);
    
    ret = getRegex("-?[0-9]+", str);
    if (ret != "")
        return atoi(ret.c_str());
    
    return 0;
}

bool stringContains(string container, string containee) {
    return (container.find(containee) != std::string::npos);
}

bool vectorContains(std::vector<string> vec, string containee) {
    for (unsigned int i = 0; i < vec.size(); i++) {
        if (stringContains(vec[i], containee))
            return true;
    }
    return false;
}

bool isNumber(char c) {
    string str(1, c);
    string ret = getRegex("[0-9]+", str);
    if (ret != "")
        return true;
    return false;
}

void errExit(string message, string line, int code = 1) {
    printf("%s on [%s](expanding includes), (%s). Check your Syntax.", message.c_str(), line.c_str(), currfile.c_str());
    exit(code);
}

bool isLogOp(Tokens token) {
    return (token == Tokens::Or || token == Tokens::And || token == Tokens::Tilde);
}

class Tokenizer {
    private:
        string work;
        std::vector<Tokens> res;
        SU iter = 0;
        short num = 2;
    private:
        void ignore(char c) {
            work.erase(std::remove(work.begin(), work.end(), c), work.end());
        }
        string matchRegex(string pattern, Tokens type) {
            std::regex rx(pattern);
            std::smatch sm;
            if (std::regex_search(work, sm, rx)) {
                if (sm.position() == 0) {
                    res.push_back(type);
                    string ret = work.substr(0, sm.length()).c_str();
                    work.erase(0, sm.length());
                    iter = 0;
                    return ret;
                }
            }
            return "";
        }
        void matchExact(char c, Tokens type) {
            if (work[0] == c) {
                res.push_back(type);
                work.erase(0, 1);
                iter = 0;
            }
        }
        void matchExact(string c, Tokens type) {
            for (SU i = 0; i < c.size(); i++) {
                if (work[i] != c[i])
                    return;
            }
            res.push_back(type);
            work.erase(0, c.size());
            iter = 0;
        }
        void matchExact(char c, Tokens type, bool checknum) {
            if (checknum) {
                string ret = matchRegex("-?[0-9]+", Tokens::Number);
                if (ret != "")
                    num = atoi(ret.c_str());
            }
            matchExact(c, type);
        }
        void matchLabelConstant() {
            std::string constlabel = getSubStrBAChar(work, ';', true);
            if (labelExists(constlabel)) {
                num = getLabel(constlabel);
                res.push_back(Tokens::Number);
                work.erase(0, constlabel.length());
                iter = 0;
            } else if (constantExists(constlabel)) {
                num = getConstant(constlabel);
                res.push_back(Tokens::Number);
                work.erase(0, constlabel.length());
                iter = 0;
            }
        }
        void matchNumber() {
            string ret = matchRegex("(0b)[0-1]+", Tokens::Number);
            if (ret != "") {
                num = stoi(ret.substr(2), (size_t *)nullptr, 2);
            } else {
                ret = matchRegex("(0x)[0-9a-fA-F]+", Tokens::Number);
                if (ret != "") {
                    num = stoi(ret.substr(2), (size_t *)nullptr, 16);
                } else {
                    ret = matchRegex("[0-9]+", Tokens::Number);
                    if (ret != "")
                        num = atoi(ret.c_str());
                }
            }
        }
    public:
        std::vector<Tokens> tokenize(string str) {
            res.clear();
            work = str;
            ignore(' ');
            ignore('\t');
            ignore('_');
            while (work.length() != 0 && iter <= 11) {
                matchNumber();
                matchExact('+', Tokens::Plus);
                matchExact('-', Tokens::Minus, true);
                matchExact('&', Tokens::And);
                matchExact('|', Tokens::Or);
                matchExact('=', Tokens::Equal);
                matchExact('~', Tokens::Tilde);
                matchExact(',', Tokens::Coma);
                matchExact("*A", Tokens::AM);
                matchExact('A', Tokens::A);
                matchExact('D', Tokens::D);
                matchExact(";JEQ", Tokens::JEQ);
                matchExact(";JNE", Tokens::JNE);
                matchExact(";JGT", Tokens::JGT);
                matchExact(";JGE", Tokens::JGE);
                matchExact(";JLT", Tokens::JLT);
                matchExact(";JLE", Tokens::JLE);
                matchRegex(";?JMP", Tokens::JMP);
                matchLabelConstant();
                iter++;
            }
            if (iter >= 11) {
                errExit("Too many iterations on tokenization", str);
            }
            return res;
        }

        std::vector<Tokens> simplify() {
            std::vector<Tokens> simpTokens;
            for (int i = 0; i < res.size(); i++) {
                if (isBetween(Tokens::_OPER, res[i], Tokens::_SOPER)) {
                    simpTokens.push_back(Tokens::_OPER);
                } else if (isBetween(Tokens::_SOPER, res[i], Tokens::_REG)) {
                    simpTokens.push_back(Tokens::_SOPER);
                } else if (isBetween(Tokens::_REG, res[i], Tokens::_JMP)) {
                    simpTokens.push_back(Tokens::_REG);
                } else if (isBetween(Tokens::_JMP, res[i], Tokens::__END__)) {
                    simpTokens.push_back(Tokens::_JMP);
                } else {
                    simpTokens.push_back(res[i]);
                }
            }
            return simpTokens;
        }

        short getNum() {
            return num;
        }
};

class GrammarChecker {
    private:
        std::vector<Tokens> work;
    private:
        bool matchExact(std::vector<Tokens> c) {
            for (SU i = 0; i < c.size(); i++) {
                if (work[i] != c[i])
                    return false;
            }
            return true;
        }
    public:
        bool check(std::vector<Tokens> tokens, std::vector<Tokens> stokens, short num) {
            work = stokens;
            //Exception: A = Number
            if (tokens.size() == 3 && tokens[0] == Tokens::A && tokens[1] == Tokens::Equal && tokens[2] == Tokens::Number && isBetween((short)-2, num, (short)32765))
                return true;

            if (!vecContains(work, Tokens::Equal)) {
                //Operations without Equal
                if (work.size() == 1 && (work[0] == Tokens::_REG || tokens[0] == Tokens::JMP)) {
                    return true;
                } else if (work.size() == 2 && (matchExact({Tokens::_SOPER, Tokens::_REG}) || matchExact({Tokens::_REG, Tokens::_JMP}) || (work[0] == Tokens::_REG && work[1] == Tokens::Number && num == -1))) {
                    return true;
                } else if (work.size() == 3 && (matchExact({Tokens::_REG, Tokens::_OPER, Tokens::_REG}) || (work[0] == Tokens::_REG && tokens[1] == Tokens::Minus && work[2] == Tokens::_REG) || matchExact({Tokens::_SOPER, Tokens::_REG, Tokens::_JMP}) || (work[0] == Tokens::_REG && tokens[tokens.size()-2] == Tokens::Plus && work[2] == Tokens::Number && num == 1) || (work[0] == Tokens::_REG && work[1] == Tokens::Number && num == -1 && work[2] == Tokens::_JMP))) {
                    return true;
                } else if (work.size() == 4 && (matchExact({Tokens::_REG, Tokens::_OPER, Tokens::_REG, Tokens::_JMP}) || (work[0] == Tokens::_REG && tokens[1] == Tokens::Minus && work[2] == Tokens::_REG && work[3] == Tokens::_JMP) || (work[0] == Tokens::_REG && tokens[tokens.size()-3] == Tokens::Plus && work[2] == Tokens::Number && num == 1 && work[3] == Tokens::_JMP))) {
                    return true;
                }
            } else {
                //Equal Operations
                //Let's simplify
                for (SU i = 0; i <= 1; i++) {
                    if (work[0] == Tokens::_REG && work[1] == Tokens::Coma && work[2] == Tokens::_REG)
                        work.erase(std::next(work.begin(), 1), std::next(work.begin(), 3));
                }
                if (!(work[0] == Tokens::_REG && work[1] == Tokens::Equal))
                    return false;

                work.erase(std::next(work.begin(), 0), std::next(work.begin(), 2));
                if (work.size() == 1 && ((work[0] == Tokens::_REG) || ((work[0] == Tokens::Number) && (isBetween((short)-2, num, (short)2))))) {
                    return true;
                } else if (work.size() == 2 && (matchExact({Tokens::_SOPER, Tokens::_REG}) || matchExact({Tokens::_REG, Tokens::_JMP}) || (matchExact({Tokens::Number, Tokens::_JMP}) && (isBetween((short)-2, num, (short)2))) || (work[0] == Tokens::_REG && work[1] == Tokens::Number && num == -1))) {
                    return true;
                } else if (work.size() == 3 && (matchExact({Tokens::_REG, Tokens::_OPER, Tokens::_REG}) || (work[0] == Tokens::_REG && tokens[tokens.size()-2] == Tokens::Minus && work[2] == Tokens::_REG) || matchExact({Tokens::_SOPER, Tokens::_REG, Tokens::_JMP})  || (work[0] == Tokens::_REG && tokens[tokens.size()-2] == Tokens::Plus && work[2] == Tokens::Number && num == 1) || (work[0] == Tokens::_REG && work[1] == Tokens::Number && num == -1 && work[2] == Tokens::_JMP))) {
                    return true;
                } else if (work.size() == 4 && (matchExact({Tokens::_REG, Tokens::_OPER, Tokens::_REG, Tokens::_JMP}), (work[0] == Tokens::_REG && tokens[tokens.size()-3] == Tokens::Minus && work[2] == Tokens::_REG && work[3] == Tokens::_JMP) || (work[0] == Tokens::_REG && tokens[tokens.size()-3] == Tokens::Plus && work[2] == Tokens::Number && num == 1 && work[3] == Tokens::_JMP))) {
                    return true;
                }
            }
            return false;
        }
};

class CodeGenerator {
    private:
        BitArr<16> work = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        std::vector<Tokens> twork;
        std::vector<Tokens> stwork;
    private:
        void pullDest(Tokens token) {
            if (token == Tokens::A) {
                work.b[InstructionMap::A] = 1;
            } else if (token == Tokens::AM) {
                work.b[InstructionMap::AM] = 1;
            } else {
                work.b[InstructionMap::D] = 1;
            }
        }
        void pullJump(Tokens token) {
            if (token == Tokens::JEQ) {
                work.b[EQ] = 1;
            } else if (token == Tokens::JNE) {
                work.b[GT] = 1;
                work.b[LT] = 1;
            } else if (token == Tokens::JGT) {
                work.b[GT] = 1;
            } else if (token == Tokens::JGE) {
                work.b[GT] = 1;
                work.b[EQ] = 1;
            } else if (token == Tokens::JLT) {
                work.b[LT] = 1;
            } else if (token == Tokens::JLE) {
                work.b[LT] = 1;
                work.b[EQ] = 1;
            } else {
                work.b[GT] = 1;
                work.b[EQ] = 1;
                work.b[LT] = 1;
            }
        }
        bool matchExact(std::vector<Tokens> c) {
            for (SU i = 0; i < c.size(); i++) {
                if (twork[i] != c[i])
                    return false;
            }
            return true;
        }
        string decToBin(short dec) {
            return std::bitset<16>(dec).to_string();
        }
    public:
        void generate(std::vector<Tokens> tokens, std::vector<Tokens> stokens, short num) {
            twork = tokens;
            stwork = stokens;
            //Exception: A = Number
            if (tokens.size() == 3 && tokens[0] == Tokens::A && tokens[1] == Tokens::Equal && tokens[2] == Tokens::Number) {
                if (useDec) {
                    bits.push_back(std::to_string(num));
                } else {
                    bits.push_back(decToBin(num));
                }
                return;
            }
            work.b[CI] = 1; //Set Operation Bit
            if (vecContains(stwork, Tokens::Equal)) {
                for (SU i = 0; i <= 2; i++) {
                    if (stwork[0] == Tokens::_REG) {
                        if (stwork[1] == Tokens::Coma && stwork[2] == Tokens::_REG) {
                            pullDest(twork[0]);
                            twork.erase(std::next(twork.begin(), 0), std::next(twork.begin(), 2));
                            stwork.erase(std::next(stwork.begin(), 0), std::next(stwork.begin(), 2));
                        } else if(stwork[1] == Tokens::Equal) {
                            pullDest(twork[0]);
                            twork.erase(std::next(twork.begin(), 0), std::next(twork.begin(), 2));
                            stwork.erase(std::next(stwork.begin(), 0), std::next(stwork.begin(), 2));
                            break;
                        }
                    }
                }
            }
            if (vecContains(twork, Tokens::A) && vecContains(twork, Tokens::AM)) {
                errExit("Can't use A and A* as operators", " ~line (without special lines)" + linen);
            }
            if (vecContains(twork, Tokens::AM)) {
                work.b[SM] = 1;
                std::replace(twork.begin(), twork.end(), Tokens::AM, Tokens::A);
            }
            if (stwork.back() == Tokens::_JMP) {
                pullJump(twork.back());
                twork.pop_back();
                stwork.pop_back();
            }

            //TODO: implement operations (better oper generation)

            if (num == 1) {
                work.b[OP0] = 1;
            }

            if (twork.size() == 1) {
                work.b[ZX] = 1;
                if (!num == 0) {
                    work.b[U] = 1;
                    if (twork[0] == Tokens::D) {
                        work.b[SW] = 1;
                    } else if (num == -1) {
                        work.b[OP0] = 1;
                        work.b[OP1] = 1;
                    }
                }
            } else if (twork.size() == 2) {
                work.b[OP1] = 1;
                if (!isLogOp(twork[0])) {
                    work.b[OP0] = 1;
                } else {
                    work.b[U] = 1;
                    work.b[ZX] = 1;
                }
                if (matchExact({Tokens::Minus, Tokens::D}) || matchExact({Tokens::Tilde, Tokens::A})) {
                    work.b[SW] = 1;
                }
                if (num == -1) {
                    work.b[U] = 1;
                    if (twork[0] == Tokens::A) {
                        work.b[SW] = 1;
                    }
                }
            } else if (twork.size() == 3) {
                if (twork[0] == Tokens::D && stwork[1] == Tokens::_OPER && twork[2] == Tokens::A && twork[1] != Tokens::Minus) {
                    twork[0] = Tokens::A;
                    twork[2] = Tokens::D;
                }
                if (!isLogOp(twork[1])) {
                    work.b[U] = 1;
                } else if (twork[1] == Tokens::Or) {
                    work.b[OP0] = 1;
                }
                if (twork[1] == Tokens::Minus) {
                    work.b[OP1] = 1;
                }
                if (twork[0] == Tokens::A) {
                    work.b[SW] = 1;
                }
            }
            
            if (useDec) {
                bits.push_back(std::to_string(composeWord(work)));
            } else {
                bits.push_back(decToBin(composeWord(work)));
            }
        }
};

void parseLine(std::string line) {
    for (unsigned int i = 0; i < macros.size(); i++) {
        if (line == macros[i].name) {
            bits.insert(std::end(bits), std::begin(macros[i].instructions), std::end(macros[i].instructions));
            return;
        }
    }
    if (line.substr(0, 3) == "\\ci") {
        bits.push_back(line.substr(3));
        return;
    }
    Tokenizer tokenizer;
    std::vector<Tokens> tokens = tokenizer.tokenize(line);
    GrammarChecker grammarchecker;
    if (grammarchecker.check(tokens, tokenizer.simplify(), tokenizer.getNum())) {
        CodeGenerator codegenerator;
        codegenerator.generate(tokens, tokenizer.simplify(), tokenizer.getNum());
    } else {
        errExit("Grammar Check failed", line);
    }
}

void firstPass(std::vector<string> *src) {
    for (unsigned int i = 0; i < src->size(); i++) {
        if ((*src)[i][0] == '#') {
            src->erase(src->begin() + i);
            i--;
        }
    }
    for (unsigned int i = 0; i < src->size(); i++) {
        if ((*src)[i][0] == '&') {
            string str = (*src)[i];
            str.erase(str.begin());
            str.erase(std::remove(str.begin(), str.end(), ' '), str.end());
            std::regex rx("^[a-zA-Z]+=(0b|0x)?[0-9a-fA-F]+$");
            std::smatch sm;
            if (!std::regex_match(str, sm, rx)) {
                errExit("Constant definition error", (*src)[i]);
            }
            constants.push_back({getSubStrBAChar(str, '=', true), (unsigned int)getHBDNum(getSubStrBAChar(str, '=', false).c_str())});
            src->erase(src->begin() + i);
            i--;
        }
    }
    for (unsigned int i = 0; i < src->size(); i++) {
        if ((*src)[i][0] == '%') {
            string name = (*src)[i];
            name.erase(name.begin());
            bool containsArgs = false;
            std::vector<DS> args;
            std::regex rx("^[A-Z_]+$");
            std::smatch sm;
            if (!std::regex_match(name, sm, rx)) {
                std::regex ry("^[A-Z_]+( \\$[a-z]+)+$");
                if (!std::regex_match(name, sm, ry)) {
                    errExit("Macro definition error", (*src)[i]);
                } else {
                    containsArgs = true;
                    std::vector<string> arguments = cutString(name, ' ');
                    arguments.erase(arguments.begin());
                    for (unsigned int j = 0; j < arguments.size(); j++) {
                        arguments[j].erase(std::remove(arguments[j].begin(), arguments[j].end(), '$'), arguments[j].end());
                    }
                    for (unsigned int j = 0; j < arguments.size(); j++) {
                        args.push_back({arguments[j], ""});
                    }
                }
            }
            std::vector<std::string> macro;
            src->erase(src->begin() + i);
            while ((*src)[i] != "%%") {
                macro.push_back((*src)[i]);
                src->erase(src->begin() + i);
            }
            src->erase(src->begin() + i);
            bool canPrecompileWhole = true;
            if (containsArgs) {
                canPrecompileWhole = false;
            } else {
                for (unsigned int j = 0; j < macro.size(); j++) {
                    if (macro[j].substr(0, 5) == "LABEL")
                        canPrecompileWhole = false;
                }
            }
            if (canPrecompileWhole) {
                for (unsigned int j = 0; j < macro.size(); j++) {
                    parseLine(macro[j]);
                }
                macros.push_back({name, bits, true, {}});
                bits.clear();
            } else {
                std::vector<string> instructions;
                for (unsigned int j = 0; j < macro.size(); j++) {
                    macro[j].erase(std::remove(macro[j].begin(), macro[j].end(), ' '), macro[j].end());
                    if (!(stringContains(macro[j], "LABEL") || stringContains(macro[j], "$") || (macro[j][0] == 'A' && macro[j][1] == '=' && !isNumber(macro[j][2])))) {
                        parseLine(macro[j]);
                        instructions.push_back("\\ci" + bits[0]);
                        bits.clear();
                    } else {
                        instructions.push_back(macro[j]);
                    }
                }
                
                macros.push_back({cutString(name, ' ')[0], instructions, false, args});
            }
            
            i--;
        }
    }
}

std::fstream openFile(string filename, bool ret = true) {
    std::fstream file;
    file.open(filename);
    if (!file.is_open() && ret) {
        printf("Could not open file %s.", filename.c_str());
        exit(1);
    }
    return file;
}

std::vector<string> fileFirstPass(string filename, bool ret = true) {
    std::fstream file = openFile(filename, ret);
    if (file.is_open()) {
        std::vector<string> data;
        std::string line;
        while(std::getline(file, line)){  //read data from file object and put it into string.
            if(line.size())
                data.push_back(line);
        }
        file.close();
        if (vectorContains(data, "\\")) {
            printf("\\ In file: %s.", filename.c_str());
            exit(1);
        }
        string lcrf = currfile;
        currfile = filename;
        firstPass(&data);
        currfile = lcrf;
        return data;
    } else if (ret) {
        printf("Could not open source file %s.", filename.c_str());
        exit(1);
    } else {
        return {};
    }
}

void processLabels(std::vector<string> *src) {
    unsigned int premOff = 0;
    for (unsigned int i = 0; i < src->size(); i++) {
        for (unsigned int j = 0; j < macros.size(); j++) {
            if ((*src)[i] == macros[j].name)
                premOff += macros[j].instructions.size() - 1;
        }
        if ((*src)[i].substr(0, 5) == "LABEL") {
            labels.push_back({cutString((*src)[i], ' ')[1], i + premOff});
            src->erase(src->begin() + i);
            i--;
        }
    }
}

void expandUncompiledMacros(std::vector<string> *src) {
    while (true) {
        firstPass(src);
        for (unsigned int i = 0; i < src->size(); i++) {
            for (unsigned int j = 0; j < macros.size(); j++) {
                if ((*src)[i] == macros[j].name && !macros[j].shouldBeExpanded) {
                    src->erase(src->begin() + i);
                    src->insert(std::begin(*src) + i, std::begin(macros[j].instructions), std::end(macros[j].instructions));
                    i--;
                }
            }
        }
        bool breakLoop = true;
        for (unsigned int i = 0; i < src->size(); i++) {
            for (unsigned int j = 0; j < macros.size(); j++) {
                if ((*src)[i] == macros[j].name && !macros[j].shouldBeExpanded)
                    breakLoop = false;
            }
        }
        if (breakLoop)
            break;
    }
    while (true) {
        for (unsigned int i = 0; i < src->size(); i++) {
            for (unsigned int j = 0; j < macros.size(); j++) {
                if (cutString((*src)[i], ' ')[0] == macros[j].name && !macros[j].shouldBeExpanded) {
                    string work = (*src)[i].substr(cutString((*src)[i], ' ')[0].length());
                    src->erase(src->begin() + i);
                    std::regex rx(string("^( [0-9A-Za-z]+){") + std::to_string(macros[j].arguments.size()) + "}$");
                    std::smatch sm;
                    if (!std::regex_match(work, sm, rx)) {
                        errExit("Macro missuse", (*src)[i]);
                    }
                    work = work.substr(1);
                    std::vector<string> arguments = cutString(work, ' ');
                    Macro macro = macros[j];
                    for (unsigned int k = 0; k < arguments.size(); k++) {
                        macro.arguments[k].value = arguments[k];
                    }
                    std::vector<string> instructions = macro.instructions;
                    for (unsigned int k = 0; k < instructions.size(); k++) {
                        for (unsigned int l = 0; l < macro.arguments.size(); l++) {
                            replace(instructions[k], "$" + macro.arguments[l].key, macro.arguments[l].value);
                        }
                    }
                    src->insert(std::begin(*src) + i, std::begin(instructions), std::end(instructions));
                    i--;
                }
            }
        }
        for (unsigned int i = 0; i < src->size(); i++) {
            for (unsigned int j = 0; j < macros.size(); j++) {
                if ((*src)[i] == macros[j].name && !macros[j].shouldBeExpanded)
                    expandUncompiledMacros(src);
            }
        }
        bool breakLoop = true;
        for (unsigned int i = 0; i < src->size(); i++) {
            for (unsigned int j = 0; j < macros.size(); j++) {
                if (cutString((*src)[i], ' ')[0] == macros[j].name && !macros[j].shouldBeExpanded)
                    breakLoop = false;
            }
        }
        if (breakLoop)
            break;
    }
}

void handleIncludes(std::vector<string> *src) {
    for (unsigned int i = 0; i < src->size(); i++) {
        if ((*src)[i][0] == '@') {
            string filename = (*src)[i].substr(1);
            src->erase(src->begin() + i);
            std::vector<string> include = fileFirstPass(filename);
            src->insert(std::begin(*src) + i, std::begin(include), std::end(include));
            i--;
        }
    }
}

void parseSource(std::vector<string> src) {
    handleIncludes(&src);
    expandUncompiledMacros(&src);
    processLabels(&src);
    for (unsigned int i = 0; i < src.size(); i++) {
        parseLine(src[i]);
        linen++;
    }
    if (useDec) {
        bits.push_back("49152");
    } else {
        bits.push_back("1100000000000000");
    }
}

int main(int argc, char **argv) {
    string filename;
    if (argc < 2) {
        printf("Usage: %s [-d] program.src", argv[0]);
        return 1;
    } else if (argc == 2) {
        filename = argv[1];
    } else if (argc == 3) {
        if (string(argv[1]) == "-d")
            useDec = true;
        filename = argv[2];
    }
    
    std::vector<string> src;
    src = fileFirstPass(filename);
    
    fileFirstPass("macros.src", false);

    currfile = filename;
    parseSource(src);

    std::ofstream code(getSubStrBAChar(string(filename), '.', true) + ".bit", std::ofstream::trunc);
    if (!code.is_open()) {
        printf("Could not open destination file %s.", (string(filename) + ".bit").c_str());
        return 1;
    }
    for (unsigned int i = 0; i < bits.size(); i++) {
        code << bits[i] << std::endl;
    }
    code.close();
    return 0;
}