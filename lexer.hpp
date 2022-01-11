#include <string>

enum Token;

class Lexer {
    private:
        std::string IdentifierStr; 
        double NumVal;
    public:
        std::string getIdentifierStr() {}

        double getNumVal() {}

        int getTok() {}
};