#pragma once

#include "Number.h"
#include <iostream>

class Vector {
public:
  Vector();
  explicit Vector(Number x, Number y);
  Vector(const Vector& other);
  Vector& operator=(const Vector& other);

  Vector add(Vector other);
  Number polar_phi();
  Number polar_r();

  friend std::ostream& operator<<(std::ostream& os, const Vector& v);

  static const Vector Zero;
  static const Vector One;

private:
  Number x_;
  Number y_;
};