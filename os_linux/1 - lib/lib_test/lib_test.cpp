#include <iostream>
#include "Number.h"
#include "Vector.h"

int main()
{
    Number a = Number::One;
    Number b = Number(8);
    Number c = Number::Zero;

    std::cout << "a: " << a << '\n';
    std::cout << "b: " << b << '\n';
    std::cout << "c: " << c << '\n';

    a *= b;
    std::cout << "a *= b, a = " << a << '\n';
    c = Number::add(a, b);
    std::cout << "c = a + b, c = " << c << '\n';

    Vector v1 = Vector::One;
    Vector v2 = Vector(a, c);
    std::cout << "v1: " << v1 << "\nv2: " << v2 << '\n';
    v1 = v1.add(v2);
    std::cout << "v1 = v1 + v2, v1 = " << v1 << '\n';
    std::cout << "v1: phi: " << v1.polar_phi() << ", r: " << v1.polar_r() << '\n';

}