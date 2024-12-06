#ifndef OPCODE_HPP
#define OPCODE_HPP

#include <string>

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
        std::string out = "\t";
        out.append(id);
        out.append(": db \"");
        out.append(toDefine.replace(toDefine.find("\\n"), 2, "\", 0xA, 0xD, \""));
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