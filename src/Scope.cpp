#include "Scope.hpp"

Scope::Scope(Scope* parent) {
    this->parent = parent;
}

Scope* Scope::getParent() {
    return parent;
}

void Scope::addVar(Identifier id, std::string address) {
    vars.emplace(id, address);
}

std::string Scope::getVar(Identifier id) {
    if(vars.find(id) == vars.end()) {
        return parent->getVar(id);
    }
    return vars.find(id)->second;
}