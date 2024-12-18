#ifndef OPCODE_HPP
#define OPCODE_HPP

#include <cstdio>
#include <string>

inline void replaceAll(std::string& source, const std::string& from, const std::string& to)
{
    std::string newString;
    newString.reserve(source.length());  // avoids a few memory allocations

    std::string::size_type lastPos = 0;
    std::string::size_type findPos;

    while(std::string::npos != (findPos = source.find(from, lastPos)))
    {
        newString.append(source, lastPos, findPos - lastPos);
        newString += to;
        lastPos = findPos + from.length();
    }

    // Care for the rest after last occurrence
    newString += source.substr(lastPos);

    source.swap(newString);
}

class OpCode {
public:
    virtual ~OpCode() = default;
    virtual std::string genNasm() = 0;
};

class Label final : public OpCode {
public:
    explicit Label(const std::string& name) {
        this->name = name;
    }

    std::string genNasm() override {
        std::string out = name;
        out.append(":");

        return out;
    }

private:
    std::string name;
};

class Push final : public OpCode {
public:
    explicit Push(const std::string& reg) {
        this->reg = reg;
    }

    std::string genNasm() override {
        std::string out = "\tpush ";
        out.append(reg);
        return out;
    }

private:
    std::string reg;
};

class Pop final : public OpCode {
public:
    explicit Pop(const std::string& reg) {
        this->reg = reg;
    }

    std::string genNasm() override {
        std::string out = "\tpop ";
        out.append(reg);
        return out;
    }

private:
    std::string reg;
};

class Move final : public OpCode {
public:
    Move(const std::string& first, const std::string& second) {
        this->first = first;
        this->second = second;
    }
    
    std::string genNasm() override {
        std::string out = "\tmov ";
        out.append(first);
        out.append(", ");
        out.append(second);
        return out;
    }

private:
    std::string first;
    std::string second;
};

class LoadEffectiveAddr final : public OpCode {
public:
    LoadEffectiveAddr(const std::string& first, const std::string& second) {
        this->first = first;
        this->second = second;
    }

    std::string genNasm() override {
        std::string out = "\tlea ";
        out.append(first);
        out.append(", ");
        out.append(second);
        return out;
    }

private:
    std::string first;
    std::string second;
};

class Jump final : public OpCode
{
public:
    Jump(const std::string& type, const std::string& label) {
        this->type = type;
        this->label = label;
    }

    std::string genNasm() override
    {
        std::string out = "\t";
        out.append(type);
        out.append(" ");
        out.append(label);
        return out;
    }

private:
    std::string type;
    std::string label;
};

class Compare final : public OpCode
{
public:
    Compare(const std::string& first, const std::string& second) {
        this->first = first;
        this->second = second;
    }

    std::string genNasm() override
    {
        std::string out = "\tcmp ";
        out.append(first);
        out.append(", ");
        out.append(second);
        return out;
    }

private:
    std::string first;
    std::string second;
};

enum class BinaryOperator {
    PLUS, MINUS, MUL, DIV, MOD, EQUALS, NEQUALS, LESS, GREATER, LEQUALS, GEQUALS, BIT_OR, BIT_AND
};

class Add final : public OpCode {
public:
    Add(const std::string& first, const std::string& second, bool sign = false) {
        this->first = first;
        this->second = second;
        this->sign = sign;
    }

    std::string genNasm() override {
        std::string out = "\tadd ";
        out.append(first);
        out.append(", ");
        out.append(second);
        return out;
    }

private:
    std::string first;
    std::string second;
    bool sign;
};

class Sub final : public OpCode {
public:
    Sub(const std::string& first, const std::string& second, bool sign = false) {
        this->first = first;
        this->second = second;
        this->sign = sign;
    }

    std::string genNasm() override {
        std::string out = "\tsub ";
        out.append(first);
        out.append(", ");
        out.append(second);
        return out;
    }

private:
    std::string first;
    std::string second;
    bool sign;
};

class Multiply final : public OpCode {
public:
    Multiply(const std::string& first, const std::string& second, bool sign = false) {
        this->first = first;
        this->second = second;
        this->sign = sign;
    }

    std::string genNasm() override {
        std::string out = sign ? "\timul " : "\tmul ";
        out.append(first);
        out.append(", ");
        out.append(second);
        return out;
    }

private:
    std::string first;
    std::string second;
    bool sign;
};

class Div final : public OpCode {
public:
    Div(const std::string& first, bool sign = false) {
        this->first = first;
        this->sign = sign;
    }

    std::string genNasm() override {
        std::string out = sign ? "\tidiv " : "\tdiv ";
        out.append(first);
        return out;
    }

private:
    std::string first;
    bool sign;
};

class OR final : public OpCode {
public:
    OR(const std::string& first, const std::string& second) {
        this->first = first;
        this->second = second;
    }

    std::string genNasm() override
    {
        std::string out = "\tor ";
        out.append(first);
        out.append(", ");
        out.append(second);
        return out;
    }

private:
    std::string first;
    std::string second;
};

class AND final : public OpCode {
public:
    AND(const std::string& first, const std::string& second) {
        this->first = first;
        this->second = second;
    }

    std::string genNasm() override
    {
        std::string out = "\tor ";
        out.append(first);
        out.append(", ");
        out.append(second);
        return out;
    }

private:
    std::string first;
    std::string second;
};

class XOR final : public OpCode
{
public:
    XOR(const std::string& first, const std::string& second) {
        this->first = first;
        this->second = second;
    }

    std::string genNasm() override
    {
        std::string out = "\txor ";
        out.append(first);
        out.append(", ");
        out.append(second);
        return out;
    }

private:
    std::string first;
    std::string second;
};

class Comparison final : public OpCode
{
public:

    Comparison(const std::string& first, const std::string& second, const std::string& cmp1, const std::string& cmp2, const std::string& reg, BinaryOperator op) {
        this->first = first;
        this->second = second;
        this->op = op;
        this->cmp1 = cmp1;
        this->cmp2 = cmp2;
        this->reg = reg;
    }

    std::string genNasm() override
    {
        std::string out = "\tmov ";
        out.append(cmp1);
        out.append(", 0\n");
        out.append("\tmov ");
        out.append(cmp2);
        out.append(", 1\n");
        out.append("\tcmp ");
        out.append(first);
        out.append(", ");
        out.append(second);
        out.append("\n\tcmov");

        switch (op) {
        case BinaryOperator::PLUS:
        case BinaryOperator::MINUS:
        case BinaryOperator::MUL:
        case BinaryOperator::DIV:
        case BinaryOperator::BIT_OR:
        case BinaryOperator::BIT_AND:
            break;
        case BinaryOperator::EQUALS:
            out.append("e");
            break;
        case BinaryOperator::NEQUALS:
            out.append("ne");
            break;
        case BinaryOperator::LESS:
            out.append("l");
            break;
        case BinaryOperator::GREATER:
            out.append("g");
            break;
        case BinaryOperator::LEQUALS:
            out.append("le");
            break;
        case BinaryOperator::GEQUALS:
            out.append("ge");
            break;
        case BinaryOperator::MOD:
            break;
        }

        out.append(" ");
        out.append(cmp1);
        out.append(", ");
        out.append(cmp2);
        out.append("\n\tmov ");
        out.append(first);
        out.append(", ");
        out.append(reg);
        return out;
    }

private:
    std::string first;
    std::string second;
    std::string cmp1;
    std::string cmp2;
    std::string reg;
    BinaryOperator op;
};

class Syscall final : public OpCode {
public:
    std::string genNasm() override {
        return "\tsyscall";
    }
};

class Call final : public OpCode {
public:
    explicit Call(const std::string& func) {
        this->func = func;
    }

    std::string genNasm() override {
        return "\tcall " + func;
    }

private:
    std::string func;
};

class ReturnOp final : public OpCode {
public:
    std::string genNasm() override {
        return "\tret";
    }
};

class DefineString final : public OpCode {
public:
    DefineString(const std::string& toDefine, const int index) {
        this->toDefine = toDefine;
        this->index = index;

        id = "string_" + std::to_string(index);
    }

    std::string getId() {
        return id;
    }

    std::string genNasm() override {
        replaceAll(toDefine, "\\n", "\", 0xA, \"");
        std::string out = "\t";
        out.append(id);
        out.append(": db \"");
        out.append(toDefine);
        out.append("\", 0");
        return out;
    }

private:
    std::string toDefine;
    std::string id;
    int index;
};

class DefineVar final : public OpCode {
public:
    DefineVar(const std::string& id, const std::string& type, const std::string& value) {
        this->id = id;
        this->type = type;
        this->value = value;
    }

    std::string genNasm() override {
        std::string out = "\t";
        out.append(id);
        out.append(": ");
        out.append(type);
        out.append(" ");
        out.append(value);
        return out;
    }
private:
    std::string id;
    std::string type;
    std::string value;
};

#endif