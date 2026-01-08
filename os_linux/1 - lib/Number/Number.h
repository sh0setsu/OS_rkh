#pragma once
#include <iostream>

class Number
{
    public:
        Number();
        explicit Number(double a);
        ~Number() = default;

        Number(const Number& other);
        Number& operator=(const Number& other);
        int const getValue();

        static Number add(Number a, Number b);
        static Number substract(Number a, Number b);
        static Number multiply(Number a, Number b);
        static Number divide(Number a, Number b);
        static Number sqrt(Number a);

        void operator++();
        void operator--();
        void operator+=(const Number other);
        void operator-=(const Number other);
        void operator*=(const Number other);
        void operator/=(const Number other);
        friend std::ostream& operator<<(std::ostream& os, const Number& a);

        static const Number Zero;
        static const Number One;
    private:
        double value_;
};