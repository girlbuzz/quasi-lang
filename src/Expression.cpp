#include "Expression.h"

#include <cmath>
#include <stdexcept>

//=============================================================================
// Constructors and Destructors
//=============================================================================

Expression::Expression() {}
Expression::Expression(double scalar) : m_type(Lexicon::Type::SCALAR), m_scalar(scalar) {}
Expression::Expression(Op op) : m_type(Lexicon::Type::OPERATOR), m_op(op) {}
Expression::Expression(const std::string& ident) : m_type(Lexicon::Type::IDENTIFIER), m_ident(ident) {}

Expression::Expression(const Lexicon& lex) {
    this->m_type = lex.type();
    switch (lex.type()) {
        case Lexicon::Type::SCALAR: this->m_scalar = lex.scalar();
        break;
        case Lexicon::Type::OPERATOR: this->m_op = lex.op();
        break;
        case Lexicon::Type::IDENTIFIER: this->m_ident = lex.ident();
        break;
    }
}

Expression::~Expression() {
    if (left != nullptr) delete left;
    if (right != nullptr) delete right;
}

//=============================================================================
// Getters for union members
//=============================================================================

double Expression::scalar() const {
    return std::get<double>(m_scalar);
}

Op Expression::op() const {
    return this->m_type == Lexicon::Type::OPERATOR ? std::get<Op>(m_op) : Op::NONE;
}

const std::string& Expression::ident() const {
    return std::get<std::string>(m_ident);
}

//=============================================================================
// Public Functions
//=============================================================================

int Expression::precedence() const {
    switch (op()) {
        case Op::OPAREN: return -5;
        case Op::EQU: return -4;
        case Op::ADD: case Op::SUB: return -3;
        case Op::MUL: case Op::DIV: return -2;
        case Op::EXP: return -1;
        default: return 0;
    }
}

double Expression::evaluate(std::unordered_map<std::string, double>& variables) const {
    switch (op()) {
        case Op::ADD: {
            if (this->left == nullptr)
                return +this->right->evaluate(variables);
            
            return this->left->evaluate(variables) + this->right->evaluate(variables);
        }
        case Op::SUB: {
            if (this->left == nullptr)
                return -this->right->evaluate(variables);
            
            return this->left->evaluate(variables) - this->right->evaluate(variables);
        }
        case Op::MUL: return this->left->evaluate(variables) * this->right->evaluate(variables);
        case Op::DIV: return this->left->evaluate(variables) / this->right->evaluate(variables);
        case Op::EXP: return std::pow(this->left->evaluate(variables), this->right->evaluate(variables));
        case Op::EQU: {
            double value = this->right->evaluate(variables);
            variables[this->left->ident()] = value;
            return value;
        }
        case Op::OPAREN: return this->left->evaluate(variables);
    }

    switch (this->m_type) {
        case Lexicon::Type::SCALAR: return scalar();
        case Lexicon::Type::IDENTIFIER:
            try {
                return variables.at(ident());
            } catch (std::out_of_range& e) {
                throw ParseException("variable does not exist");
            }
    }

    throw ParseException("invalid parse tree");
}

Expression* Expression::parse(const std::vector<Lexicon>& lex) {
    if (lex.size() == 0) throw ParseException("expected input");

    Expression* left = new Expression(lex[0]);
    size_t pos_op = 1;

    switch (left->op()) {
        case Op::OPAREN: {
            size_t depth = 1;

            for (int i = 1; i < lex.size() && depth > 0; i++, pos_op++) {
                switch (lex[i].op()) {
                    case Op::OPAREN: depth++; break;
                    case Op::CPAREN: depth--; break;
                }
            }

            if (depth != 0)
                throw ParseException("expected a ')' to match");
            
            left->left = parse(std::vector<Lexicon>(lex.begin() + 1, lex.begin() + pos_op - 1));
        }
        break;
        case Op::SUB: case Op::ADD: {
            if (lex.size() < 2) throw ParseException("unary operator expected identifier or scalar");

            left->right = new Expression(lex[1]);
            pos_op += 1;
        }
        break;
        case Op::NONE: break;
        default: throw ParseException("unexpected operator, expected identifier or scalar");
    }

    if (pos_op >= lex.size())
        return left;

    Expression *current = new Expression(lex[pos_op]);

    if (current->m_type != Lexicon::Type::OPERATOR) {
        throw ParseException("expected operator");
    }

    current->left = left;

    Expression* next = parse(std::vector<Lexicon>(lex.begin() + pos_op + 1, lex.end()));

    if (next->precedence() < current->precedence()) {
        current->right = next->left;
        next->left = current;
        current = next;
    } else {
        current->right = next;
    }

    return current;
}
