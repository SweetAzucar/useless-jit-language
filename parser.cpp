#include <string>
#include <vector>
#include "expr_ast.hpp"
#include "prototype_ast.hpp"
#include "number_expr_ast.hpp"
#include "lexer.hpp"
#include "logger.hpp"
#include "variable_expr_ast.hpp"
#include "call_expr_ast.hpp"

class Parser {
private:
    Lexer lexer;
    int curTok;
public:
    Parser(Lexer lexer);

    int getNextToken() {
        return this->curTok = this->lexer.getTok();
    };

    std::unique_ptr<ExprAST> ParseNumberExpr() {
        /*
        Expects to be called when the current token is a tok_number token. 
        Takes the current number value, creates a NumberExprAST node, 
        advances the lexer to the next token, and finally returns.
        */
        auto Result = std::make_unique<NumberExprAST>(lexer.getNumVal());
        this->getNextToken(); // consume the number
        return std::move(Result);
    };
    
    std::unique_ptr<ExprAST> ParseParenExpr() {     
        // parenexpr ::= '(' expression ')'
        this->getNextToken(); // eat (.
        auto V = ParseExpression();
        if (!V)
            return nullptr;

        if (this->curTok != ')')
            return Logger::LogError("expected ')'");
        this->getNextToken(); // eat ).
        return V;
    }

    std::unique_ptr<ExprAST> ParseIdentifierExpr() {
        /// identifierexpr
        ///   ::= identifier
        ///   ::= identifier '(' expression* ')'
        std::string IdName = this->lexer->getIdentifierStr();

        this->getNextToken();  // eat identifier.

        if (this->curTok != '(') // Simple variable ref.
        return std::make_unique<VariableExprAST>(IdName);

        // Call.
        this->getNextToken();  // eat (
        std::vector<std::unique_ptr<ExprAST>> Args;
        if (this->curTok != ')') {
        while (1) {
            if (auto Arg = ParseExpression())
            Args.push_back(std::move(Arg));
            else
            return nullptr;

            if (this->curTok == ')')
            break;

            if (this->curTok != ',')
            return Logger::LogError("Expected ')' or ',' in argument list");
            this->getNextToken();
        }
        }

        // Eat the ')'.
        this->getNextToken();

        return std::make_unique<CallExprAST>(IdName, std::move(Args));
    }

    std::unique_ptr<ExprAST> ParsePrimary() {
        switch (this->curTok) {
        default:
            return Logger::LogError("unknown token when expecting an expression");
        case tok_identifier:
            return ParseIdentifierExpr();
        case tok_number:
            return ParseNumberExpr();
        case '(':
            return ParseParenExpr();
    }
};