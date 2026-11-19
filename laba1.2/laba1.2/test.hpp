#pragma once
#include <functional>
#include <iostream>
#include <string>

int g_checks = 0;
int g_failed = 0;
std::string g_test;

void check(bool ok) {
    ++g_checks;
    if (!ok) {
        ++g_failed;
        std::cout << "  [FAIL] " << g_test << "\n";
    }
}

void checkThrows(const std::function<void()>& action) {
    bool thrown = false;
    try {
        action();
    } catch (const std::exception&) {
        thrown = true;
    }
    check(thrown);
}

void runTest(const std::string& name, void (*test)()) {
    g_test = name;
    int before = g_failed;
    test();
    std::cout << (g_failed == before ? "[ OK ] " : "[FAIL] ") << name << "\n";
}
