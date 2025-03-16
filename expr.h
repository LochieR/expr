/*
MIT License

Copyright (c) 2025 LochieR

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <regex>
#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <numbers>
#include <type_traits>
#include <unordered_map>

namespace expr {

    class AstNode;

    class Function
    {
    public:
        virtual std::string GetExpressionID() const = 0;
        virtual double Exec(double x) const = 0;
        virtual std::shared_ptr<AstNode> Differentiate(const std::string& respectTo, const std::shared_ptr<AstNode>& argument) const = 0;
        virtual std::shared_ptr<AstNode> Simplify(const std::shared_ptr<AstNode>& argument) const = 0;

        virtual std::string ToString() const = 0;
    };

#define FUNC_IMPL(expressionID, exec) \
    virtual std::string GetExpressionID() const override { return #expressionID; } \
    virtual double Exec(double x) const override exec \
    virtual std::shared_ptr<AstNode> Differentiate(const std::string& respectTo, const std::shared_ptr<AstNode>& argument) const override; \
    virtual std::shared_ptr<AstNode> Simplify(const std::shared_ptr<AstNode>& argument) const override; \
    virtual std::string ToString() const override { return GetExpressionID(); }

#define sameType(node, astType) dynamic_cast<astType*>(node.get())

    class SineFunction : public Function
    {
    public:
        FUNC_IMPL(sin, { return sin(x); });
    };

    class CosineFunction : public Function
    {
    public:
        FUNC_IMPL(cos, { return cos(x); });
    };

    class TangentFunction : public Function
    {
    public:
        FUNC_IMPL(tan, { return tan(x); });
    };

    class CotangentFunction : public Function
    {
    public:
        FUNC_IMPL(cot, { return cos(x) / sin(x); });
    };

    class SecantFunction : public Function
    {
    public:
        FUNC_IMPL(sec, { return 1 / cos(x); });
    };

    class CosecantFunction : public Function
    {
    public:
        FUNC_IMPL(csc, { return 1 / sin(x); });
    };

    class HyperbolicSineFunction : public Function
    {
    public:
        FUNC_IMPL(sinh, { return sinh(x); });
    };

    class HyperbolicCosineFunction : public Function
    {
    public:
        FUNC_IMPL(cosh, { return cosh(x); });
    };

    class HyperbolicTangentFunction : public Function
    {
    public:
        FUNC_IMPL(tanh, { return tanh(x); });
    };

    class HyperbolicCotangentFunction : public Function
    {
    public:
        FUNC_IMPL(coth, { return cosh(x) / sinh(x); });
    };

    class HyperbolicSecantFunction : public Function
    {
    public:
        FUNC_IMPL(sech, { return 1 / cosh(x); });
    };

    class HyperbolicCosecantFunction : public Function
    {
    public:
        FUNC_IMPL(csch, { return 1 / sinh(x); });
    };

    class Base10LogarithmFunction : public Function
    {
    public:
        FUNC_IMPL(log, { return log10(x); });
    };

    class NaturalLogarithmFunction : public Function
    {
    public:
        FUNC_IMPL(ln, { return log(x); });
    };

    class ExponentialFunction : public Function
    {
    public:
        FUNC_IMPL(exp, { return exp(x); });
    };

    class SquareRootFunction : public Function
    {
    public:
        FUNC_IMPL(sqrt, { return sqrt(x); });
    };

    class ModulusFunction : public Function
    {
    public:
        FUNC_IMPL(abs, { return abs(x); });
    };

    class AstNode
    {
    public:
        virtual ~AstNode() = default;

        virtual std::shared_ptr<AstNode> Differentiate(const std::string& respectTo) const = 0;
        virtual double Evaluate(const std::unordered_map<std::string, double>& variableValues) const = 0;
        virtual std::shared_ptr<AstNode> Simplify() const = 0;
        
        virtual std::string ToString() const = 0;
    };

#define AST_IMPL(diff, eval, simpl, str) \
    virtual std::shared_ptr<AstNode> Differentiate(const std::string& respectTo) const override diff \
    virtual double Evaluate(const std::unordered_map<std::string, double>& variableValues) const override eval \
    virtual std::shared_ptr<AstNode> Simplify() const override simpl \
    virtual std::string ToString() const override str

    class ErrorNode : public AstNode
    {
    public:
        ErrorNode(const std::string& message)
            : m_Message(message)
        {
        }
        ~ErrorNode() = default;

        AST_IMPL(
            { return std::make_shared<ErrorNode>(m_Message); },
            { return std::numeric_limits<double>::signaling_NaN(); },
            { return std::make_shared<ErrorNode>(m_Message); },
            { return m_Message; }
        );

        const std::string& GetMessage() const { return m_Message; }
    private:
        std::string m_Message;
    };

    class NumberNode : public AstNode
    {
    public:
        NumberNode(double value) : m_Value(value) {}
        ~NumberNode() = default;
        
        double GetValue() const { return m_Value; }

        AST_IMPL(
            { return std::make_shared<NumberNode>(0); },
            { return m_Value; },
            { return std::make_shared<NumberNode>(m_Value); },
            {
                std::ostringstream oss;
                oss.precision(15);
                oss << m_Value;
                return oss.str();
            }
        );
    private:
        double m_Value;
    };

    class VariableNode : public AstNode
    {
    public:
        VariableNode(const std::string& name) : m_Name(name) {}
        ~VariableNode() = default;

        const std::string& GetName() const { return m_Name; }

        AST_IMPL(
            ;,
            {
                auto it = variableValues.find(m_Name);
                
                if (it != variableValues.end())
                    return it->second;
                
                return std::numeric_limits<double>::signaling_NaN();
            },
            { return std::make_shared<VariableNode>(m_Name); },
            { return m_Name; }
        );
    private:
        std::string m_Name;
    };

    class ConstantNode : public AstNode
    {
    public:
        ConstantNode(const std::string& constantName);
        ~ConstantNode() = default;

        const std::string& GetName() const { return m_Name; }
        double GetValue() const { return m_Value; }

        AST_IMPL(
            { return std::make_shared<NumberNode>(0); },
            { return m_Value; },
            { return std::make_shared<ConstantNode>(m_Name); },
            { return m_Name; }
        );
    private:
        double m_Value;
        std::string m_Name;
    };

    class OperatorNode : public AstNode
    {
    public:
        OperatorNode(const std::string& op, const std::shared_ptr<AstNode>& left, const std::shared_ptr<AstNode>& right)
            : m_Operator(op), m_Left(left), m_Right(right) {}
        ~OperatorNode() = default;

        const std::string& GetOperator() const { return m_Operator; }
        const std::shared_ptr<AstNode>& GetLeft() const { return m_Left; }
        const std::shared_ptr<AstNode>& GetRight() const { return m_Right; }

        AST_IMPL(
            ;, // implemented in c++ file
            {
                double leftVal = m_Left->Evaluate(variableValues);
                double rightVal = m_Right->Evaluate(variableValues);

                if (m_Operator == "+")
                    return leftVal + rightVal;
                else if (m_Operator == "-")
                    return leftVal - rightVal;
                else if (m_Operator == "*")
                    return leftVal * rightVal;
                else if (m_Operator == "/")
                    return leftVal / rightVal;
                else if (m_Operator == "^")
                    return pow(leftVal, rightVal);
                else
                    return std::numeric_limits<double>::signaling_NaN();
            },
            ;,
            ;
        );
    private:
        std::string m_Operator;
        std::shared_ptr<AstNode> m_Left, m_Right;
    };

    class EqualsNode : public AstNode
    {
    public:
        EqualsNode(const std::shared_ptr<AstNode>& lhs, const std::shared_ptr<AstNode>& rhs)
            : m_Left(lhs), m_Right(rhs)
        {
        }
        ~EqualsNode() = default;

        AST_IMPL(
            {
                auto diffLeft = m_Left->Differentiate(respectTo);
                auto diffRight = m_Right->Differentiate(respectTo);

                if (ErrorNode* error = sameType(diffLeft, ErrorNode))
                    return diffLeft;
                if (ErrorNode* error = sameType(diffRight, ErrorNode))
                    return diffRight;

                return std::make_shared<EqualsNode>(diffLeft, diffRight);
            },
            { return std::numeric_limits<double>::signaling_NaN(); },
            {
                auto simplifiedLeft = m_Left->Simplify();
                auto simplifiedRight = m_Right->Simplify();

                if (ErrorNode* error = sameType(simplifiedLeft, ErrorNode))
                    return simplifiedLeft;
                if (ErrorNode* error = sameType(simplifiedRight, ErrorNode))
                    return simplifiedRight;

                return std::make_shared<EqualsNode>(simplifiedLeft, simplifiedRight); 
            },
            {
                if (ErrorNode* error = sameType(m_Left, ErrorNode))
                    return error->GetMessage();
                if (ErrorNode* error = sameType(m_Right, ErrorNode))
                    return error->GetMessage();

                return m_Left->ToString() + " = " + m_Right->ToString();
            }
        );

        const std::shared_ptr<AstNode>& GetLeft() const { return m_Left; }
        const std::shared_ptr<AstNode>& GetRight() const { return m_Right; }
    private:
        std::shared_ptr<AstNode> m_Left, m_Right;
    };

    class DifferentialNode : public AstNode
    {
    public:
        DifferentialNode(const std::string& variable, const std::string& respectTo, int order = 1)
            : m_Variable(variable), m_RespectTo(respectTo), m_Order(order)
        {
        }
        ~DifferentialNode() = default;

        AST_IMPL(
            {
                if (respectTo == m_RespectTo)
                    return std::make_shared<DifferentialNode>(m_Variable, m_RespectTo, m_Order + 1);
                else
                {
                    // d/dt (dy/dx) = d^2y/dx^2 * dx/dt
                    return std::make_shared<OperatorNode>(
                        "*", 
                        std::make_shared<DifferentialNode>(m_Variable, m_RespectTo, m_Order + 1), 
                        std::make_shared<DifferentialNode>(m_RespectTo, respectTo, 1)
                    );
                }
            },
            { return std::numeric_limits<double>::signaling_NaN(); },
            { return std::make_shared<DifferentialNode>(m_Variable, m_RespectTo, m_Order); },
            {
                if (m_Order == 1)
                    return "d" + m_Variable + "/d" + m_RespectTo;
                else
                {
                    return "d^" + std::to_string(m_Order) + m_Variable + "/d" + m_RespectTo + "^" + std::to_string(m_Order);
                }
            }
        );
    private:
        std::string m_Variable, m_RespectTo;
        int m_Order;
    };

    class FunctionNode : public AstNode
    {
    public:
        FunctionNode(const std::string& funcID, const std::shared_ptr<AstNode>& argument);
        ~FunctionNode() = default;

        const Function* GetFunction() const { return m_Function; }
        const std::shared_ptr<AstNode> GetArgument() const { return m_Argument; }

        AST_IMPL(
            {
                if (ErrorNode* error = sameType(m_Argument, ErrorNode))
                    return m_Argument;

                return m_Function->Differentiate(respectTo, m_Argument);
            },
            {
                double argumentVal = m_Argument->Evaluate(variableValues);

                return m_Function->Exec(argumentVal);
            },
            {
                if (ErrorNode* error = sameType(m_Argument, ErrorNode))
                    return m_Argument;

                return m_Function->Simplify(m_Argument);
            },
            {
                if (ErrorNode* error = sameType(m_Argument, ErrorNode))
                    return error->GetMessage();

                return m_Function->ToString() + "(" + m_Argument->ToString() + ")";
            }
        );
    private:
        Function* m_Function;
        std::shared_ptr<AstNode> m_Argument;
    };

    class Configuration
    {
    public:
        static void Init();
        static void Shutdown();

        static Function* GetFunction(const std::string& name);
        static double GetConstantValue(const std::string& name);
        static const std::unordered_map<std::string, Function*>& GetAllFunctions() { return s_Functions; }
        static const std::unordered_map<std::string, double>& GetAllConstants() { return s_Constants; }

        template<typename T>
        static std::enable_if_t<std::is_base_of<Function, T>::value> AddFunction(const std::string& expressionID)
        {
            if (s_Functions.find(expressionID) == s_Functions.end())
            {
                s_Functions[expressionID] = new T();
            }
        }

        static void AddConstant(const std::string& name, double value);
    private:
        inline static std::unordered_map<std::string, Function*> s_Functions;
        inline static std::unordered_map<std::string, double> s_Constants;
        inline static bool s_Init = false;
    };

    enum class TokenType : uint32_t
    {
        Number = 0,
        Operator,
        Variable,
        Constant,
        Function,
        Parenthesis,
        ModulusDelimiter,
        Equals,
        Unknown
    };

    struct Token
    {
        TokenType Type;
        std::string Value;

        Token(TokenType type, const std::string& value)
            : Type(type), Value(value)
        {
        }
    };

    class Tokenizer
    {
    public:
        static std::vector<Token> Tokenize(const std::string& input);
    private:
        static std::string GetFunctionIDs();
        static std::string GetConstantNames();

        static bool IsNegativeSign(int index, const std::vector<Token>& tokens, bool insideModulus);
    };

    class Parser
    {
    public:
        Parser(const std::vector<Token>& tokens);
        ~Parser();

        std::shared_ptr<AstNode> ParseExpression();
    private:
        std::shared_ptr<AstNode> ParseEquals();
        std::shared_ptr<AstNode> ParseAdditionSubtraction();
        std::shared_ptr<AstNode> ParseMultiplicationDivision();
        std::shared_ptr<AstNode> ParseExponentiation();
        std::shared_ptr<AstNode> ParsePrimary();

        const Token* Peek() const { return m_Position < m_Tokens.size() ? &m_Tokens[m_Position] : nullptr; }
        const Token* Consume() { return m_Position < m_Tokens.size() ? &m_Tokens[m_Position++] : nullptr; }
    private:
        const std::vector<Token>& m_Tokens;
        int m_Position;
    };

}

#ifndef EXPR_IMPLEMENTATION

namespace expr {

    #define implementation(className, diff, simpl) \
        std::shared_ptr<AstNode> className::Differentiate(const std::string& respectTo, const std::shared_ptr<AstNode>& argument) const diff \
        std::shared_ptr<AstNode> className::Simplify(const std::shared_ptr<AstNode>& argument) const simpl


    implementation(
        SineFunction,
        {
            return std::make_shared<OperatorNode>("*", argument->Differentiate(respectTo), std::make_shared<FunctionNode>("cos", argument));
        },
        {
            auto simplifiedArgument = argument->Simplify();

            if (NumberNode* number = sameType(simplifiedArgument, NumberNode))
            {
                if (number->GetValue() == 0)
                    return std::make_shared<NumberNode>(0);
            }

            return std::make_shared<FunctionNode>(GetExpressionID(), simplifiedArgument);
        }
    )

    implementation(
        CosineFunction,
        {
            return std::make_shared<OperatorNode>("*", std::make_shared<NumberNode>(-1), std::make_shared<OperatorNode>("*", argument->Differentiate(respectTo), std::make_shared<FunctionNode>("sin", argument)));
        },
        {
            auto simplifiedArgument = argument->Simplify();

            if (NumberNode* number = sameType(simplifiedArgument, NumberNode))
            {
                if (number->GetValue() == 0)
                    return std::make_shared<NumberNode>(1);
            }

            return std::make_shared<FunctionNode>(GetExpressionID(), simplifiedArgument);
        }
    )

    implementation(
        TangentFunction,
        {
            return std::make_shared<OperatorNode>("*", argument->Differentiate(respectTo), std::make_shared<OperatorNode>("^", std::make_shared<FunctionNode>("sec", argument), std::make_shared<NumberNode>(2)));
        },
        {
            auto simplifiedArgument = argument->Simplify();

            if (NumberNode* number = sameType(simplifiedArgument, NumberNode))
            {
                if (number->GetValue() == 0)
                    return std::make_shared<NumberNode>(0);
            }

            return std::make_shared<FunctionNode>(GetExpressionID(), simplifiedArgument);
        }
    )

    implementation(
        CotangentFunction,
        {
            return std::make_shared<OperatorNode>("*", std::make_shared<NumberNode>(-1), std::make_shared<OperatorNode>("*", argument->Differentiate(respectTo), std::make_shared<OperatorNode>("^", std::make_shared<FunctionNode>("csc", argument), std::make_shared<NumberNode>(2))));
        },
        {
            auto simplifiedArgument = argument->Simplify();

            return std::make_shared<FunctionNode>(GetExpressionID(), simplifiedArgument);
        }
    )

    implementation(
        SecantFunction,
        {
            return std::make_shared<OperatorNode>("*", argument->Differentiate(respectTo), std::make_shared<OperatorNode>("*", std::make_shared<FunctionNode>("tan", argument), std::make_shared<FunctionNode>("sec", argument)));
        },
        {
            auto simplifiedArgument = argument->Simplify();

            if (NumberNode* number = sameType(simplifiedArgument, NumberNode))
            {
                if (number->GetValue() == 0)
                    return std::make_shared<NumberNode>(1);
            }

            return std::make_shared<FunctionNode>(GetExpressionID(), simplifiedArgument);
        }
    )

    implementation(
        CosecantFunction,
        {
            return std::make_shared<OperatorNode>("*", std::make_shared<NumberNode>(-1), std::make_shared<OperatorNode>("*", argument->Differentiate(respectTo), std::make_shared<OperatorNode>("*", std::make_shared<FunctionNode>("cot", argument), std::make_shared<FunctionNode>("csc", argument))));
        },
        {
            auto simplifiedArgument = argument->Simplify();

            return std::make_shared<FunctionNode>(GetExpressionID(), simplifiedArgument);
        }
    )

    implementation(
        HyperbolicSineFunction,
        {
            return std::make_shared<OperatorNode>("*", argument->Differentiate(respectTo), std::make_shared<FunctionNode>("cosh", argument));
        },
        {
            auto simplifiedArgument = argument->Simplify();

            if (NumberNode* number = sameType(simplifiedArgument, NumberNode))
            {
                if (number->GetValue() == 0)
                    return std::make_shared<NumberNode>(0);
            }

            return std::make_shared<FunctionNode>(GetExpressionID(), simplifiedArgument);
        }
    )

    implementation(
        HyperbolicCosineFunction,
        {
            return std::make_shared<OperatorNode>("*", argument->Differentiate(respectTo), std::make_shared<FunctionNode>("sinh", argument));
        },
        {
            auto simplifiedArgument = argument->Simplify();

            if (NumberNode* number = sameType(simplifiedArgument, NumberNode))
            {
                if (number->GetValue() == 0)
                    return std::make_shared<NumberNode>(1);
            }

            return std::make_shared<FunctionNode>(GetExpressionID(), simplifiedArgument);
        }
    )

    implementation(
        HyperbolicTangentFunction,
        {
            return std::make_shared<OperatorNode>("*", argument->Differentiate(respectTo), std::make_shared<OperatorNode>("^", std::make_shared<FunctionNode>("sech", argument), std::make_shared<NumberNode>(2)));
        },
        {
            auto simplifiedArgument = argument->Simplify();

            if (NumberNode* number = sameType(simplifiedArgument, NumberNode))
            {
                if (number->GetValue() == 0)
                    return std::make_shared<NumberNode>(0);
            }

            return std::make_shared<FunctionNode>(GetExpressionID(), simplifiedArgument);
        }
    )

    implementation(
        HyperbolicCotangentFunction,
        {
            return std::make_shared<OperatorNode>("*", std::make_shared<NumberNode>(-1), std::make_shared<OperatorNode>("*", argument->Differentiate(respectTo), std::make_shared<OperatorNode>("^", std::make_shared<FunctionNode>("csch", argument), std::make_shared<NumberNode>(2))));
        },
        {
            auto simplifiedArgument = argument->Simplify();

            return std::make_shared<FunctionNode>(GetExpressionID(), simplifiedArgument);
        }
    )

    implementation(
        HyperbolicSecantFunction,
        {
            return std::make_shared<OperatorNode>("*", std::make_shared<NumberNode>(-1), std::make_shared<OperatorNode>("*", argument->Differentiate(respectTo), std::make_shared<OperatorNode>("*", std::make_shared<FunctionNode>("tanh", argument), std::make_shared<FunctionNode>("sech", argument))));
        },
        {
            auto simplifiedArgument = argument->Simplify();

            if (NumberNode* number = sameType(simplifiedArgument, NumberNode))
            {
                if (number->GetValue() == 0)
                    return std::make_shared<NumberNode>(1);
            }

            return std::make_shared<FunctionNode>(GetExpressionID(), simplifiedArgument);
        }
    )

    implementation(
        HyperbolicCosecantFunction,
        {
            return std::make_shared<OperatorNode>("*", std::make_shared<NumberNode>(-1), std::make_shared<OperatorNode>("*", argument->Differentiate(respectTo), std::make_shared<OperatorNode>("*", std::make_shared<FunctionNode>("coth", argument), std::make_shared<FunctionNode>("csch", argument))));
        },
        {
            auto simplifiedArgument = argument->Simplify();

            return std::make_shared<FunctionNode>(GetExpressionID(), simplifiedArgument);
        }
    )

    implementation(
        Base10LogarithmFunction,
        {
            return std::make_shared<OperatorNode>("/", argument->Differentiate(respectTo), std::make_shared<OperatorNode>("*", std::make_shared<FunctionNode>("ln", std::make_shared<NumberNode>(10)), argument));
        },
        {
            auto simplifiedArgument = argument->Simplify();

            if (NumberNode* number = sameType(simplifiedArgument, NumberNode))
            {
                if (number->GetValue() == 1)
                    return std::make_shared<NumberNode>(0);
                if (number->GetValue() == 10)
                    return std::make_shared<NumberNode>(1);
            }

            return std::make_shared<FunctionNode>(GetExpressionID(), simplifiedArgument);
        }
    )

    implementation(
        NaturalLogarithmFunction,
        {
            return std::make_shared<OperatorNode>("/", argument->Differentiate(respectTo), argument);
        },
        {
            auto simplifiedArgument = argument->Simplify();

            if (NumberNode* number = sameType(simplifiedArgument, NumberNode))
            {
                if (number->GetValue() == 1)
                    return std::make_shared<NumberNode>(0);
                if (number->GetValue() == std::numbers::e)
                    return std::make_shared<NumberNode>(1);
            }
            if (ConstantNode* number = sameType(simplifiedArgument, ConstantNode))
            {
                if (number->GetName() == "e")
                    return std::make_shared<NumberNode>(1);
            }

            return std::make_shared<FunctionNode>(GetExpressionID(), simplifiedArgument);
        }
    )

    implementation(
        ExponentialFunction,
        {
            return std::make_shared<OperatorNode>("*", argument->Differentiate(respectTo), std::make_shared<FunctionNode>("exp", argument));
        },
        {
            auto simplifiedArgument = argument->Simplify();
            
            if (NumberNode* number = sameType(simplifiedArgument, NumberNode))
            {
                if (number->GetValue() == 1)
                    return std::make_shared<ConstantNode>("e");
                if (number->GetValue() == 0)
                    return std::make_shared<NumberNode>(1);
            }

            return std::make_shared<FunctionNode>(GetExpressionID(), simplifiedArgument);
        }
    )

    implementation(
        SquareRootFunction,
        {
            return std::make_shared<OperatorNode>("/", argument->Differentiate(respectTo), std::make_shared<OperatorNode>("*", std::make_shared<NumberNode>(2), std::make_shared<FunctionNode>("sqrt", argument)));
        },
        {
            auto simplifiedArgument = argument->Simplify();
            
            if (NumberNode* number = sameType(simplifiedArgument, NumberNode))
            {
                double result = sqrt(number->GetValue());

                if ((double)(int)result == result)
                {
                    // number is a square number
                    return std::make_shared<NumberNode>(result);
                }
            }

            return std::make_shared<FunctionNode>(GetExpressionID(), simplifiedArgument);
        }
    )

    implementation(
        ModulusFunction,
        {
            return std::make_shared<OperatorNode>("/", std::make_shared<OperatorNode>("*", argument, argument->Differentiate(respectTo)), std::make_shared<FunctionNode>("abs", argument));
        },
        {
            auto simplifiedArgument = argument->Simplify();
            
            if (NumberNode* number = sameType(simplifiedArgument, NumberNode))
            {
                if (number->GetValue() < 0)
                    return std::make_shared<NumberNode>(-number->GetValue());
                else
                    return std::make_shared<NumberNode>(number->GetValue());
            }

            return std::make_shared<FunctionNode>(GetExpressionID(), simplifiedArgument);
        }
    )

    std::shared_ptr<AstNode> VariableNode::Differentiate(const std::string& respectTo) const
    {
        if (m_Name == respectTo)
            return std::make_shared<NumberNode>(1);
        
        return std::make_shared<DifferentialNode>(m_Name, respectTo, 1);
    }

    std::shared_ptr<AstNode> OperatorNode::Differentiate(const std::string& respectTo) const
    {
        auto diffLeft = m_Left->Differentiate(respectTo);
        auto diffRight = m_Right->Differentiate(respectTo);

        if (ErrorNode* error = sameType(diffLeft, ErrorNode))
            return diffLeft;
        if (ErrorNode* error = sameType(diffRight, ErrorNode))
            return diffRight;

        if (m_Operator == "+")
            return std::make_shared<OperatorNode>("+", m_Left->Differentiate(respectTo), m_Right->Differentiate(respectTo));
        else if (m_Operator == "-")
            return std::make_shared<OperatorNode>("-", m_Left->Differentiate(respectTo), m_Right->Differentiate(respectTo));
        else if (m_Operator == "*")
        {
            return std::make_shared<OperatorNode>("+",
                std::make_shared<OperatorNode>("*", m_Left->Differentiate(respectTo), m_Right),
                std::make_shared<OperatorNode>("*", m_Left, m_Right->Differentiate(respectTo)));
        }
        else if (m_Operator == "/")
        {
            // d/dx a/f(x) where a is a constant
            // = -a*f'(x)/f(x)^2

            if (NumberNode* numerator = sameType(m_Left, NumberNode))
            {
                return std::make_shared<OperatorNode>("*", std::make_shared<NumberNode>(-1),
                    std::make_shared<OperatorNode>("*", std::make_shared<NumberNode>(numerator->GetValue()),
                        std::make_shared<OperatorNode>("/", m_Right->Differentiate(respectTo),
                            std::make_shared<OperatorNode>("^", m_Right, std::make_shared<NumberNode>(2)))));
            }
            if (ConstantNode* numerator = sameType(m_Left, ConstantNode))
            {
                return std::make_shared<OperatorNode>("*", std::make_shared<NumberNode>(-1),
                    std::make_shared<OperatorNode>("*", std::make_shared<ConstantNode>(numerator->GetName()),
                        std::make_shared<OperatorNode>("/", m_Right->Differentiate(respectTo),
                            std::make_shared<OperatorNode>("^", m_Right, std::make_shared<NumberNode>(2)))));
            }

            // d/dx f(x)/a where a is a constant
            // = f'(x)/a
            if (NumberNode* denominator = sameType(m_Right, NumberNode))
                return std::make_shared<OperatorNode>("/", m_Left->Differentiate(respectTo), std::make_shared<NumberNode>(denominator->GetValue()));
            if (ConstantNode* denominator = sameType(m_Right, ConstantNode))
                return std::make_shared<OperatorNode>("/", m_Left->Differentiate(respectTo), std::make_shared<ConstantNode>(denominator->GetName()));

            // quotient rule
            return std::make_shared<OperatorNode>("/",
                std::make_shared<OperatorNode>("-",
                    std::make_shared<OperatorNode>("*", m_Right, m_Left->Differentiate(respectTo)),
                    std::make_shared<OperatorNode>("*", m_Left, m_Right->Differentiate(respectTo))),
                std::make_shared<OperatorNode>("^", m_Right, std::make_shared<NumberNode>(2.0)));
        }
        else if (m_Operator == "^")
        {
            // if base is a variable, and power is a number
            if (VariableNode* varNode = sameType(m_Left, VariableNode))
            {
                if (NumberNode* numberNode = sameType(m_Right, NumberNode))
                {
                    if (numberNode->GetValue() == 1)
                        return std::make_shared<NumberNode>(1);
                    if (numberNode->GetValue() == 0)
                        return std::make_shared<NumberNode>(0);

                    // power rule
                    return std::make_shared<OperatorNode>("*", std::make_shared<NumberNode>(numberNode->GetValue()), std::make_shared<OperatorNode>("^", std::make_shared<VariableNode>(varNode->GetName()), std::make_shared<NumberNode>(numberNode->GetValue() - 1)));
                }
                if (ConstantNode* constantNode = sameType(m_Right, ConstantNode))
                {
                    // power rule
                    // example: d/dx x^e = e*x^(e-1)
                    return std::make_shared<OperatorNode>("*", std::make_shared<ConstantNode>(constantNode->GetName()), std::make_shared<OperatorNode>("^", std::make_shared<VariableNode>(varNode->GetName()), std::make_shared<OperatorNode>("-", std::make_shared<ConstantNode>(constantNode->GetName()), std::make_shared<NumberNode>(1))));
                }
            }

            // if base is a constant and power is a function
            if (NumberNode* numberNode = sameType(m_Left, NumberNode))
            {
                // d/dx a^f(x) = ln(a)*a^f(x)*f'(x)

                return std::make_shared<OperatorNode>("*", std::make_shared<FunctionNode>("ln", std::make_shared<NumberNode>(numberNode->GetValue())),
                    std::make_shared<OperatorNode>("*", std::make_shared<OperatorNode>("^", m_Left, m_Right),
                    m_Right->Differentiate(respectTo)));
            }
            if (ConstantNode* constant = sameType(m_Left, ConstantNode))
            {
                // d/dx a^f(x) = ln(a)*a^f(x)*f'(x)

                return std::make_shared<OperatorNode>("*", std::make_shared<FunctionNode>("ln", std::make_shared<ConstantNode>(constant->GetName())),
                    std::make_shared<OperatorNode>("*", std::make_shared<OperatorNode>("^", m_Left, m_Right),
                    m_Right->Differentiate(respectTo)));
            }

            // cover all cases
            // d/dx (f^g) = f^g * (g' * ln(f) + g * f'/f)

            auto baseDerivative = m_Left->Differentiate(respectTo);
            auto exponentDerivative = m_Right->Differentiate(respectTo);

            // f' / f
            auto baseFraction = std::make_shared<OperatorNode>("/", baseDerivative, m_Left);
            
            // ln(f)
            auto lnBase = std::make_shared<FunctionNode>("ln", m_Left);

            auto firstTerm = std::make_shared<OperatorNode>("*", m_Right, baseFraction);
            auto secondTerm = std::make_shared<OperatorNode>("*", lnBase, exponentDerivative);

            auto result = std::make_shared<OperatorNode>("+", firstTerm, secondTerm);
            return std::make_shared<OperatorNode>("*", std::make_shared<OperatorNode>(*this), result);
        }
        else
        {
            return std::make_shared<ErrorNode>("Unknown operator " + m_Operator);
        }
    }

    std::shared_ptr<AstNode> OperatorNode::Simplify() const
    {
        auto simplifiedLeft = m_Left->Simplify();
        auto simplifiedRight = m_Right->Simplify();

        if (ErrorNode* error = sameType(simplifiedLeft, ErrorNode))
            return simplifiedLeft;
        if (ErrorNode* error = sameType(simplifiedRight, ErrorNode))
            return simplifiedRight;

        if (m_Operator == "+")
        {
            if (NumberNode* leftNum = sameType(simplifiedLeft, NumberNode))
            {
                if (leftNum->GetValue() == 0)
                    return simplifiedRight;
                
                if (NumberNode* rightNum = sameType(simplifiedRight, NumberNode))
                    return std::make_shared<NumberNode>(leftNum->GetValue() + rightNum->GetValue());
            }
            if (NumberNode* rightNum = sameType(simplifiedRight, NumberNode))
            {
                if (rightNum->GetValue() == 0)
                    return simplifiedLeft;
            }
        }
        else if (m_Operator == "-")
        {
            if (NumberNode* leftNum = sameType(simplifiedLeft, NumberNode))
            {
                if (leftNum->GetValue() == 0)
                    return std::make_shared<OperatorNode>("*", std::make_shared<NumberNode>(-1), simplifiedRight);
                
                if (NumberNode* rightNum = sameType(simplifiedRight, NumberNode))
                    return std::make_shared<NumberNode>(leftNum->GetValue() - rightNum->GetValue());
            }
            if (NumberNode* rightNum = sameType(simplifiedRight, NumberNode))
            {
                if (rightNum->GetValue() == 0)
                    return simplifiedLeft;
            }
        }
        else if (m_Operator == "*")
        {
            if (NumberNode* leftNum = sameType(simplifiedLeft, NumberNode))
            {
                if (leftNum->GetValue() == 1)
                    return simplifiedRight;
                if (leftNum->GetValue() == 0)
                    return simplifiedLeft;
            }
            if (NumberNode* rightNum = sameType(simplifiedRight, NumberNode))
            {
                if (rightNum->GetValue() == 1)
                    return simplifiedLeft;
                if (rightNum->GetValue() == 0)
                    return simplifiedRight;
            }

            if (ConstantNode* lhs = sameType(simplifiedLeft, ConstantNode))
            {
                if (ConstantNode* rhs = sameType(simplifiedRight, ConstantNode))
                {
                    if (lhs->GetName() == rhs->GetName())
                        return std::make_shared<OperatorNode>("^", simplifiedLeft, std::make_shared<NumberNode>(2.0));
                }
            }

            if (VariableNode* lhs = sameType(simplifiedLeft, VariableNode))
            {
                if (VariableNode* rhs = sameType(simplifiedRight, VariableNode))
                {
                    if (lhs->GetName() == rhs->GetName())
                        return std::make_shared<OperatorNode>("^", simplifiedLeft, std::make_shared<NumberNode>(2.0));
                }
            }

            if (OperatorNode* rightOp = sameType(simplifiedRight, OperatorNode))
            {
                if (rightOp->GetOperator() == "+" || rightOp->GetOperator() == "-")
                {
                    if (OperatorNode* leftOp = sameType(simplifiedLeft, OperatorNode))
                    {
                        if (leftOp->GetOperator() == "+" || leftOp->GetOperator() == "-")
                        {
                            // (a + b)(c + d)
                            // = ac + ad + bc + bd
                            // (a + b)(c - d)
                            // = ac - ad + bc - bd
                            // (a - b)(c + d)
                            // = ac - bc + ad - bd
                            // (a - b)(c - d)
                            // = ac - ad + bd - bc

                            const auto& leftOperator = leftOp->GetOperator();
                            const auto& rightOperator = rightOp->GetOperator();

                            std::string first, second, third;

                            if (leftOperator == "+" && rightOperator == "+")
                            {
                                first = second = third = "+";

                                return std::make_shared<OperatorNode>(second,
                                    std::make_shared<OperatorNode>(first,
                                        std::make_shared<OperatorNode>("*",
                                            leftOp->m_Left,
                                            rightOp->m_Left
                                        ),
                                        std::make_shared<OperatorNode>("*",
                                            leftOp->m_Left,
                                            rightOp->m_Right
                                        )
                                    ),
                                    std::make_shared<OperatorNode>(third,
                                        std::make_shared<OperatorNode>("*",
                                            leftOp->m_Right,
                                            rightOp->m_Left
                                        ),
                                        std::make_shared<OperatorNode>("*",
                                            leftOp->m_Right,
                                            rightOp->m_Right
                                        )
                                    )
                                );
                            }
                            else if (leftOperator == "+" && rightOperator == "-")
                            {
                                first = third = "-";
                                second = "+";

                                return std::make_shared<OperatorNode>(second,
                                    std::make_shared<OperatorNode>(first,
                                        std::make_shared<OperatorNode>("*",
                                            leftOp->m_Left,
                                            rightOp->m_Left
                                        ),
                                        std::make_shared<OperatorNode>("*",
                                            leftOp->m_Left,
                                            rightOp->m_Right
                                        )
                                    ),
                                    std::make_shared<OperatorNode>(third,
                                        std::make_shared<OperatorNode>("*",
                                            leftOp->m_Right,
                                            rightOp->m_Left
                                        ),
                                        std::make_shared<OperatorNode>("*",
                                            leftOp->m_Right,
                                            rightOp->m_Right
                                        )
                                    )
                                );
                            }
                            else if (leftOperator == "-" && rightOperator == "+")
                            {
                                first = third = "-";
                                second = "+";

                                return std::make_shared<OperatorNode>(second,
                                    std::make_shared<OperatorNode>(first,
                                        std::make_shared<OperatorNode>("*",
                                            leftOp->m_Left,
                                            rightOp->m_Left
                                        ),
                                        std::make_shared<OperatorNode>("*",
                                            leftOp->m_Right,
                                            rightOp->m_Left
                                        )
                                    ),
                                    std::make_shared<OperatorNode>(third,
                                        std::make_shared<OperatorNode>("*",
                                            leftOp->m_Left,
                                            rightOp->m_Right
                                        ),
                                        std::make_shared<OperatorNode>("*",
                                            leftOp->m_Right,
                                            rightOp->m_Right
                                        )
                                    )
                                );
                            }
                            else if (leftOperator == "-" && rightOperator == "-")
                            {
                                first = third = "-";
                                second = "+";

                                return std::make_shared<OperatorNode>(second,
                                    std::make_shared<OperatorNode>(first,
                                        std::make_shared<OperatorNode>("*",
                                            leftOp->m_Left,
                                            rightOp->m_Left
                                        ),
                                        std::make_shared<OperatorNode>("*",
                                            leftOp->m_Left,
                                            rightOp->m_Right
                                        )
                                    ),
                                    std::make_shared<OperatorNode>(third,
                                        std::make_shared<OperatorNode>("*",
                                            leftOp->m_Right,
                                            rightOp->m_Right
                                        ),
                                        std::make_shared<OperatorNode>("*",
                                            leftOp->m_Right,
                                            rightOp->m_Left
                                        )
                                    )
                                );
                            }
                        }
                    }
                    if (NumberNode* leftNum = sameType(simplifiedLeft, NumberNode))
                    {
                        // a(b + c)
                        // = ab + ac
                        // a(b - c)
                        // = ab - ac

                        return std::make_shared<OperatorNode>(rightOp->GetOperator(),
                            std::make_shared<OperatorNode>("*",
                                simplifiedLeft,
                                rightOp->m_Left
                            ),
                            std::make_shared<OperatorNode>("*",
                                simplifiedLeft,
                                rightOp->m_Right
                            )
                        );
                    }
                    if (ConstantNode* leftNum = sameType(simplifiedLeft, ConstantNode))
                    {
                        // a(b + c)
                        // = ab + ac
                        // a(b - c)
                        // = ab - ac

                        return std::make_shared<OperatorNode>(rightOp->GetOperator(),
                            std::make_shared<OperatorNode>("*",
                                simplifiedLeft,
                                rightOp->m_Left
                            ),
                            std::make_shared<OperatorNode>("*",
                                simplifiedLeft,
                                rightOp->m_Right
                            )
                        );
                    }
                    if (FunctionNode* leftFunc = sameType(simplifiedLeft, FunctionNode))
                    {
                        // f(x)(b + c)
                        // = f(x)b + f(x)c
                        // f(x)(b - c)
                        // = f(x)b - f(x)c

                        return std::make_shared<OperatorNode>(rightOp->GetOperator(),
                            std::make_shared<OperatorNode>("*",
                                simplifiedLeft,
                                rightOp->m_Left
                            ),
                            std::make_shared<OperatorNode>("*",
                                simplifiedLeft,
                                rightOp->m_Right
                            )
                        );
                    }
                }
            }

            if (OperatorNode* leftOp = sameType(simplifiedLeft, OperatorNode))
            {
                if (leftOp->GetOperator() == "+" || leftOp->GetOperator() == "-")
                {
                    if (NumberNode* rightNum = sameType(simplifiedRight, NumberNode))
                    {
                        // (b + c)a
                        // = ab + ac
                        // (b - c)a
                        // = ab - ac

                        return std::make_shared<OperatorNode>(leftOp->GetOperator(),
                            std::make_shared<OperatorNode>("*",
                                simplifiedRight,
                                leftOp->m_Left
                            ),
                            std::make_shared<OperatorNode>("*",
                                simplifiedRight,
                                leftOp->m_Right
                            )
                        );
                    }
                    if (ConstantNode* rightNum = sameType(simplifiedRight, ConstantNode))
                    {
                        // (b + c)a
                        // = ab + ac
                        // (b - c)a
                        // = ab - ac

                        return std::make_shared<OperatorNode>(leftOp->GetOperator(),
                            std::make_shared<OperatorNode>("*",
                                simplifiedRight,
                                leftOp->m_Left
                            ),
                            std::make_shared<OperatorNode>("*",
                                simplifiedRight,
                                leftOp->m_Right
                            )
                        );
                    }
                    if (FunctionNode* rightFunc = sameType(simplifiedRight, FunctionNode))
                    {
                        // (b + c)f(x)
                        // = f(x)b + f(x)c
                        // (b - c)f(x)
                        // = f(x)b - f(x)c

                        return std::make_shared<OperatorNode>(leftOp->GetOperator(),
                            std::make_shared<OperatorNode>("*",
                                simplifiedRight,
                                leftOp->m_Left
                            ),
                            std::make_shared<OperatorNode>("*",
                                simplifiedRight,
                                leftOp->m_Right
                            )
                        );
                    }
                }
            }
        }
        else if (m_Operator == "/")
        {
            if (NumberNode* rightNum = sameType(simplifiedRight, NumberNode))
            {
                if (rightNum->GetValue() == 1)
                    return simplifiedLeft;
            }
            if (NumberNode* leftNum = sameType(simplifiedLeft, NumberNode))
            {
                if (leftNum->GetValue() == 0)
                    return simplifiedLeft;
            }
        }
        else if (m_Operator == "^")
        {
            if (NumberNode* leftNum = sameType(simplifiedLeft, NumberNode))
            {
                if (leftNum->GetValue() == 0)
                {
                    if (!sameType(simplifiedRight, NumberNode))
                        return std::make_shared<NumberNode>(0);
                    else if (NumberNode* rightNum = sameType(simplifiedRight, NumberNode))
                    {
                        if (rightNum->GetValue() != 0)
                            return std::make_shared<NumberNode>(0);
                    }
                }
                if (leftNum->GetValue() == 1)
                    return std::make_shared<NumberNode>(1);
            }
            if (NumberNode* rightNum = sameType(simplifiedRight, NumberNode))
            {
                if (rightNum->GetValue() == 1)
                    return simplifiedLeft;
                if (rightNum->GetValue() == 0)
                    return std::make_shared<NumberNode>(1);
            }
        }

        return std::make_shared<OperatorNode>(m_Operator, simplifiedLeft, simplifiedRight);
    }

    std::string OperatorNode::ToString() const
    {
        if (ErrorNode* error = sameType(m_Left, ErrorNode))
            return error->GetMessage();
        if (ErrorNode* error = sameType(m_Right, ErrorNode))
            return error->GetMessage();

        if (m_Operator == "*")
        {
            if (OperatorNode* lhs = sameType(m_Left, OperatorNode))
            {
                if (OperatorNode* rhs = sameType(m_Right, OperatorNode))
                {
                    return "(" + lhs->ToString() + ")(" + rhs->ToString() + ")";
                }

                // example: (x + y) * z = z(x + y)
                return m_Right->ToString() + "(" + m_Left->ToString() + ")";
            }
            if (OperatorNode* rhs = sameType(m_Right, OperatorNode))
            {
                return m_Left->ToString() + "(" + m_Right->ToString() + ")";
            }

            // x * y -> xy
            return m_Left->ToString() + m_Right->ToString();
        }
        else if (m_Operator == "/")
        {
            return "(" + m_Left->ToString() + " " + m_Operator + " " + m_Right->ToString() + ")";
        }
        else if (m_Operator == "+" || m_Operator == "-")
        {
            return m_Left->ToString() + " " + m_Operator + " " + m_Right->ToString();
        }
        else if (m_Operator == "^")
        {
            return m_Left->ToString() + m_Operator + m_Right->ToString();
        }

        return "(" + m_Left->ToString() + " " + m_Operator + " " + m_Right->ToString() + ")";
    }

    FunctionNode::FunctionNode(const std::string& funcID, const std::shared_ptr<AstNode>& argument)
        : m_Function(nullptr), m_Argument(argument)
    {
        m_Function = Configuration::GetFunction(funcID);
        if (m_Function == nullptr)
            m_Argument = std::make_shared<ErrorNode>("Could not find function " + funcID);
    }

    ConstantNode::ConstantNode(const std::string& constantName)
        : m_Name(constantName), m_Value(Configuration::GetConstantValue(constantName))
    {
    }

    void Configuration::Init()
    {
        if (s_Init)
            return;

        AddFunction<SineFunction>("sin");
        AddFunction<CosineFunction>("cos");
        AddFunction<TangentFunction>("tan");
        AddFunction<CotangentFunction>("cot");
        AddFunction<SecantFunction>("sec");
        AddFunction<CosecantFunction>("csc");
        AddFunction<HyperbolicSineFunction>("sinh");
        AddFunction<HyperbolicCosineFunction>("cosh");
        AddFunction<HyperbolicTangentFunction>("tanh");
        AddFunction<HyperbolicCotangentFunction>("coth");
        AddFunction<HyperbolicSecantFunction>("sech");
        AddFunction<HyperbolicCosecantFunction>("csch");
        AddFunction<Base10LogarithmFunction>("log");
        AddFunction<NaturalLogarithmFunction>("ln");
        AddFunction<SquareRootFunction>("sqrt");
        AddFunction<ModulusFunction>("abs");

        AddConstant("e", std::numbers::e);
        AddConstant("pi", std::numbers::pi);

        s_Init = true;
    }

    void Configuration::Shutdown()
    {
        for (const auto& [name, func] : s_Functions)
        {
            delete func;
        }

        s_Functions.clear();
        s_Init = false;
    }

    Function* Configuration::GetFunction(const std::string& name)
    {
        auto it = s_Functions.find(name);

        if (it != s_Functions.end())
            return it->second;
        
        return nullptr;
    }

    double Configuration::GetConstantValue(const std::string &name)
    {
        auto it = s_Constants.find(name);

        if (it != s_Constants.end())
            return it->second;
        
        return std::numeric_limits<double>::signaling_NaN();
    }

    void Configuration::AddConstant(const std::string &name, double value)
    {
        s_Constants[name] = value;
    }

    std::vector<Token> Tokenizer::Tokenize(const std::string& input)
    {
        std::string numberPattern = R"(-?\d+(\.\d+)?)";
        std::string operatorPattern = R"([\+\-\*/\^=])";
        std::string variablePattern = R"([a-zA-Z]+)";
        std::string parenthesisPattern = R"([()])";
        std::string modulusDelimiterPattern = R"(\|)";
        std::string functionPattern = "\\b(" + GetFunctionIDs() + ")\\b";
        std::string constantPattern = "\\b(" + GetConstantNames() + ")\\b";

        std::string tokenPattern = functionPattern + "|" + constantPattern + "|" + numberPattern + "|" + operatorPattern + "|" + variablePattern + "|" + parenthesisPattern + "|" + modulusDelimiterPattern;

        std::regex regex(tokenPattern);
        std::sregex_iterator it(input.begin(), input.end(), regex);
        std::sregex_iterator end;

        std::vector<Token> tokens;

        bool insideModulus = false;

        int i = 0;
        while (it != end)
        {
            std::string value = it->str();
            TokenType type = TokenType::Unknown;

            if (std::regex_match(value, std::regex(functionPattern)))
                type = TokenType::Function;
            else if (std::regex_match(value, std::regex(constantPattern)))
                type = TokenType::Constant;
            else if (std::regex_match(value, std::regex(numberPattern)))
                type = TokenType::Number;
            else if (std::regex_match(value, std::regex(operatorPattern)))
            {
                if (value == "=")
                    type = TokenType::Equals;
                else
                    type = TokenType::Operator;
            }
            else if (std::regex_match(value, std::regex(variablePattern)))
                type = TokenType::Variable;
            else if (std::regex_match(value, std::regex(parenthesisPattern)))
                type = TokenType::Parenthesis;
            else if (std::regex_match(value, std::regex(parenthesisPattern)))
            {
                type = TokenType::ModulusDelimiter;
                insideModulus = !insideModulus;
            }

            if (type == TokenType::Operator && value == "-" && IsNegativeSign(i, tokens, insideModulus))
            {
                ++it;
                if (it != end)
                {
                    std::string nextValue = it->str();
                    tokens.emplace_back(TokenType::Number, value + nextValue);
                }
            }
            else
            {
                tokens.emplace_back(type, value);
            }

            i++;
            ++it;
        }

        return tokens;
    }

    std::string Tokenizer::GetFunctionIDs()
    {
        const auto& allFunctions = Configuration::GetAllFunctions();
        std::string functionString;

        for (const auto& [name, function] : allFunctions)
        {
            functionString += name + "|";
        }
        functionString.pop_back(); // remove trailing |

        return functionString;
    }

    std::string Tokenizer::GetConstantNames()
    {
        const auto& allConstants = Configuration::GetAllConstants();
        std::string constantString;

        for (const auto& [name, value] : allConstants)
            constantString += name + "|";
        constantString.pop_back();

        return constantString;
    }

    bool Tokenizer::IsNegativeSign(int index, const std::vector<Token> &tokens, bool insideModulus)
    {
        if (index == 0)
            return true;
        
        const Token& previousToken = tokens[tokens.size() - 1];
        return previousToken.Type == TokenType::Operator
            || (previousToken.Type == TokenType::Parenthesis && previousToken.Value == "(")
            || (previousToken.Type == TokenType::ModulusDelimiter && insideModulus);
    }

    Parser::Parser(const std::vector<Token>& tokens)
        : m_Tokens(tokens), m_Position(0)
    {
    }

    Parser::~Parser()
    {
    }

    std::shared_ptr<AstNode> Parser::ParseExpression()
    {
        return ParseEquals();
    }

    std::shared_ptr<AstNode> Parser::ParseEquals()
    {
        auto left = ParseAdditionSubtraction();
        if (ErrorNode* error = sameType(left, ErrorNode))
            return left;

        while (true)
        {
            const Token* token = Peek();
            if (!token)
                break;

            if (token->Value != "=")
                break;
            
            Consume();
            auto right = ParseAdditionSubtraction();
            if (ErrorNode* error = sameType(right, ErrorNode))
                return right;

            left = std::make_shared<EqualsNode>(left, right);
        }

        return left;
    }

    std::shared_ptr<AstNode> Parser::ParseAdditionSubtraction()
    {
        auto left = ParseMultiplicationDivision();
        if (ErrorNode* error = sameType(left, ErrorNode))
            return left;

        while (true)
        {
            const Token* token = Peek();
            if (!token)
                break;
            
            if (token->Value != "+" && token->Value != "-")
                break;
            
            Consume();
            auto right = ParseMultiplicationDivision();
            if (ErrorNode* error = sameType(right, ErrorNode))
                return right;

            left = std::make_shared<OperatorNode>(token->Value, left, right);
        }

        return left;
    }

    std::shared_ptr<AstNode> Parser::ParseMultiplicationDivision()
    {
        auto left = ParseExponentiation();
        if (ErrorNode* error = sameType(left, ErrorNode))
            return left;

        while (true)
        {
            const Token* token = Peek();
            if (!token)
                break;
            
            if (token->Value != "*" && token->Value != "/")
                break;
            
            Consume();
            auto right = ParseExponentiation();
            if (ErrorNode* error = sameType(right, ErrorNode))
                return right;

            left = std::make_shared<OperatorNode>(token->Value, left, right);
        }

        return left;
    }

    std::shared_ptr<AstNode> Parser::ParseExponentiation()
    {
        auto left = ParsePrimary();
        if (ErrorNode* error = sameType(left, ErrorNode))
            return left;

        while (true)
        {
            const Token* token = Peek();
            if (!token)
                break;
            
            if (token->Value != "^")
                break;
            
            Consume();
            auto right = ParsePrimary();
            if (ErrorNode* error = sameType(right, ErrorNode))
                return right;

            left = std::make_shared<OperatorNode>(token->Value, left, right);
        }

        return left;
    }

    std::shared_ptr<AstNode> Parser::ParsePrimary()
    {
        const Token* token = Peek();

        if (!token)
            return std::make_shared<ErrorNode>("Unexpected end of tokens");
        
        if (token->Type == TokenType::Number)
        {
            Consume();
            return std::make_shared<NumberNode>(std::stod(token->Value));
        }

        if (token->Type == TokenType::Constant)
        {
            Consume();
            return std::make_shared<ConstantNode>(token->Value);
        }

        if (token->Type == TokenType::Variable)
        {
            Consume();
            return std::make_shared<VariableNode>(token->Value);
        }

        if (token->Type == TokenType::Function)
        {
            Consume();
            if (!Peek())
                return std::make_shared<ErrorNode>("expected '(' after function");
            if (Peek()->Value != "(")
                return std::make_shared<ErrorNode>("expected '(' after function");
            
            Consume();
            auto argument = ParseExpression();

            if (!Peek())
                return std::make_shared<ErrorNode>("expected ')' after function argument");
            if (Peek()->Value != ")")
                return std::make_shared<ErrorNode>("expected ')' after function argument");
            
            Consume();

            return std::make_shared<FunctionNode>(token->Value, argument);
        }

        if (token->Type == TokenType::ModulusDelimiter)
        {
            Consume();
            auto argument = ParseExpression();

            if (!Peek())
                return std::make_shared<ErrorNode>("expected '|' to close modulus expression");
            if (Peek()->Value != "|")
                return std::make_shared<ErrorNode>("expected '|' to close modulus expression");
            
            Consume();

            return std::make_shared<FunctionNode>("abs", argument);
        }

        if (token->Type == TokenType::Parenthesis && token->Value == "(")
        {
            Consume();
            auto expression = ParseExpression();
            if (!Peek())
                return std::make_shared<ErrorNode>("expected ')'");
            if (Peek()->Value != ")")
                return std::make_shared<ErrorNode>("expected ')'");

            Consume();

            return expression;
        }

        return std::make_shared<ErrorNode>("unexpected token " + token->Value + " (type = " + std::to_string((uint32_t)token->Type) + ")");
    }

}

#endif
