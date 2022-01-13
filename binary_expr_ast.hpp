#include <string>
#include <vector>
#include "expr_ast.hpp"

class BinaryExprAST : public ExprAST {
public:
  BinaryExprAST(char op, std::unique_ptr<ExprAST> LHS,
                std::unique_ptr<ExprAST> RHS) {}
};
