#include "Number.h"
#include <cmath>

Number::Number() {
	value_ = 0;
}

Number::Number(double a) {
	value_ = a;
}

Number::Number(const Number& other) {
	value_ = other.value_;
}

Number Number::add(Number a, Number b) {
	return Number(a.value_ + b.value_);
}

Number Number::substract(Number a, Number b) {
	return Number(a.value_ - b.value_);
}

Number Number::multiply(Number a, Number b) {
	return Number(a.value_ * b.value_);
}

Number Number::divide(Number a, Number b) {
	return Number(a.value_ / b.value_);
}

void Number::operator++() {
	value_++;
}

void Number::operator--() {
	value_--;
}

void Number::operator+=(const Number other) {
	value_ += other.value_;
}

void Number::operator-=(const Number other) {
	value_ -= other.value_;
}

void Number::operator*=(const Number other) {
	value_ *= other.value_;
}

void Number::operator/=(const Number other) {
	value_ /= other.value_;
}

Number& Number::operator=(const Number& other) {
	value_ = other.value_;
	return *this;
}

int const Number::getValue() {
	return value_;
}

std::ostream& operator<<(std::ostream& os, const Number& a) {
	os << a.value_;
	return os;
}

Number Number::sqrt(Number a) {
	return Number(std::sqrt(a.value_));
}

const Number Number::Zero(0);
const Number Number::One(1);