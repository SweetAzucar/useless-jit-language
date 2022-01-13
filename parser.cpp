#include <string>
#include <vector>
#include <map>
#include "expr_ast.hpp"
#include "prototype_ast.hpp"
#include "number_expr_ast.hpp"
#include "lexer.hpp"
#include "logger.hpp"
#include "variable_expr_ast.hpp"
#include "call_expr_ast.hpp"
#include "binary_expr_ast.hpp"
#include "function_ast.hpp"
#include "token.hpp"

class Parser
{
private:
    Lexer lexer;
    int curTok;
    std::map<char, int> BinopPrecedence;

public:
    Parser(Lexer lexer);

    int getNextToken()
    {
        return this->curTok = this->lexer.getTok();
    };

    std::unique_ptr<ExprAST> ParseNumberExpr()
    {
        /*
        Expects to be called when the current token is a tok_number token. 
        Takes the current number value, creates a NumberExprAST node, 
        advances the lexer to the next token, and finally returns.
        */
        auto Result = std::make_unique<NumberExprAST>(lexer.getNumVal());
        this->getNextToken(); // consume the number
        return std::move(Result);
    };

    std::unique_ptr<ExprAST> ParseParenExpr()
    {
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

    std::unique_ptr<ExprAST> ParseIdentifierExpr()
    {
        /// identifierexpr
        ///   ::= identifier
        ///   ::= identifier '(' expression* ')'
        std::string IdName = this->lexer.getIdentifierStr();

        this->getNextToken(); // eat identifier.

        if (this->curTok != '(') // Simple variable ref.
            return std::make_unique<VariableExprAST>(IdName);

        // Call.
        this->getNextToken(); // eat (
        std::vector<std::unique_ptr<ExprAST>> Args;
        if (this->curTok != ')')
        {
            while (1)
            {
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

    std::unique_ptr<ExprAST> ParsePrimary()
    {
        switch (this->curTok)
        {
        default:
            return Logger::LogError("unknown token when expecting an expression");
        case (int)Token::tok_identifier:
            return ParseIdentifierExpr();
        case (int)Token::tok_number:
            return ParseNumberExpr();
        case '(':
            return ParseParenExpr();
        }
    }

    std::unique_ptr<ExprAST> ParseExpression()
    {
        auto LHS = this->ParsePrimary();

        if (!LHS)
            return nullptr;

        return this->ParseBinOpRHS(0, std::move(LHS));
    }

    int GetTokPrecedence()
    {
        if (!isascii(this->curTok))
            return -1;

        // Make sure it's a declared binop.
        int TokPrec = BinopPrecedence[this->curTok];
        if (TokPrec <= 0)
            return -1;
        return TokPrec;
    }

    std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec,
                                           std::unique_ptr<ExprAST> LHS)
    {
        // If this is a binop, find its precedence.
        while (1)
        {
            int TokPrec = GetTokPrecedence();

            // If this is a binop that binds at least as tightly as the current binop,
            // consume it, otherwise we are done.
            if (TokPrec < ExprPrec)
                return LHS;
            // Okay, we know this is a binop.
            int BinOp = this->curTok;
            this->getNextToken(); // eat binop

            // Parse the primary expression after the binary operator.
            auto RHS = ParsePrimary();
            if (!RHS)
                return nullptr;

            // If BinOp binds less tightly with RHS than the operator after RHS, let
            // the pending operator take RHS as its LHS.
            int NextPrec = GetTokPrecedence();
            if (TokPrec < NextPrec)
            {
                RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));
                if (!RHS)
                    return nullptr;
            }
            // Merge LHS/RHS.
            LHS = std::make_unique<BinaryExprAST>(BinOp, std::move(LHS),
                                                  std::move(RHS));
        }
    }

    std::unique_ptr<PrototypeAST> ParsePrototype()
    {
        if (this->curTok != (int)Token::tok_identifier)
            return Logger::LogErrorP("Expected function name in prototype");

        std::string FnName = this->lexer.getIdentifierStr();
        getNextToken();

        if (this->curTok != '(')
            return Logger::LogErrorP("Expected '(' in prototype");

        // Read the list of argument names.
        std::vector<std::string> ArgNames;
        while (getNextToken() == (int)Token::tok_identifier)
            ArgNames.push_back(this->lexer.getIdentifierStr());
        if (this->curTok != ')')
            return Logger::LogErrorP("Expected ')' in prototype");

        // success.
        getNextToken(); // eat ')'.

        return std::make_unique<PrototypeAST>(FnName, std::move(ArgNames));
    }

    std::unique_ptr<FunctionAST> ParseDefinition()
    {
        getNextToken(); // eat def.
        auto Proto = ParsePrototype();
        if (!Proto)
            return nullptr;

        if (auto E = ParseExpression())
            return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
        return nullptr;
    }

    std::unique_ptr<PrototypeAST> ParseExtern()
    {
        getNextToken(); // eat extern.
        return ParsePrototype();
    }

    std::unique_ptr<FunctionAST> ParseTopLevelExpr()
    {
        if (auto E = ParseExpression())
        {
            // Make an anonymous proto.
            auto Proto = std::make_unique<PrototypeAST>("", std::vector<std::string>());
            return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
        }
        return nullptr;
    }

    void HandleDefinition()
    {
        if (this->ParseDefinition())
        {
            fprintf(stderr, "Parsed a function definition.\n");
        }
        else
        {
            // Skip token for error recovery.
            this->getNextToken();
        }
    }

    void HandleExtern()
    {
        if (this->ParseExtern())
        {
            fprintf(stderr, "Parsed an extern\n");
        }
        else
        {
            // Skip token for error recovery.
            this->getNextToken();
        }
    }

    void HandleTopLevelExpression()
    {
        // Evaluate a top-level expression into an anonymous function.
        if (this->ParseTopLevelExpr())
        {
            fprintf(stderr, "Parsed a top-level expr\n");
        }
        else
        {
            // Skip token for error recovery.
            this->getNextToken();
        }
    }

    void MainLoop()
    {
        while (1)
        {
            fprintf(stderr, "ready> ");
            switch (this->curTok)
            {
            case (int)Token::tok_eof:
                return;
            case ';': // ignore top-level semicolons.
                this->getNextToken();
                break;
            case (int)Token::tok_def:
                this->HandleDefinition();
                break;
            case (int)Token::tok_extern:
                this->HandleExtern();
                break;
            default:
                this->HandleTopLevelExpression();
                break;
            }
        }
    }
};