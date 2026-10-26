// src/tools/calculator_eval.cpp — YOU IMPLEMENT (spec §4.6).
//
// calc_evaluate: evaluate a well-formed token list with two stacks
// (one for numbers, one for operators and '('). See spec §4.6 and the
// worked trace in Appendix C. std::stack or std::vector are both fine.

#include "tools/calculator.h"

// Returns the precedence of an arithmetic operator
int precedence(CalcToken::Kind op) {
    if (op == CalcToken::Kind::Times ||
        op == CalcToken::Kind::Divide) {
        return 2;
    }

    if (op == CalcToken::Kind::Plus ||
        op == CalcToken::Kind::Minus) {
        return 1;
    }

    return 0;
}

// Evaluates a list of calculator tokens
bool calc_evaluate(const std::vector<CalcToken>& tokens, double* out,
                   std::string* error) {

    // Stack for storing numbers
    std::vector<double> numbers;

    // Stack for storing operators
    std::vector<CalcToken::Kind> operators;

    // Applies the operator at the top of the operator stack
    auto apply_operator = [&]() -> bool {

        // Make sure there is an operator and two numbers
        if (operators.empty() || numbers.size() < 2) {
            *error = "Invalid expression";
            return false;
        }

        // Get the operator
        CalcToken::Kind op = operators.back();
        operators.pop_back();

        // Get the right operand
        double right = numbers.back();
        numbers.pop_back();

        // Get the left operand
        double left = numbers.back();
        numbers.pop_back();

        double result = 0.0;

        // Perform the correct arithmetic operation
        if (op == CalcToken::Kind::Plus) {
            result = left + right;
        }
        else if (op == CalcToken::Kind::Minus) {
            result = left - right;
        }
        else if (op == CalcToken::Kind::Times) {
            result = left * right;
        }
        else if (op == CalcToken::Kind::Divide) {

            // Prevent division by zero
            if (right == 0.0) {
                *error = "division by zero";
                return false;
            }

            result = left / right;
        }
        else {
            *error = "Invalid operator";
            return false;
        }

        // Store the calculated result
        numbers.push_back(result);

        return true;
    };

    // Process each token in the expression
    for (const auto& token : tokens) {

        // Store numbers in the number stack
        if (token.kind == CalcToken::Kind::Number) {
            numbers.push_back(token.value);
        }

        // Store opening parentheses
        else if (token.kind == CalcToken::Kind::LParen) {
            operators.push_back(token.kind);
        }

        // Evaluate everything inside closing parentheses
        else if (token.kind == CalcToken::Kind::RParen) {

            while (!operators.empty() &&
                   operators.back() != CalcToken::Kind::LParen) {

                if (!apply_operator()) {
                    return false;
                }
            }

            // Check for a matching opening parenthesis
            if (operators.empty()) {
                *error = "Mismatched parentheses";
                return false;
            }

            operators.pop_back();
        }

        // Process arithmetic operators
        else {

            // Apply operators with higher or equal precedence
            while (!operators.empty() &&
                   operators.back() != CalcToken::Kind::LParen &&
                   precedence(operators.back()) >= precedence(token.kind)) {

                if (!apply_operator()) {
                    return false;
                }
            }

            // Store the current operator
            operators.push_back(token.kind);
        }
    }

    // Apply any remaining operators
    while (!operators.empty()) {

        if (operators.back() == CalcToken::Kind::LParen) {
            *error = "Mismatched parentheses";
            return false;
        }

        if (!apply_operator()) {
            return false;
        }
    }

    // Exactly one number should remain
    if (numbers.size() != 1) {
        *error = "Invalid expression";
        return false;
    }

    // Store the final result
    *out = numbers.back();
    error->clear();

    return true;
}