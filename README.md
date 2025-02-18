# expr

expr is a maths expression parser which generates an abstract syntax tree which can be symbolically differentiated and evaluated.

To use, include expr.h. In one of your C++ files you must define `EXPR_IMPLEMENTATION` so that the implementation of the library is compiled.

There is minimal simplification, so after differentiation expressions may look complicated.

```c++
#define EXPR_IMPLEMENTATION
#include "expr.h"

int main()
{
    expr::Configuration::Init();

    std::string expression = "4*sin(x^2) - (2*x)/cos(x)";

    std::vector<expr::Token> tokens = expr::Tokenizer::Tokenize(expression);

    expr::Parser parser(tokens);
    std::shared_ptr<AstNode> ast = parser.ParseExpression();

    std::cout << ast->ToString() << std::endl;
}
```

See samples for more examples.
