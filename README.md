# expr

expr is a maths expression parser for C++ 20 which generates an abstract syntax tree which can be symbolically differentiated and evaluated.

To use, include expr.h. In one of your C++ files you must define `EXPR_IMPLEMENTATION` so that the implementation of the library is compiled.

There is minimal simplification, so after differentiation expressions may look complicated.

```c++
#define EXPR_IMPLEMENTATION
#include "expr.h"

#include <string>
#include <iostream>

int main(int argc, char** argv)
{
    expr::Configuration::Init();

    std::string expression;
    std::cout << "Enter math expression: ";
    std::getline(std::cin, expression);

    std::vector<expr::Token> tokens = expr::Tokenizer::Tokenize(expression);
    
    expr::Parser parser(tokens);
    std::shared_ptr<expr::AstNode> ast = parser.ParseExpression();

    std::cout << ast->ToString() << std::endl;

    auto diff = ast->Differentiate("x")->Simplify();
    std::cout << diff->ToString() << std::endl;

    auto diff2 = diff->Differentiate("x")->Simplify();
    std::cout << diff2->ToString() << std::endl;

    std::unordered_map<std::string, double> variables;
    variables["x"] = 12.46;
    std::cout << diff2->Evaluate(variables) << std::endl;

    expr::Configuration::Shutdown();
}
```
