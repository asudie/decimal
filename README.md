# Decimal library

This repository contains the implementation of the custom s21_decimal.h library in C. The library introduces a decimal data type designed for precise financial calculations, minimizing errors typical of floating-point arithmetic. It supports arithmetic operations, comparison, and conversion functions, modeled for high-precision use cases.

## Key Features
- Binary Representation: Implements a 96-bit integer with a scaling factor for precise fractional calculations.
- Arithmetic Operations: Addition, subtraction, multiplication, division, modulo, and rounding.
- Comparison Operators: Logical comparisons like <, >, ==, and others.
- Converters: Supports conversion between integers, floats, and decimals.
- Unit Testing: Includes comprehensive test coverage (80%+) using the Check library with gcov-based HTML reports.
- Makefile: Facilitates library building, cleaning, and testing.
