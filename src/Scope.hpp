#ifndef SCOPE_HPP
#define SCOPE_HPP

#include "AST.hpp"
#include <map>

class Scope {
public:
    Scope(Scope* parent);

    Scope* getParent();

    void addVar(Identifier id, std::string address);
    std::string getVar(Identifier id);

private:
    Scope* parent;
    std::map<Identifier, std::string> vars;
};

#endif