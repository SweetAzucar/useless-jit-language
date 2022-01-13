#include <string>
#include "prototype_ast.hpp"
#include "expr_ast.hpp"

/// FunctionAST - This class represents a function definition itself.
class FunctionAST {
public:
  FunctionAST(std::unique_ptr<PrototypeAST> Proto,
              std::unique_ptr<ExprAST> Body){}
};