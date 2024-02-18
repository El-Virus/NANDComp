#include "../debug.h"
#include "../misc.hpp"
#include <stdio.h>
#include <ctype.h>
#include <bitset>
#include <fstream>
#include <regex>
#include <tr1/regex>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

#pragma GCC diagnostic ignored "-Wwrite-strings"

using namespace BITMAN;
using namespace MISC;
using namespace TYPES;

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
	_SPECIAL,
	Machine,
	Simulator,
	__END__
};

template <typename T, typename U>
struct KVP {
	T key;
	U value;
};

struct Macro {
	std::string name;
	std::vector<std::string> instructions;
	bool shouldBeExpanded;
	std::vector<KVP<std::string, std::string>> arguments;
};

std::vector<std::string> bits;
unsigned int linen = 1;
std::string currfile;
bool useDec = false;
std::vector<KVP<std::string, unsigned int>> labels;
std::vector<KVP<std::string, unsigned int>> constants;
std::vector<Macro> macros;

/**
 * @brief Check if a value is between a range
 * 
 * @tparam T type
 * @param min Minimum value
 * @param x Value
 * @param max Maximum value
 * @return true if between
 * @return false if not between
 */
template <typename T>
bool isBetween(const T& min, const T& x, const T& max) {
	return (min < x && x < max);
}

/**
 * @brief Check if a vector contains an element
 * 
 * @tparam T type of element
 * @param v Vector
 * @param x Element
 * @return true if contained
 * @return false if not contained
 */
template <typename T>
bool vecContains(const std::vector<T>& v, const T& x) {
	return (std::find(v.begin(), v.end(), x) != v.end());
}

/**
 * @brief Get a substring up to a char, or return the string
 * 
 * @param str String to divide
 * @param chr Character to find
 * @param before Whether if to return the part before or after the char
 * @return std::string Substring, string if char not found
 */
std::string getSubStrBAChar(const std::string& str, const char chr, const bool before) {
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

/**
 * @brief Cut a string each time a specific character is found
 * 
 * @param str The string to be cut
 * @param delim The character to cut at
 * @return std::vector<std::string> Vector of substrings
 */
std::vector<std::string> cutString(const std::string& str, const char delim) {
	std::stringstream ss(str);
	std::string segment;
	std::vector<std::string> seglist;

	while (std::getline(ss, segment, delim)) {
		seglist.push_back(segment);
	}
	return seglist;
}

/**
 * @brief Replace one instance of a string to another string in a string
 * 
 * @param str The main string
 * @param from The string that should be replaced
 * @param to The string that will replace the old one
 * @return true If string replaced
 * @return false If string not found
 */
bool replace(std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = str.find(from);
	if(start_pos == std::string::npos)
		return false;
	
	str.replace(start_pos, from.length(), to);
	return true;
}

/**
 * @brief Check if a label has been defined
 * 
 * @param label Label to find
 * @return true if found
 * @return false if not found
 */
bool labelExists(const std::string& label) {
	for (SU i = 0; i < labels.size(); i++) {
		if (labels[i].key == label)
			return true;
	}
	return false;
}

/**
 * @brief Get the position of a label
 * 
 * @param label The label to fetch
 * @return unsigned int Label position, 0 if not defined
 */
unsigned int getLabel(const std::string& label) {
	for (SU i = 0; i < labels.size(); i++) {
		if (labels[i].key == label)
			return labels[i].value;
	}
	return 0;
}

/**
 * @brief Check if a constant has been defined
 * 
 * @param constant Constant to find
 * @return true if defined
 * @return false if not defined
 */
bool constantExists(const std::string& constant) {
	for (SU i = 0; i < constants.size(); i++) {
		if (constants[i].key == constant)
			return true;
	}
	return false;
}

/**
 * @brief Get the value of a constant
 * 
 * @param constant Constant to fetch
 * @return unsigned int Constant value, 0 if not defined
 */
unsigned int getConstant(const std::string& constant) {
	for (SU i = 0; i < constants.size(); i++) {
		if (constants[i].key == constant)
			return constants[i].value;
	}
	return 0;
}

/**
 * @brief Get substring matching regex
 * 
 * @param pattern Regex pattern
 * @param str String to apply the regex to
 * @return std::string The matched substring
 */
std::string getRegex(const std::string& pattern, const std::string& str) {
	std::regex rx(pattern);
	std::smatch sm;
	if (std::regex_search(str, sm, rx)) {
		if (sm.position() == 0)
			return str.substr(0, sm.length()).c_str();
	}
	return "";
}

/**
 * @brief Return the decimal representation of a string which
 * 			may be a binary, decimal or hexadecimal number
 * 
 * @param str String
 * @return int Decimal representation
 */
int getHBDNum(const std::string& str) {
	std::string ret = getRegex("0b[0-1]+", str);
	if (ret != "")
		return stoi(ret.substr(2), (size_t *)nullptr, 2);
	
	ret = getRegex("0x[0-9a-fA-F]+", str);
	if (ret != "")
		return stoi(ret.substr(2), (size_t *)nullptr, 16);
	
	ret = getRegex("-?[0-9]+", str);
	if (ret != "")
		return atoi(ret.c_str());
	
	throw "Unable to obtain number";
}

/**
 * @brief Returns the number of digits of a number in a string at a certain position
 * 
 * @param str String where number is located
 * @param at Position of the number
 * @return size_t Number of digits
 */
size_t countNumberDigits(const std::string& str, const size_t at) {
	int n = 1;
	size_t str_sz = str.size();
	char ll1, ll2, ll3, ul1, ul2, ul3;
	ll1 = ll2 = ll3 = '0';
	ul1 = ul2 = ul3 = '9'; 

	if (str[at] == '0') {
		if (str[at + 1] == 'b') {
			n++;
			ul1 = ul2 = ul3 = '1';
		} else if (str[at + 1] == 'x') {
			n++;
			ll2 = 'a';
			ul2 = 'f';
			ll3 = 'A';
			ul3 = 'F';
		}
	}

	while ((at + n) < str_sz && ((str[at + n] >= ll1 && str[at + n] <= ul1) || \
		(str[at + n] >= ll2 && str[at + n] <= ul2) || (str[at + n] >= ll3 && str[at + n] <= ul3)))
		++n;

	return n;
}

/**
 * @brief Check whether a string is contained in another
 * 
 * @param container String that may contain the substring
 * @param containee String which may be contained
 * @return true if contained
 * @return false if not contained
 */
bool stringContains(const std::string& container, const std::string& containee) {
	return (container.find(containee) != std::string::npos);
}

/**
 * @brief Check whether if a string inside a vector contains a substring
 * 
 * @param vec Vector which contains the string that may contain the substring
 * @param containee The string that may be contained in a string of the vector
 * @return true if contained
 * @return false if not
 */
bool vectorContains(const std::vector<std::string>& vec, const std::string& containee) {
	for (unsigned int i = 0; i < vec.size(); i++) {
		if (stringContains(vec[i], containee))
			return true;
	}
	return false;
}

/**
 * @brief Get the position of the maximum value in a vector
 * 
 * @tparam T Type of element in vector
 * @param vec Vector
 * @return size_t Position of the maximum value
 */
template <typename T>
size_t getMaxValIdx(const std::vector<T>& vec) {
	size_t vec_sz = vec.size();
	if (!vec_sz)
		return 0;
	size_t max_pos = 0;
	T max = vec[0];
	for (size_t i = 1; i < vec_sz; i++) {
		if (vec[i] > max) {
			max = vec[i];
			max_pos = i;
		}
	}
	return max_pos;
}

/**
 * @brief Exit with a message thrown by an error in a line (with an exit code)
 * 
 * @param message Error message
 * @param line Line where the error is present
 * @param code Exit code
 */
[[noreturn]] void errExit(std::string message, std::string line, int code = 1) {
	printf("%s on [%s](expanding includes), (%s). Check your Syntax.\n", message.c_str(), line.c_str(), currfile.c_str());
	exit(code);
}

class PreExpr {
	public:
		/**
		 * @brief Finds the corresponding closing parenthesis to a level
		 * 
		 * @param expr String to operate on
		 * @param from The position of first character inside the parenthesis
		 * @return size_t The position of the closing parenthesis
		 */
		static size_t findClosing(const std::string& expr, size_t from) {
			++inflvl;
			unsigned int open = 0;
			size_t expr_sz = expr.size();
			for (size_t i = from; i < expr_sz; i++) {
				if (expr[i] == '(') {
					open++;
				} else if (expr[i] == ')') {
					if (!open) {
						debug_log("Closing for %lu at %lu \n", from, i);
						--inflvl;
						return i;
					}
					open--;
				}
			}
			--inflvl;
			errExit("Parenthesis not closed properly on preprocessor expresion", "");
		}
	protected:
		enum PEP {
			Expression,
			BitwiseOR,
			BitwiseXOR,
			BitwiseAND,
			AddSub,
			MulDivMod,
			BitwiseNOT
		};

		/**
		 * @brief Gets the priority of an expression
		 * 
		 * @param expr Expression
		 * @return PEP Priority
		 */
		static PEP getExprPriority(const std::variant<int, char, PreExpr>& expr) {
			if (std::holds_alternative<int>(expr) || std::holds_alternative<PreExpr>(expr)) {
				return PEP::Expression;
			} else {
				char op = std::get<char>(expr);
				switch (op) {
					case '~':
					case '!':
						return PEP::BitwiseNOT;

					case '*':
					case '/':
					case '%':
						return PEP::MulDivMod;

					case '+':
					case '-':
						return PEP::AddSub;

					case '&':
						return PEP::BitwiseAND;

					case '^':
						return PEP::BitwiseXOR;

					case '|':
						return PEP::BitwiseOR;
					
					default:
						errExit("Unrecognized character when evaluating preprocessor expression", std::to_string(op));
						break;
				}
			}
		}

		typedef struct {
			int first;
			char oper;
			int second;
		} operation_chain_t;

		/**
		 * @brief Performs the operation on op.oper to op.first (and op.second)
		 * 
		 * @param op Operation chain
		 * @return int Result of the operation
		 */
		static int operate(operation_chain_t op) {
			switch (op.oper) {
				case '~':
				case '!':
					return ~op.first;

				case '*':
					return op.first * op.second;
				case '/':
					return op.first / op.second;
				case '%':
					return op.first % op.second;

				case '+':
					return op.first + op.second;
				case '-':
					return op.first - op.second;

				case '&':
					return op.first & op.second;

				case '^':
					return op.first ^ op.second;

				case '|':
					return op.first | op.second;
				
				default:
					errExit("Unrecognized character when evaluating preprocessor expression", "");
					break;
			}
		}

		std::vector<std::variant<int, char, PreExpr>> chain;

		/**
		 * @brief Evaluates an element of the chain
		 * 
		 * @param which The index of the element to evaluate
		 * @param pchain A pointer of the chain priority vector, if any
		 * @param can_operate Wether an operation will be evaluated, externally should be true
		 * @return int The result of the evaluation
		 */
		int eval_element(size_t which, std::vector<PEP>* pchain = NULL, bool can_operate = true) {
			if (std::holds_alternative<PreExpr>(chain[which])) {
				return std::get<PreExpr>(chain[which]).eval();
			} else if (std::holds_alternative<int>(chain[which])) {
				return std::get<int>(chain[which]);
			} else { //Operation
				operation_chain_t op;
				op.oper = std::get<char>(chain[which]);
				if (which == 0 && !(op.oper == '~' || op.oper == '!'))
					errExit("First element of preprocessor expression is an operation", "");
				if (which == chain.size() - 1)
					errExit("Last element of preprocessor expression is an operator", "");
				
				//Handle negative numbers
				if (op.oper == '-' && !std::holds_alternative<char>(chain[which + 1]) &&
					(!can_operate || std::holds_alternative<char>(chain[which - 1]))) {
						which++;
						op.first = -1;
						op.oper = '*';
						op.second = eval_element(which, pchain, false);
						chain.erase(chain.begin() + which);
						if (pchain != NULL)
							pchain->erase(pchain->begin() + which);						
						goto operation;
				}

				if (!can_operate) {
					if (op.oper != '-' || std::holds_alternative<char>(chain[which + 1]))
						errExit("There are two consecutive operations in preprocessor expression", "");
					//Else, we just want to negate the symbol of the next expression
					return (-1 * eval_element(which + 1, NULL, false));
				}

				op.first = eval_element(which - 1, pchain, false);
				chain.erase(chain.begin() + (which - 1));
				if (pchain != NULL)
					pchain->erase(pchain->begin() + (which - 1));

				if (!(op.oper == '~' || op.oper == '!')) {
					op.second = eval_element(which, pchain, false);
					chain.erase(chain.begin() + which);
					if (pchain != NULL)
						pchain->erase(pchain->begin() + which);
				}

operation:
				chain[which - 1] = operate(op);
				if (pchain != NULL)
					(*pchain)[which - 1] = PEP::Expression;

				return eval_element(which - 1, NULL, false);
			}
		}

	public:
		/**
		 * @brief Constructs a PreExpr object by parsing an expression
		 * 
		 * @param expr Expression to parse
		 */
		PreExpr(const std::string& expr) {
			++inflvl;
			size_t expr_sz = expr.size();

#if DBLVL >= DBLVL_DEBUG
			debug_log("Evaluating expr:");
			_debug_log_header();
			printf("%s\n", expr.c_str());
			for (int i = 1; i < expr_sz; i *= 10) {
				_debug_log_header();
				for (int j = 0; j < expr_sz; j += i) {
					printf("%c", (j / i) % 10 + '0');
					for (int k = 1; k < i; k++) {
						printf(" ");
					}
				}
				printf("\n");
			}
#endif

			for (size_t i = 0; i < expr_sz; i++) {
				if (isdigit(expr[i])) {
					size_t cnt = countNumberDigits(expr, i);
					chain.push_back(getHBDNum(expr.substr(i, cnt)));
					i += (cnt - 1);
				} else {
					switch (expr[i]) {
						case '(': {
							fflush(NULL);
							i++;
							size_t closing = findClosing(expr, i);
							chain.push_back(PreExpr(expr.substr(i, closing - i)));
							i = closing;
							continue;
						}

						default:
							fflush(NULL);
							if (!isspace(expr[i]))
								chain.push_back(expr[i]);
							break;
					}
				}
			}
			--inflvl;
		}

		/**
		 * @brief Evaluates the expression chain
		 * 
		 * @return int Result of the chain evaluation
		 */
		int eval() {
			++inflvl;
			size_t chain_sz = chain.size();
			std::vector<PEP> priorities(chain_sz);
			for (size_t i = 0; i < chain_sz; i++) {
				priorities[i] = getExprPriority(chain[i]);
#if 0
				if (std::holds_alternative<int>(chain[i])) {
					printf("s%hi", std::get<int>(chain[i]));
				} else if (std::holds_alternative<char>(chain[i])) {
					printf("c%c", std::get<char>(chain[i]));
				} else {
					printf(",( ");
					std::get<PreExpr>(chain[i]).eval();
					printf(")");
				}
				printf(" ");
				return 0;
#else
			}
			
			while (chain.size() > 1) {
				size_t at = getMaxValIdx(priorities);
				eval_element(at, &priorities);
			}

			--inflvl;
			return eval_element(0);
#endif
		}
};

class Tokenizer {
	private:
		std::string work;
		std::vector<Tokens> res;
		SU iter = 0;
		short num = 2;
	private:
		/**
		 * @brief Remove (Ignore) all instances of a char from work
		 * 
		 * @param c Char to remove
		 */
		void ignore(const char c) {
			work.erase(std::remove(work.begin(), work.end(), c), work.end());
		}

		/**
		 * @brief Try to match a pattern with the next work term.
		 * 			If matched, push type to res and delete from work.
		 * 			Returns the matched string
		 * 
		 * @param pattern Pattern to match
		 * @param type Type of term matched by the pattern
		 * @return std::string The matched string
		 */
		std::string matchRegex(const std::string& pattern, const Tokens type) {
			std::regex rx(pattern);
			std::smatch sm;
			if (std::regex_search(work, sm, rx)) {
				if (sm.position() == 0) {
					res.push_back(type);
					std::string ret = work.substr(0, sm.length()).c_str();
					work.erase(0, sm.length());
					iter = 0;
					return ret;
				}
			}
			return "";
		}

		/**
		 * @brief Try to match a character with the next work term.
		 * 			If matched, push type to res and delete from work.
		 * @param c Character to match
		 * @param type Type of term matched by the character
		 */
		void matchExact(const char c, const Tokens type) {
			if (work[0] == c) {
				res.push_back(type);
				work.erase(0, 1);
				iter = 0;
			}
		}

		/**
		 * @brief Try to match a string with the next work term.
		 * 			If matched, push type to res and delete from work.
		 * @param c String to match
		 * @param type Type of term matched by the string
		 */
		void matchExact(const std::string& c, const Tokens type) {
			for (SU i = 0; i < c.size(); i++) {
				if (work[i] != c[i])
					return;
			}
			res.push_back(type);
			work.erase(0, c.size());
			iter = 0;
		}

		/**
		 * @brief Try to match a character with the next work term.
		 * 			If checknum, and char is a number set num
		 * 			If matched, push type to res and delete from work.
		 * @param c Character to match
		 * @param type Type of term matched by the character
		 * @param checknum Whether to try match a number
		 */
		void matchExact(const char c, const Tokens type, const bool checknum) {
			if (checknum) {
				std::string ret = matchRegex("-?[0-9]+", Tokens::Number);
				if (ret != "")
					num = atoi(ret.c_str());
			}
			matchExact(c, type);
		}

		/**
		 * @brief Attempt to match the next work term to a label or a constant
		 * 			If matched, set num to match and push the Number type to res
		 */
		void matchLabelConstant() {
			std::string constlabel;
			if (res[0] == Tokens::Simulator) {
				//Handle simulator instruction
				constlabel = getSubStrBAChar(work, ',', true);
				if (constantExists(constlabel)) {
					/*
					HACK: In order to avoid implementing another type or multiple numbers
					per instruction, we store the numbers as tokens past their end value.
					Given that the way we handle simulator is independent from other instructions
					we can decode the desired values later
					*/
					res.push_back((Tokens)(Tokens::__END__ + getConstant(constlabel)));
					work.erase(0, constlabel.length());
				}
			} else {
				constlabel = getSubStrBAChar(work, ';', true);
				if (labelExists(constlabel)) {
					num = getLabel(constlabel);
					res.push_back(Tokens::Number);
					work.erase(0, constlabel.length());
				} else if (constantExists(constlabel)) {
					num = getConstant(constlabel);
					res.push_back(Tokens::Number);
					work.erase(0, constlabel.length());
				}
			}
			//iter = 0;
		}

		/**
		 * @brief Attempt to match the next term of work to a number
		 * 			If matched, set num to match and push the Number type to res
		 */
		void matchNumber() {
			std::string ret = matchRegex("(0b)[0-1]+", Tokens::Number);
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
		/**
		 * @brief Transform a line of code into tokens for further processing
		 * 
		 * @param str The line of code to be tokenized
		 * @return std::vector<Tokens> Tokenized code
		 */
		std::vector<Tokens> tokenize(std::string str) {
			res.clear();
			work = str;
			ignore(' ');
			ignore('\t');
			ignore('_');

			while (work.length() != 0 && iter <= 17) {
				debug_log("Iteration: %s", work.c_str());
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
				matchExact("MACH", Tokens::Machine);
				matchExact("SIM", Tokens::Simulator);
				matchLabelConstant();
				iter++;
			}

			if (iter >= 17)
				errExit("Too many iterations on tokenization", str);
			
			return res;
		}

		/**
		 * @brief Convert the set of tokens into simplified tokens
		 * 			to make further processing easier
		 * 
		 * @return std::vector<Tokens> Simplified tokens
		 */
		std::vector<Tokens> simplify() {
			std::vector<Tokens> simpTokens;
			for (int i = 0; i < res.size(); i++) {
				if (isBetween(Tokens::_OPER, res[i], Tokens::_SOPER)) {
					simpTokens.push_back(Tokens::_OPER);
				} else if (isBetween(Tokens::_SOPER, res[i], Tokens::_REG)) {
					simpTokens.push_back(Tokens::_SOPER);
				} else if (isBetween(Tokens::_REG, res[i], Tokens::_JMP)) {
					simpTokens.push_back(Tokens::_REG);
				} else if (isBetween(Tokens::_JMP, res[i], Tokens::_SPECIAL)) {
					simpTokens.push_back(Tokens::_JMP);
				} else if (isBetween(Tokens::_SPECIAL, res[i], Tokens::__END__)){
					simpTokens.push_back(Tokens::_SPECIAL);
				} else {
					simpTokens.push_back(res[i]);
				}
			}
			return simpTokens;
		}

		/**
		 * @brief Returns the number asociated with a tokenized line of code
		 * 
		 * @return short Number
		 */
		short getNum() {
			return num;
		}
};

class GrammarChecker {
	private:
		std::vector<Tokens> work;
	private:
		/**
		 * @brief Check if a vector of tokens exactly matches work
		 * 
		 * @param c Vector of tokens
		 * @return true if it exactly matches
		 * @return false if it does not match exactly
		 */
		bool matchExact(const std::vector<Tokens>& c) {
			for (SU i = 0; i < c.size(); i++) {
				if (work[i] != c[i])
					return false;
			}
			return true;
		}
	public:
		/**
		 * @brief Check if a vector of tokens, given also it's simplified form and associated number
		 * 			is arranged in a correct way in order to generate machine code from it
		 * 
		 * @param tokens Vector of tokens
		 * @param stokens Simplified version of the vector of tokens
		 * @param num Number associated with the vector of tokens
		 * @return true if further processing can be performed
		 * @return false if an error should be thrown
		 */
		bool check(std::vector<Tokens> tokens, std::vector<Tokens> stokens, short num) {
			work = stokens;

			//Exception: A = Number
			if (tokens.size() == 3 && tokens[0] == Tokens::A && tokens[1] == Tokens::Equal && tokens[2] == Tokens::Number && isBetween((short)-2, num, (short)32765))
				return true;
			
			//Exception: Special operations
			if (work[0] == Tokens::_SPECIAL) {
				if (tokens[0] == Tokens::Machine) {
					if (tokens.size() == 2 && tokens[1] == Number)
						return true;
				} else if (tokens[0] == Tokens::Simulator) {
					if (tokens.size() == 1) {
						printf("Simulator instruction missing its parameters\n");
						return false;
					}
					for (int i = 1; i < tokens.size(); i++){
						if (tokens[i] < Tokens::__END__ && tokens[i] != Tokens::Coma)
							printf("Simulator instruction only accepts simulator specific values\n");
						return true;
					}
				} else {
					printf("Special token check not implemented");
				}
				return false;
			}

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
		/**
		 * @brief Set the destination bits given a token that can be used a destination
		 * 
		 * @param token Destination token
		 */
		void pullDest(const Tokens token) {
			if (token == Tokens::A) {
				work.b[InstructionMap::A] = 1;
			} else if (token == Tokens::AM) {
				work.b[InstructionMap::AM] = 1;
			} else {
				work.b[InstructionMap::D] = 1;
			}
		}

		/**
		 * @brief Set the jump bits given a jmp token
		 * 
		 * @param token Jmp token
		 */
		void pullJump(const Tokens token) {
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

		/**
		 * @brief Check if a vector of tokens exactly matches work
		 * 
		 * @param c Vector of tokens
		 * @return true if it exactly matches
		 * @return false if it does not match exactly
		 */
		bool matchExact(const std::vector<Tokens>& c) {
			for (SU i = 0; i < c.size(); i++) {
				if (twork[i] != c[i])
					return false;
			}
			return true;
		}

		/**
		 * @brief Convert a decimal number into a string of bits
		 * 
		 * @param dec Decimal number
		 * @return std::string String of bits
		 */
		std::string decToBin(const short dec) {
			return std::bitset<16>(dec).to_string();
		}
	public:
		/**
		 * @brief Generate machine code from a valid vector of tokens, its simplified version an associated number
		 * 
		 * @param tokens Vector of tokens
		 * @param stokens Vector of simplified tokens
		 * @param num Associated number
		 */
		void generate(std::vector<Tokens> tokens, std::vector<Tokens> stokens, short num) {
			twork = tokens;
			stwork = stokens;

			//Exception: A = Number
			if (tokens.size() == 3 && tokens[0] == Tokens::A && tokens[1] == Tokens::Equal && tokens[2] == Tokens::Number) {
				debug_log("Set A to: %d", num);
				if (useDec) {
					bits.push_back(std::to_string(num));
				} else {
					bits.push_back(decToBin(num));
				}
				return;
			}

			//Exception: Special operations
			if (stokens[0] == Tokens::_SPECIAL) {
				debug_log("Special operation:");
				if (tokens[0] == Tokens::Machine) {
					debug_log(" Machine");
					if (tokens.size() == 2 && tokens[1] == Number) {
						if (useDec) {
							bits.push_back(std::to_string(num));
						} else {
							bits.push_back(decToBin(num));
						}
					}
				} else if (tokens[0] == Tokens::Simulator) {
					debug_log(" Simulator");
					work.b[CI] = 1;
					work.b[__NULL3__] = 1;

					for (int i = 1; i < tokens.size(); i++) {
						if (tokens[i] == Tokens::Coma)
							continue;
						
						int index = (tokens[i] - Tokens::__END__); //Convert back to the correct number
						if (index > 13)
							errExit("Simulator var higher than allowed", "N/A");
						
						work.b[index] = 1;
					}

					if (useDec) {
						bits.push_back(std::to_string(composeWord(work)));
					} else {
						bits.push_back(decToBin(composeWord(work)));
					}
				} else {
					errExit("Special operation implementation missing", "N/A");
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

			if (vecContains(twork, Tokens::A) && vecContains(twork, Tokens::AM))
				errExit("Can't use A and A* as operators", " ~line (without special lines)" + linen);
			
			if (vecContains(twork, Tokens::AM)) {
				debug_log("Using memory instead of A");
				work.b[SM] = 1;
				std::replace(twork.begin(), twork.end(), Tokens::AM, Tokens::A);
			}

			if (stwork.back() == Tokens::_JMP) {
				pullJump(twork.back());
				twork.pop_back();
				stwork.pop_back();
			}

			/*
			Fair warning: The next bit of code comes from me staring at the instruction table
			for too long trying to find the least number of conditions required to generate
			the correct opcode.
			So, if you can't make sense of it, that's why.
			*/

			switch (twork[0]) {
				case Tokens::A:
					debug_log("A is first opperand");
					if (twork.size() != 1) {
						debug_log(" A swap has been performed");
						work.b[SW] = 1;
					}
					break;
				
				case Tokens::Minus:
					debug_log("The operator is minus");
						work.b[ZX] = 1;
						work.b[OP1] = 1;
						if (twork.size() > 1) {
							if (twork[1] == Tokens::D)
								work.b[SW] = 1;
							if (twork.size() == 2 && num != 1)
								work.b[U] = 1;
						}
					break;

				case Tokens::Tilde:
					debug_log("The operator is tilde, invert");
					work.b[OP0] = 1;
					work.b[OP1] = 1;
					if (twork.size() > 1)
						if (twork[1] == Tokens::A)
							work.b[SW] = 1;
					break;
				
				default:
					break;
			}

			if (twork.size() == 1) {
				debug_log("Taking into account that operation is one byte long");
				work.b[ZX] = 1;
				if (twork[0] == Tokens::D)
					work.b[SW] = 1;
				if (num == -1) {
					work.b[OP1] = 1;
				} else if (num != 0) {
					work.b[U] = 1;
				}
			} else {
				switch (twork[1]) {
					case Tokens::Or:
						debug_log("The operator is or");
						work.b[OP0] = 1;
						break;
					
					case Tokens::Number:
					case Tokens::Minus:
						debug_log("Number or minus, setting OP1");
						work.b[OP1] = 1;
						// falltrough
					case Tokens::Plus:
						debug_log("Arithmetic operation");
						work.b[U] = 1;
						break;
					
					default:
						break;
				}
			}

			if (num == 1 || num == -1)
				work.b[OP0] = 1;
			
			if (useDec) {
				bits.push_back(std::to_string(composeWord(work)));
			} else {
				bits.push_back(decToBin(composeWord(work)));
			}
		}
};

/**
 * @brief Transform a line of code into machine code.
 * 		AKA: Expand macros, tokenize, check and generate.
 * @note: Result goes in bits var
 * 
 * @param line The line of code
 */
void parseLine(std::string line) {
	inflvl++; debug_inform("Parsing line: %s", line.c_str()); inflvl++; 
	for (unsigned int i = 0; i < macros.size(); i++) {
		if (line == macros[i].name) {
			debug_inform("Expanding macro %s", line.c_str());
			bits.insert(std::end(bits), std::begin(macros[i].instructions), std::end(macros[i].instructions));
			inflvl--; inflvl--;
			return;
		}
	}

	if (line.substr(0, 3) == "\\ci") {
		bits.push_back(line.substr(3));
		inflvl--; inflvl--;
		return;
	}

	debug_inform("Tokenizing line");
	Tokenizer tokenizer;
	std::vector<Tokens> tokens = tokenizer.tokenize(line);

	//Optimized out when necessary
	debug_log("Tokens follow:");
	for (int i = 0; i < tokens.size(); i++) {
		debug_log(" %d", tokens[i]);
	}

	GrammarChecker grammarchecker;
	if (grammarchecker.check(tokens, tokenizer.simplify(), tokenizer.getNum())) {
		debug_inform("Generating code for line");
		CodeGenerator codegenerator;
		codegenerator.generate(tokens, tokenizer.simplify(), tokenizer.getNum());
	} else {
		errExit("Grammar Check failed", line);
	}
	inflvl--; inflvl--;
}

//TODO: Implement an if macro to check constants, make it nestable
/**
 * @brief Perform the first pass operations to a source file
 * 		AKA: Remove comments, define constants, evaluate preprocessor expressions & parse macros 
 * @param src 
 */
void firstPass(std::vector<std::string> *src) {
	inflvl++;
	debug_inform("Removing comments");
	for (unsigned int i = 0; i < src->size(); i++) {
		if ((*src)[i][0] == '#') {
			src->erase(src->begin() + i);
			i--;
		}
	}

	debug_inform("Reading constants");
	for (unsigned int i = 0; i < src->size(); i++) {
		if ((*src)[i][0] == '&') {
			std::string str = (*src)[i];
			str.erase(str.begin());
			str.erase(std::remove(str.begin(), str.end(), ' '), str.end());

			std::regex rx("^[a-zA-Z]+=(?:0b[01]+|0x[0-9a-fA-F]+|-?[0-9]+)$");
			std::smatch sm;
			if (!std::regex_match(str, sm, rx))
				errExit("Constant definition error", (*src)[i]);

			std::string name = getSubStrBAChar(str, '=', true);
			if (name == "LABEL" || name == "MACH" || name == "SIM")
				errExit("Constant named same as keyword", (*src)[i]);

			constants.push_back({name, (unsigned int)getHBDNum(getSubStrBAChar(str, '=', false).c_str())});
			src->erase(src->begin() + i);
			debug_log("Defined constant %s with value %u", constants[constants.size() - 1].key.c_str(), constants[constants.size() - 1].value);
			i--;
		}
	}

	debug_inform("Evaluating preprocessor expressions");
	for (unsigned int i = 0; i < src->size(); i++) {
		if (size_t p = (*src)[i].find(",("); p != std::string::npos) {
			inflvl++;
			size_t t = PreExpr::findClosing((*src)[i], p + 2);
			debug_log("Evaluating expression: %s", (*src)[i].substr(p, t - (p - 1)).c_str());

			//Replace constants
			inflvl++;
			std::string e = (*src)[i].substr(p + 2, t - (p + 2));
			std::string::const_iterator iter = e.cbegin();
			std::regex rx("(?:\\s|\\(|^)([a-zA-Z]+)(?:\\s|\\)|$)");
			std::smatch match;
			while (regex_search(iter, e.cend(), match, rx)) {
				if (std::string str = match[1].str(); constantExists(str)) {
					debug_log("Replacing constant in expression: %s", str.c_str());
					e.erase(match.position(1), str.length());
					e.insert(match.position(1), std::to_string(getConstant(str)));
					iter = e.cbegin();
				} else {
					iter = match.suffix().first;
				}
			}
			inflvl--;

			PreExpr expr(e);
			(*src)[i].erase(p, t - (p - 1));
			(*src)[i].insert(p, std::to_string(expr.eval()));
			debug_log("Expansion after evaluation: %s", (*src)[i].c_str());
			inflvl--;
			i--; //Reprocess line for more expressions
		}
	}

	debug_inform("Parsing macros");
	for (unsigned int i = 0; i < src->size(); i++) { //TODO: VA_ARGS for macros ($$) -> ($0$, $1$, $2$, ...) + ($|$ -> ,($0$ | $1$ | ...))
		if ((*src)[i][0] == '%') {
			inflvl++;
			std::string name = (*src)[i];
			name.erase(name.begin());
			bool containsArgs = false;
			std::vector<KVP<std::string, std::string>> args;
			debug_inform("Parsing macro %s", name.c_str());
			inflvl++;

			std::regex rx("^[A-Z_]+$");
			std::smatch sm;
			if (!std::regex_match(name, sm, rx)) {
				std::regex ry("^[A-Z_]+( \\$[a-z]+)+$");
				if (!std::regex_match(name, sm, ry)) {
					errExit("Macro definition error", (*src)[i]);
				} else {
					containsArgs = true;
					std::vector<std::string> arguments = cutString(name, ' ');
					if (arguments[0] == "LABEL" || arguments[0] == "MACH" || arguments[0] == "SIM")
						errExit("Macro named same as keyword", (*src)[i]);
					arguments.erase(arguments.begin());
					for (unsigned int j = 0; j < arguments.size(); j++) {
						arguments[j].erase(std::remove(arguments[j].begin(), arguments[j].end(), '$'), arguments[j].end());
						debug_log("With argument: %s", arguments[arguments.size() - 1].c_str());
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
			/*
			Some macros consist entirely of instructions that do not use
			any arguments, we can precompile those in order to not have to
			recompile them every time we use them.
			We can also precompile the parts of macros that do not directly
			interact with arguments
			*/

			if (containsArgs) {
				canPrecompileWhole = false;
			} else {
				for (unsigned int j = 0; j < macro.size(); j++) {
					if (macro[j].substr(0, 5) == "LABEL")
						canPrecompileWhole = false;
				}
			}

			debug_inform("Precompiling macro");
			if (canPrecompileWhole) {
				for (unsigned int j = 0; j < macro.size(); j++) {
					parseLine(macro[j]);
				}
				macros.push_back({name, bits, true, {}});
				bits.clear();
			} else {
				std::vector<std::string> instructions;
				for (unsigned int j = 0; j < macro.size(); j++) {
					macro[j].erase(std::remove(macro[j].begin(), macro[j].end(), ' '), macro[j].end());
					if (!(stringContains(macro[j], "LABEL") || stringContains(macro[j], "$") || (macro[j][0] == 'A' && macro[j][1] == '=' && !isdigit(macro[j][2])))) {
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
			inflvl--; inflvl--;
		}
	}
	inflvl--;
}

/**
 * @brief Open a file. That's it.
 * 
 * @param filename Filename
 * @param ret Whether if file *has* to be opened
 * @return std::fstream File stream to opened file
 */
std::fstream openFile(const std::string& filename, const bool ret = true) {
	std::fstream file;
	file.open(filename);
	if (!file.is_open() && ret) {
		printf("Could not open file %s.", filename.c_str());
		exit(1);
	}
	return file;
}

/**
 * @brief Read file, clear it of undesired characters and perform a firstPass
 * 
 * @param filename Filename
 * @param ret Whether if file *has* to be processed
 * @return std::vector<std::string> Processed lines
 */
std::vector<std::string> fileFirstPass(const std::string& filename, const bool ret = true) {
	std::fstream file = openFile(filename, ret);
	if (file.is_open()) {
		std::vector<std::string> data;
		std::string line;

		while (std::getline(file, line)) {  //read data from file object and put it into string.
			if (line.size()){
				line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
				data.push_back(line);
			} 
		}
		file.close();

		if (vectorContains(data, "\\")) {
			printf("Compiler reserrved character(\\) in file: %s.", filename.c_str());
			exit(1);
		}
		
		std::string lcrf = currfile;
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

/**
 * @brief Scan for labels and store them
 * 
 * @param src Vector of lines to be scanned
 */
void processLabels(std::vector<std::string> *src) {
	unsigned int premOff = 0;
	for (unsigned int i = 0; i < src->size(); i++) {
		for (unsigned int j = 0; j < macros.size(); j++) {
			if ((*src)[i] == macros[j].name)
				premOff += macros[j].instructions.size() - 1;
		}
		if ((*src)[i].substr(0, 5) == "LABEL") {
			labels.push_back({cutString((*src)[i], ' ')[1], i + premOff});
			src->erase(src->begin() + i);
			debug_log("Defined label %s at %u", labels[labels.size() - 1].key.c_str(), labels[labels.size() - 1].value);
			i--;
		}
	}
}

/**
 * @brief Recursively expands macros that couldn't be precompiled
 * 
 * @param src Source to work with
 */
void expandUncompiledMacros(std::vector<std::string> *src) {
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
					std::string work = (*src)[i].substr(cutString((*src)[i], ' ')[0].length());
					src->erase(src->begin() + i);

					std::regex rx(std::string("^( [0-9A-Za-z]+){") + std::to_string(macros[j].arguments.size()) + "}$");
					std::smatch sm;
					if (!std::regex_match(work, sm, rx))
						errExit("Macro missuse", (*src)[i]);

					work = work.substr(1);
					std::vector<std::string> arguments = cutString(work, ' ');
					Macro macro = macros[j];
					for (unsigned int k = 0; k < arguments.size(); k++) {
						macro.arguments[k].value = arguments[k];
					}

					std::vector<std::string> instructions = macro.instructions;
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

//TODO: Maybe create a vector of files included as to avoid include loops
//NOTE: @include <thisfile>.src must be fun
/**
 * @brief Expands an include line
 * 
 * @param src 
 */
void handleIncludes(std::vector<std::string> *src) {
	inflvl++;
	debug_inform("Including files");
	for (unsigned int i = 0; i < src->size(); i++) {
		if ((*src)[i][0] == '@') {
			std::string filename = (*src)[i].substr(1);
			src->erase(src->begin() + i);
			inflvl++; debug_inform("Including file %s", filename.c_str()); inflvl--;
			std::vector<std::string> include = fileFirstPass(filename);
			src->insert(std::begin(*src) + i, std::begin(include), std::end(include));
			i--;
		}
	}
	inflvl--;
}

/**
 * @brief Parse a vector of strings and convert them into a vector of bits
 * 		AKA: Handle include, Expand macros, process labels and parse code
 * 
 * @param src Vector of strings
 */
void parseSource(std::vector<std::string> src) {
	inflvl++;
	handleIncludes(&src);
	debug_inform("Expanding uncompiled macros");
	expandUncompiledMacros(&src);
	debug_inform("Processing labels");
	processLabels(&src);
	debug_inform("Parsing lines");
	for (unsigned int i = 0; i < src.size(); i++) {
		parseLine(src[i]);
		linen++;
	}

	if (useDec) {
		bits.push_back("49153");
	} else {
		bits.push_back("1100000000000001");
	}
	inflvl--;
}

int main(int argc, char **argv) {
	std::string filename;
	if (argc < 2 || argc > 3) {
		printf("Usage: %s [-d] <program.src>\n", argv[0]);
		return 1;
	} else if (argc == 2) {
		filename = argv[1];
	} else if (argc == 3) {
		if (std::string(argv[1]) == "-d")
			useDec = true;
		filename = argv[2];
	}
	
	debug_inform("Processing macros file");
	fileFirstPass("macros.src", false);

	debug_inform("First pass on source");
	std::vector<std::string> src;
	src = fileFirstPass(filename);

	debug_inform("Parsing source");
	currfile = filename;
	parseSource(src);

	std::ofstream code(getSubStrBAChar(std::string(filename), '.', true) + ".bit", std::ofstream::trunc);
	if (!code.is_open()) {
		printf("Could not open destination file %s.", (std::string(filename) + ".bit").c_str());
		return 1;
	}

	for (unsigned int i = 0; i < bits.size(); i++) {
		code << bits[i] << std::endl;
	}
	code.close();

	return 0;
}