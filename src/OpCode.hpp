#ifndef OPCODE_HPP
#define OPCODE_HPP

#include <string>

class OpCode {
    virtual std::string genNasm(unsigned int index) {
        return "";
    }
};

class Move : public OpCode {
    Move(std::string first, std::string second) {
        this->first = first;
        this->second = second;
    }
    
    virtual std::string genNasm(unsigned int index) {
        std::string out = "mov ";
        out.append(first);
        out.append(", ");
        out.append(second);
        return out;
    }

    std::string first;
    std::string second;
};

class Syscall : public OpCode {
    virtual std::string genNasm(unsigned int index) {
        return "syscall";
    }
};

class DefineString : public OpCode {
    DefineString(std::string toDefine) {
        this->toDefine = toDefine;
    }

    virtual std::string genNasm(unsigned int index) {
        std::string out = "string_";
        out.append(std::to_string(index));
        out.append(": db \"");
        out.append(toDefine);
        out.append("\"");
    }

    std::string toDefine;
};

#endif