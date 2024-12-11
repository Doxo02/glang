#ifndef OPCODE_HPP
#define OPCODE_HPP

#include <string>
#include <iostream>
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
    virtual std::string genNasm() = 0;
};

class Label : public OpCode {
public:
    Label(std::string name) {
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

class Push : public OpCode {
public:
    Push(std::string reg) {
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

class Pop : public OpCode {
public:
    Pop(std::string reg) {
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

class Move : public OpCode {
public:
    Move(std::string first, std::string second) {
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

class Add : public OpCode {
public:
    Add(std::string first, std::string second) {
        this->first = first;
        this->second = second;
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
};

class Sub : public OpCode {
public:
    Sub(std::string first, std::string second) {
        this->first = first;
        this->second = second;
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
};

class Multiply : public OpCode {
public:
    Multiply(std::string first, std::string second) {
        this->first = first;
        this->second = second;
    }

    std::string genNasm() override {
        std::string out = "\timul ";
        out.append(first);
        out.append(", ");
        out.append(second);
        return out;
    }

private:
    std::string first;
    std::string second;
};

class Div : public OpCode {
public:
    Div(std::string first, std::string second) {
        this->first = first;
        this->second = second;
    }

    std::string genNasm() override {
        std::string out = "\tidiv ";
        out.append(first);
        out.append(", ");
        out.append(second);
        return out;
    }

private:
    std::string first;
    std::string second;
};

class Syscall : public OpCode {
public:
    std::string genNasm() override {
        return "\tsyscall";
    }
};

class Call : public OpCode {
public:
    Call(std::string func) {
        this->func = func;
    }

    std::string genNasm() override {
        return "\tcall " + func;
    }

private:
    std::string func;
};

class ReturnOp : public OpCode {
public:
    std::string genNasm() override {
        return "\tret";
    }
};

class DefineString : public OpCode {
public:
    DefineString(std::string toDefine, int index) {
        this->toDefine = toDefine;
        this->index = index;

        id = "string_" + std::to_string(index);
    }

    std::string getId() {
        return id;
    }

    std::string genNasm() override {
        replaceAll(toDefine, "\\n", "\", 0xA, 0xD, \"");
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

class DefineVar : public OpCode {
public:
    DefineVar(std::string id, std::string type) {
        this->id = id;
        this->type = type;
    }

    std::string genNasm() override {
        std::string out = "\t";
        out.append(id);
        out.append(": ");
        out.append(type);
        out.append(" 0");
        return out;
    }
private:
    std::string id;
    std::string type;
};

#endif