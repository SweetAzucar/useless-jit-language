#include <string>
#include "expr_ast.hpp"

class VariableExprAST : public ExprAST {
public:
  VariableExprAST(const std::string &Name) {}
};