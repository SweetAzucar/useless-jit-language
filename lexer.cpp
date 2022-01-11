#include <string>
#include "token.hpp"
class Lexer {
    private:
        std::string IdentifierStr; // Filled in if tok_identifier
        double NumVal;             // Filled in if tok_number

    public:
        std::string getIdentifierStr() {
            return this->IdentifierStr;
        }

        double getNumVal() {
            return this->NumVal;
        }

        int getTok() {
            int LastChar = ' ';

            // Skip any whitespace.
            while (isspace(LastChar)) 
                LastChar = getchar();

            if (isalpha(LastChar)) {
                IdentifierStr = LastChar;
                while (isalnum((LastChar = getchar())))
                    IdentifierStr += LastChar;

                // Not using switch cause it would mean having to convert
                // string into int.
                if (IdentifierStr == "def")
                    return Token::tok_def;
                if (IdentifierStr == "extern")
                    return Token::tok_extern;
                return tok_identifier;
            }
            
            if (isdigit(LastChar) || LastChar == '.') {   // Number: [0-9.]+
                std::string NumStr;
                int DotCounter;
                do {
                    NumStr += LastChar;
                    LastChar = getchar();
                    if (LastChar == '.' && DotCounter >= 1) {
                        return tok_num_parsing_err;
                    } else {
                        DotCounter++;
                    }    
                } while (isdigit(LastChar) || LastChar == '.');

                NumVal = strtod(NumStr.c_str(), 0);
                return tok_number;
            }

            if (LastChar == '#') {
                // Comment until end of line.
                do
                    LastChar = getchar();
                while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

                if (LastChar != EOF)
                    return this->getTok();
            }

            if (LastChar == EOF)
                return tok_eof;

            // If non of the cases above, just 
            // return the character as its ascii value.
            int ThisChar = LastChar;
            LastChar = getchar();
            return ThisChar;  
        }
};
