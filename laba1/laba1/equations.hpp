#pragma once
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "complex.hpp"

class Equation {
public:
    virtual Complex operator()(const Complex& x) const = 0;
    virtual std::vector<Complex> solve() const = 0;
    virtual std::vector<Complex> coefficients() const = 0;
};

std::string formatEquation(const Equation& e) {
    std::vector<Complex> c = e.coefficients();
    std::string text;
    for (int i = 0; i < c.size(); ++i) {
        int power = c.size() - 1 - i;
        if (i > 0) 
            text += " + ";
        text += "(" + c[i].toString() + ")";
        if (power == 1) 
            text += "x";
        if (power > 1) 
            text += "x^" + std::to_string(power);
    }
    return text + " = 0";
}

void requireNonZeroFirst(const Complex& a) {
    if (a == Complex()) 
        throw std::invalid_argument("a не может быть равно 0");
}

class LinearEquation : public Equation {
public:
    LinearEquation(Complex inA, Complex inB) : a(inA), b(inB) { 
        requireNonZeroFirst(a); 
    }

    Complex operator()(const Complex& x) const override { 
        return a * x + b; 
    }

    std::vector<Complex> solve() const override { 
        return {(Complex() - b) / a}; 
    }

    std::vector<Complex> coefficients() const override { 
        return {a, b}; 
    }

private:
    Complex a, b;
};

class QuadraticEquation : public Equation {
public:
    QuadraticEquation(Complex inA, Complex inB, Complex inC) : a(inA), b(inB), c(inC) {
        requireNonZeroFirst(a);
    }

    Complex operator()(const Complex& x) const override { 
        return a * x * x + b * x + c; 
    }

    std::vector<Complex> solve() const override {
        Complex d = b * b - Complex(4) * a * c;
        Complex s = d.sqrt();
        Complex twoA = Complex(2) * a;
        return {(Complex() - b + s) / twoA, (Complex() - b - s) / twoA};
    }

    std::vector<Complex> coefficients() const override { 
        return {a, b, c}; 
    }

private:
    Complex a, b, c;
};

class EquationCollection {
public:
    void add(std::unique_ptr<Equation> equation) {
        if (!equation) 
            throw std::invalid_argument("пустое уравнение");
        equations.push_back(std::move(equation));
    }

    int size() const { 
        return equations.size(); 
    }

    std::vector<std::vector<Complex>> solveAll() const {
        std::vector<std::vector<Complex>> result;
        for (const auto& e : equations) 
            result.push_back(e->solve());
        return result;
    }

    std::vector<Complex> evaluateAll(const Complex& x) const {
        std::vector<Complex> result;
        for (const auto& e : equations) 
            result.push_back((*e)(x));
        return result;
    }

    std::vector<const Equation*> chooseWithRoot(const Complex& x) const {
        std::vector<const Equation*> result;
        for (const auto& e : equations)
            if ((*e)(x) == Complex()) 
                result.push_back(e.get());
        return result;
    }

private:
    std::vector<std::unique_ptr<Equation>> equations;
};