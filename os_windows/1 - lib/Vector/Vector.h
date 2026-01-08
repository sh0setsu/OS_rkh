#pragma once

#ifdef VECTOR_EXPORTS
#define VECTOR_API __declspec(dllexport)
#else
#define VECTOR_API __declspec(dllimport)
#endif

#include "Number.h"
#include <iostream>

class VECTOR_API Vector {
public:
	Vector();
	explicit Vector(Number x, Number y);
	Vector(const Vector& other);
	Vector& operator=(const Vector& other);

	Vector add(Vector other);
	Number polar_phi();
	Number polar_r();

	friend VECTOR_API std::ostream& operator<<(std::ostream& os, const Vector& v);

	static const Vector Zero;
	static const Vector One;

private:
	Number x_;
	Number y_;
};