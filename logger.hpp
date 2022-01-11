#include <string>
#include "expr_ast.hpp"
#include "prototype_ast.hpp"

class Logger {
    public:
        std::unique_ptr<ExprAST> LogError(const char *Str);
        std::unique_ptr<PrototypeAST> LogErrorP(const char *Str);
};