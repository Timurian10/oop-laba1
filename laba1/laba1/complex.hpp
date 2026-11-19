#pragma once
#include <cmath>
#include <sstream>
#include <stdexcept>
#include <string>

class Complex {
public:
    Complex(double inRe = 0, double inIm = 0) : re(inRe), im(inIm) {}

    double getRe() const { 
        return re;
    }

    double getIm() const { 
        return im;
    }

    Complex operator+(const Complex& o) const { 
        return Complex(re + o.re, im + o.im); 
    }
    Complex operator-(const Complex& o) const { 
        return Complex(re - o.re, im - o.im); 
    }
    Complex operator*(const Complex& o) const {
        return Complex(re * o.re - im * o.im, re * o.im + im * o.re);
    }
    Complex operator/(const Complex& o) const {
        double d = o.re * o.re + o.im * o.im;
        if (d == 0) 
            throw std::domain_error("деление на ноль");
        return Complex((re * o.re + im * o.im) / d, (im * o.re - re * o.im) / d);
    }

    bool operator==(const Complex& o) const {
        return std::fabs(re - o.re) < 1e-9 && std::fabs(im - o.im) < 1e-9;
    }
    bool operator!=(const Complex& o) const { 
        return !(*this == o); 
    }

    Complex sqrt() const {
        double r = std::sqrt(re * re + im * im);
        double a = std::sqrt((r + re) / 2);
        double b = std::sqrt((r - re) / 2);
        return Complex(a, im < 0 ? -b : b);
    }

    std::string toString() const {
        std::ostringstream s;
        s << re << (im < 0 ? "-" : "+") << std::fabs(im) << "i";
        return s.str();
    }

private:
    double re, im;
};
