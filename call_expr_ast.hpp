#include <string>
#include <vector>
#include "expr_ast.hpp"

/// CallExprAST - Expression class for function calls.
class CallExprAST : public ExprAST {

public:
  CallExprAST(const std::string &Callee,
              std::vector<std::unique_ptr<ExprAST>> Args) {}
};