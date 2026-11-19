#include <iostream>
#include <memory>
#ifdef _WIN32
#include <windows.h>
#endif

#include "equations.hpp"
#include "test.hpp"

void test_complex_operations() {
    Complex a(1, 2), b(3, -1);
    check(a + b == Complex(4, 1));
    check(a - b == Complex(-2, 3));
    check(a * b == Complex(5, 5));
    check(a / b == Complex(0.1, 0.7));
    check(a == Complex(1, 2));
    check(a != b);
    check(Complex(0, 1) * Complex(0, 1) == Complex(-1, 0));
}

void test_complex_immutable() {
    const Complex a(1, 2);
    Complex b = a + Complex(1, 1);
    check(a == Complex(1, 2));
    check(b == Complex(2, 3));
}

void test_complex_edge_cases() {
    checkThrows([] { Complex(1, 2) / Complex(0, 0); });
    check(Complex(-4, 0).sqrt() == Complex(0, 2));
    check(Complex(0, 2).sqrt() == Complex(1, 1));
    check(Complex(0, 0).sqrt() == Complex(0, 0));
    check(Complex(1, 2).toString() == "1+2i");
    check(Complex(1, -2).toString() == "1-2i");
}

void test_linear() {
    LinearEquation e(Complex(2), Complex(-6));
    check(e.solve().size() == 1);
    check(e.solve()[0] == Complex(3));
    check(e(Complex(3)) == Complex(0));
    check(e(Complex(1, 1)) == Complex(-4, 2));
}

void test_linear_invalid() {
    checkThrows([] { LinearEquation(Complex(0), Complex(5)); });
    checkThrows([] { LinearEquation(Complex(0, 0), Complex(0, 0)); });
}

void test_quadratic() {
    QuadraticEquation e(Complex(1), Complex(-3), Complex(2));
    auto roots = e.solve();
    check(roots.size() == 2);
    check(roots[0] == Complex(2) && roots[1] == Complex(1));
    check(e(roots[0]) == Complex(0) && e(roots[1]) == Complex(0));
}

void test_quadratic_double_root() {
    QuadraticEquation e(Complex(1), Complex(-2), Complex(1));
    auto roots = e.solve();
    check(roots[0] == Complex(1) && roots[1] == Complex(1));
}

void test_quadratic_complex_roots() {
    QuadraticEquation e(Complex(1), Complex(0), Complex(1));
    auto roots = e.solve();
    check(roots[0] == Complex(0, 1) && roots[1] == Complex(0, -1));

    QuadraticEquation e2(Complex(1), Complex(2), Complex(5));
    for (const Complex& x : e2.solve()) check(e2(x) == Complex(0));
}

void test_quadratic_invalid() {
    checkThrows([] {QuadraticEquation(Complex(0), Complex(1), Complex(1));});
}

void test_equation_contract() {
    std::vector<std::unique_ptr<Equation>> all;
    all.push_back(std::make_unique<LinearEquation>(Complex(2), Complex(-6)));
    all.push_back(std::make_unique<LinearEquation>(Complex(1, 1), Complex(2)));
    all.push_back(std::make_unique<QuadraticEquation>(Complex(1), Complex(-3), Complex(2)));
    all.push_back(std::make_unique<QuadraticEquation>(Complex(1), Complex(0), Complex(1)));
    all.push_back(std::make_unique<QuadraticEquation>(Complex(1), Complex(-2), Complex(1)));
    all.push_back(std::make_unique<QuadraticEquation>(Complex(1, 1), Complex(0, 2), Complex(3)));

    for (const auto& e : all) {
        std::vector<Complex> coefficients = e->coefficients();
        check(coefficients[0] != Complex());
        std::vector<Complex> roots = e->solve();
        check(roots.size() == coefficients.size() - 1);
        for (const Complex& x : roots) 
            check((*e)(x) == Complex());
    }
}

void test_coefficients_and_format() {
    LinearEquation lin(Complex(2), Complex(-6));
    check((lin.coefficients() == std::vector<Complex>{Complex(2), Complex(-6)}));
    check(formatEquation(lin) == "(2+0i)x + (-6+0i) = 0");

    QuadraticEquation quad(Complex(1), Complex(-3), Complex(2));
    check(quad.coefficients().size() == 3);
    check(formatEquation(quad) == "(1+0i)x^2 + (-3+0i)x + (2+0i) = 0");
}

EquationCollection makeCollection() {
    EquationCollection c;
    c.add(std::make_unique<LinearEquation>(Complex(1), Complex(-1)));
    c.add(std::make_unique<QuadraticEquation>(Complex(1), Complex(-3), Complex(2)));
    c.add(std::make_unique<QuadraticEquation>(Complex(1), Complex(0), Complex(1)));
    c.add(std::make_unique<LinearEquation>(Complex(2), Complex(-6)));
    return c;
}

void test_collection_empty_and_invalid() {
    EquationCollection c;
    check(c.size() == 0);
    check(c.solveAll().empty());
    check(c.chooseWithRoot(Complex(1)).empty());
    checkThrows([&] { c.add(nullptr); });
    check(c.size() == 0);
}

void test_collection_solve_and_evaluate() {
    EquationCollection c = makeCollection();
    check(c.size() == 4);

    auto all = c.solveAll();
    check(all.size() == 4);
    check(all[0].size() == 1 && all[0][0] == Complex(1));
    check(all[1].size() == 2);
    check(all[3][0] == Complex(3));

    auto values = c.evaluateAll(Complex(1));
    check(values[0] == Complex(0));
    check(values[1] == Complex(0));
    check(values[2] == Complex(2));
    check(values[3] == Complex(-4));
}

void test_collection_with_root() {
    EquationCollection c = makeCollection();
    check(c.chooseWithRoot(Complex(1)).size() == 2);
    check(c.chooseWithRoot(Complex(2)).size() == 1);
    check(c.chooseWithRoot(Complex(3)).size() == 1);
    check(c.chooseWithRoot(Complex(0, 1)).size() == 1);
    check(c.chooseWithRoot(Complex(100)).empty());
}

void demo() {
    std::cout << "\n=== Демонстрация ===\n";
    std::vector<std::unique_ptr<Equation>> list;
    list.push_back(std::make_unique<LinearEquation>(Complex(2), Complex(-6)));
    list.push_back(std::make_unique<QuadraticEquation>(Complex(1), Complex(-3), Complex(2)));
    list.push_back(std::make_unique<QuadraticEquation>(Complex(1), Complex(2), Complex(5)));
    for (const auto& e : list) {
        std::cout << formatEquation(*e) << "\n  корни:";
        for (const Complex& x : e->solve()) 
            std::cout << " " << x.toString();
        std::cout << "\n  f(1+1i) = " << (*e)(Complex(1, 1)).toString() << "\n";
    }
    std::cout << "Уравнений с корнем 1 в коллекции: " << makeCollection().chooseWithRoot(Complex(1)).size() << "\n";
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    std::cout << "=== Автотесты ===\n";
    runTest("test_complex_operations", test_complex_operations);
    runTest("test_complex_immutable", test_complex_immutable);
    runTest("test_complex_edge_cases", test_complex_edge_cases);
    runTest("test_linear", test_linear);
    runTest("test_linear_invalid", test_linear_invalid);
    runTest("test_quadratic", test_quadratic);
    runTest("test_quadratic_double_root", test_quadratic_double_root);
    runTest("test_quadratic_complex_roots", test_quadratic_complex_roots);
    runTest("test_quadratic_invalid", test_quadratic_invalid);
    runTest("test_equation_contract", test_equation_contract);
    runTest("test_coefficients_and_format", test_coefficients_and_format);
    runTest("test_collection_empty_and_invalid", test_collection_empty_and_invalid);
    runTest("test_collection_solve_and_evaluate", test_collection_solve_and_evaluate);
    runTest("test_collection_with_root", test_collection_with_root);
    std::cout << "Проверок: " << g_checks << ", провалено: " << g_failed << "\n";

    if (g_failed == 0) 
        demo();
    return g_failed == 0 ? 0 : 1;
}
