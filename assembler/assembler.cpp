#include "stdio.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <bitset>
#include "../misc.hpp"

using std::string;
using namespace TYPES;
using namespace BITMAN;
using namespace MISC;
std::vector<std::string> bits;
unsigned int linen = 1;
bool useDec = false;

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

template <typename T>
bool isBetween(T min, T x, T max) {
    return (min < x && x < max);
}

template <typename T>
bool vecContains(std::vector<T> v, T x) {
    return (std::find(v.begin(), v.end(), x) != v.end());
}

std::string getFileName(std::string const& str) {
    std::string::size_type pos = str.find('.');
    if (pos != std::string::npos) {
        return str.substr(0, pos);
    } else {
        return str;
    }
}

class Tokenizer {
    private:
        string work;
        std::vector<Tokens> res;
        SU iter = 0;
        short num = 0;
    private:
        void ignore(char c) {
            work.erase(std::remove(work.begin(), work.end(), c), work.end());
        }
        void matchRegex(string pattern, Tokens type, bool setnum) {
            std::regex rx(pattern);
            std::smatch sm;
            if (std::regex_search(work, sm, rx)) {
                if (sm.position() == 0) {
                    res.push_back(type);
                    if (setnum)
                        num = atoi(work.substr(0, sm.length()).c_str());
                    work.erase(0, sm.length());
                    iter = 0;
                }
            }
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
                if (work[i] != c[i]) {
                    return;
                }
            }
            res.push_back(type);
            work.erase(0, c.size());
            iter = 0;
        }
        void matchExact(char c, Tokens type, bool checknum) {
            if (checknum) {
                matchRegex("-?[0-9]+", Tokens::Number, true);
            }
            matchExact(c, type);
        }
    public:
        std::vector<Tokens> tokenize(string str) {
            res.clear();
            work = str;
            ignore(' ');
            ignore('\t');
            while (work.length() != 0 && iter <= 11) {
                matchRegex("[0-9]+", Tokens::Number, true);
                matchExact('+', Tokens::Plus);
                matchExact('-', Tokens::Minus, true);
                matchExact('&', Tokens::And);
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
                matchRegex(";?JMP", Tokens::JMP, false);
                iter++;
            }
            if (iter >= 10) {
                printf("Too many iterations on tokenization on line %i. Check your syntax.", linen);
                exit(1);
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
                if (work[i] != c[i]) {
                    return false;
                }
            }
            return true;
        }
    public:
        bool check(std::vector<Tokens> tokens, std::vector<Tokens> stokens, short num) {
            work = stokens;
            //Exception: A = Number
            if (tokens.size() == 3 && tokens[0] == Tokens::A && tokens[1] == Tokens::Equal && tokens[2] == Tokens::Number && isBetween((short)-2, num, (short)32765)) {
                return true;
            }

            if (!vecContains(work, Tokens::Equal)) {
                //Operations without Equal
                if (work.size() == 1 && (work[0] == Tokens::_REG || tokens[0] == Tokens::JMP)) {
                    return true;
                } else if (work.size() == 2 && (matchExact({Tokens::_SOPER, Tokens::_REG}) || matchExact({Tokens::_REG, Tokens::_JMP}) || (work[0] == Tokens::_REG && work[1] == Tokens::Number && num == -1))) {
                    return true;
                } else if (work.size() == 3 && (matchExact({Tokens::_REG, Tokens::_OPER, Tokens::_REG}) || (work[0] == Tokens::_REG && tokens[1] == Tokens::Minus && work[2] == Tokens::_REG) || matchExact({Tokens::_SOPER, Tokens::_REG, Tokens::_JMP}) || (work[0] == Tokens::_REG && tokens[1] == Tokens::Plus && work[2] == Tokens::Number && num == 1) || (work[0] == Tokens::_REG && work[1] == Tokens::Number && num == -1 && work[2] == Tokens::_JMP))) {
                    return true;
                } else if (work.size() == 4 && (matchExact({Tokens::_REG, Tokens::_OPER, Tokens::_REG, Tokens::_JMP}) || (work[0] == Tokens::_REG && tokens[1] == Tokens::Minus && work[2] == Tokens::_REG && work[3] == Tokens::_JMP) || (work[0] == Tokens::_REG && tokens[1] == Tokens::Plus && work[2] == Tokens::Number && num == 1 && work[3] == Tokens::_JMP))) {
                    return true;
                }
            } else {
                //Equal Operations
                //Let's simplify
                for (SU i = 0; i <= 1; i++) {
                    if (work[0] == Tokens::_REG && work[1] == Tokens::Coma && work[2] == Tokens::_REG) {
                        work.erase(std::next(work.begin(), 1), std::next(work.begin(), 3));
                    }
                }
                if (!(work[0] == Tokens::_REG && work[1] == Tokens::Equal)) {
                    return false;
                }
                work.erase(std::next(work.begin(), 0), std::next(work.begin(), 2));
                if (work.size() == 1 && ((work[0] == Tokens::_REG) || ((work[0] == Tokens::Number) && (isBetween((short)-2, num, (short)2))))) {
                    return true;
                } else if (work.size() == 2 && (matchExact({Tokens::_SOPER, Tokens::_REG}) || matchExact({Tokens::_REG, Tokens::_JMP}) || (matchExact({Tokens::Number, Tokens::_JMP}) && (isBetween((short)-2, num, (short)2))) || (work[0] == Tokens::_REG && work[1] == Tokens::Number && num == -1))) {
                    return true;
                } else if (work.size() == 3 && (matchExact({Tokens::_REG, Tokens::_OPER, Tokens::_REG}) || (work[0] == Tokens::_REG && tokens[-2] == Tokens::Minus && work[2] == Tokens::_REG) || matchExact({Tokens::_SOPER, Tokens::_REG, Tokens::_JMP})  || (work[0] == Tokens::_REG && tokens[1] == Tokens::Plus && work[2] == Tokens::Number && num == 1) || (work[0] == Tokens::_REG && work[1] == Tokens::Number && num == -1 && work[2] == Tokens::_JMP))) {
                    return true;
                } else if (work.size() == 4 && (matchExact({Tokens::_REG, Tokens::_OPER, Tokens::_REG, Tokens::_JMP}), (work[0] == Tokens::_REG && tokens[-3] == Tokens::Minus && work[2] == Tokens::_REG && work[3] == Tokens::_JMP) || (work[0] == Tokens::_REG && tokens[1] == Tokens::Plus && work[2] == Tokens::Number && num == 1 && work[3] == Tokens::_JMP))) {
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
                if (twork[i] != c[i]) {
                    return false;
                }
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
                    if (stwork[0] == Tokens::_REG && stwork[1] == Tokens::Coma && stwork[2] == Tokens::_REG) {
                        pullDest(twork[0]);
                        twork.erase(std::next(twork.begin(), 0), std::next(twork.begin(), 2));
                        stwork.erase(std::next(stwork.begin(), 0), std::next(stwork.begin(), 2));
                    } else if(stwork[0] == Tokens::_REG && stwork[1] == Tokens::Equal) {
                        pullDest(twork[0]);
                        twork.erase(std::next(twork.begin(), 0), std::next(twork.begin(), 2));
                        stwork.erase(std::next(stwork.begin(), 0), std::next(stwork.begin(), 2));
                        break;
                    }
                }
            }
            if (vecContains(twork, Tokens::A) && vecContains(twork, Tokens::AM)) {
                printf("Can't use A and A* as operators on line%i.", linen);
                exit(1);
            }
            if (vecContains(twork, Tokens::AM)) {
                work.b[SM] = 1;
                std::replace(twork.begin(), twork.end(), Tokens::AM, Tokens::A);
            }
            if (stwork[-1] == Tokens::_JMP) {
                pullJump(twork[-1]);
                twork.pop_back();
                stwork.pop_back();
            }

            //TODO: implement operations (better oper generation)

            if (twork.size() == 1) {
                if (stwork[0] == Tokens::_REG) {
                    if (twork[0] == Tokens::A) {
                        work.b[ZX] = 1;
                    } else {
                        work.b[ZY] = 1;
                    }
                    work.b[F] = 1;
                } else {
                    work.b[ZX] = 1;
                    work.b[ZY] = 1;
                    if (num == 1) {
                        work.b[NO] = 1;
                    } else if (num == -1) {
                        work.b[NX] = 1;
                        work.b[NY] = 1;
                        work.b[F] = 1;
                        work.b[NO] = 1;
                    }
                }
            } else if (twork.size() == 2) {
                work.b[F] = 1;
                if (twork[0] == Tokens::D || twork[1] == Tokens::D) {
                    work.b[ZY] = 1;
                    work.b[NY] = 1;
                } else {
                    work.b[ZX] = 1;
                    work.b[NX] = 1;
                }
                if (stwork[0] == Tokens::_SOPER) {
                   if (twork[0] == Tokens::Minus) {
                       work.b[NO] = 1;
                   } else {
                       work.b[NY] = !work.b[NY];
                       work.b[NX] = !work.b[NX];
                   }
                }
            } else {
                if (twork[1] == Tokens::Plus && twork[2] == Tokens::Number) {
                    if (twork[0] == Tokens::A) {
                        work.b[ZX] = 1;
                    } else {
                        work.b[ZY] = 1;
                    }
                }
                if (twork[1] == Tokens::Minus) {
                    if (twork[0] == Tokens::A) {
                        work.b[NX] = 1;
                    } else {
                        work.b[NY] = 1;
                    }
                }
                if (!(twork[1] == Tokens::And || twork[1] == Tokens::Or)) {
                    work.b[F] = 1;
                } else if (twork[1] == Tokens::Or) {
                    work.b[NX] = 1;
                    work.b[NY] = 1;
                }
                if (!(twork[1] == Tokens::And || matchExact({Tokens::D, Tokens::Plus, Tokens::A}))) {
                    work.b[NO] = 1;
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
    if (line[0] == '#') {
        return;
    }
    Tokenizer tokenizer;
    std::vector<Tokens> tokens = tokenizer.tokenize(line);
    GrammarChecker grammarchecker;
    if (grammarchecker.check(tokens, tokenizer.simplify(), tokenizer.getNum())) {
        CodeGenerator codegenerator;
        codegenerator.generate(tokens, tokenizer.simplify(), tokenizer.getNum());
    } else {
        printf("Grammar Check failed on line %u. Check your syntax.", linen);
        exit(1);
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
        useDec = true;
        filename = argv[2];
    }

    std::fstream source;
    source.open(filename);
    if (!source.is_open()) {
        printf("Could not open source file %s.", filename.c_str());
        return 1;
    }
    std::string line;
    while(std::getline(source, line)){  //read data from file object and put it into string.
        parseLine(line);
        linen++;
    }
    source.close();
    std::ofstream code(getFileName(string(filename)) + ".bit", std::ofstream::trunc);
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