#include "Vector.h"
#include <cmath>

Vector::Vector() {
  x_ = Number::Zero;
  y_ = Number::Zero;
}

Vector::Vector(Number x, Number y) {
  x_ = x;
  y_ = y;
}

Vector::Vector(const Vector& other) {
  x_ = other.x_;
  y_ = other.y_;
}

Vector& Vector::operator=(const Vector& other) {
  x_ = other.x_;
  y_ = other.y_;
  return *this;
}

Vector Vector::add(Vector other) {
  return Vector(Number::add(x_, other.x_), Number::add(y_, other.y_));
}

Number Vector::polar_phi() {
  return Number(std::atan2(y_.getValue(), x_.getValue()));
}

Number Vector::polar_r() {
  return Number(Number::sqrt(Number::add(Number::multiply(x_, x_), Number::multiply(y_, y_))));
}

std::ostream& operator<<(std::ostream& os, const Vector& v) {
  os << '(' << v.x_ << ", " << v.y_ << ')';
  return os;
}

const Vector Vector::Zero(Number(0), Number(0));
const Vector Vector::One(Number(1), Number(1));