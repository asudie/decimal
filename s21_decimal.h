#ifndef SRC_S21_DECIMAL_H
#define SRC_S21_DECIMAL_H

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  int bits[4];
} s21_decimal;

// typedef struct {
//   const int bits[4];
// } const decimal;

typedef struct {
  int bits[8];
} s21_big_decimal;

// typedef struct {
//   const int bits[8];
// } const big_decimal;

int s21_from_float_to_decimal(float src, s21_decimal *dst);
int getExp(double src);
void set_zero(s21_decimal *dst);
int get_bit64(uint64_t src, int index_bit);
void set_bit_96(s21_decimal *x, int index_bit, int value);
s21_decimal s21_Shift_left(s21_decimal value, int n);
void set_power(s21_decimal *x, uint8_t p);
int get_bit(uint32_t src, int index_bit);
void set_bit(int *dst, int index_bit, int value);
int get_bit_96(s21_decimal x, int index_bit);
void set_zero_mantissa(s21_decimal *dst);
int is_greater_sasha(s21_decimal a, s21_decimal b);
int dec_equal(s21_decimal a, s21_decimal b);
int find_sign_sasha(unsigned int bits);
s21_big_decimal from_d_to_big(s21_decimal value);
void normalise(s21_big_decimal *a, s21_big_decimal *b);
s21_big_decimal set_zero_big_decimal(s21_big_decimal a);
int find_mant(int bits);
s21_decimal support_int_to_decimal(int value);
s21_big_decimal mult_big_decimal(s21_big_decimal a, s21_big_decimal b);
s21_decimal set_zero_decimal(s21_decimal a);
s21_big_decimal add_big_decimal(s21_big_decimal first, s21_big_decimal second);
s21_big_decimal bit_move(s21_big_decimal a, int index);
unsigned int add_chank(unsigned int a, unsigned int b, bool *flag);
int s21_from_decimal_to_float(s21_decimal src, float *dst);
int check_for_float(s21_decimal *result);
int get_power(s21_decimal x);
// int find_first_one(s21_decimal src);
void set_bit_64(uint64_t *dst, int index_bit, int value);
void print_decimal_bit_by_bit(s21_decimal bits);
int s21_div(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);
int check_for_div(s21_decimal *result);
int s21_from_int_to_decimal_denis(int src, s21_decimal *dst);
int s21_from_decimal_to_int_denis(s21_decimal src, int *dst);
void dec_normalise(s21_decimal *a, s21_decimal *b);
int s21_is_equal_sasha(s21_decimal a, s21_decimal b);
void set_zero_big(s21_big_decimal *dst);
int from_decimal_to_big_denis(s21_decimal src, s21_big_decimal *dst);
int div_big(s21_big_decimal delimoe, s21_big_decimal delitel,
            s21_big_decimal *result, s21_big_decimal *ostatok);
int is_zero_big(s21_big_decimal x);
int find_first_one_in_big(s21_big_decimal src);
// int find_first_one(s21_decimal src);
s21_big_decimal s21_Shift_left_big_10(s21_big_decimal value, int n);
int from_big_to_decimal_denis(s21_big_decimal src, s21_decimal *dst);
s21_big_decimal s21_Shift_right_big_10(s21_big_decimal value, int n);
int get_bit_192(s21_big_decimal x, int index_bit);
s21_decimal int_to_decimal(int value);
s21_decimal mult_decimal(s21_decimal a, s21_decimal b);
int is_greater(s21_decimal a, s21_decimal b);
int normalozator(s21_big_decimal *src);
int compare_big_mantis(s21_big_decimal x1, s21_big_decimal x2);
int otnimator_mantiss_big(s21_big_decimal M1, s21_big_decimal M2,
                          s21_big_decimal *resultusion);
s21_big_decimal s21_Shift_right_big(s21_big_decimal value, int n);
void set_bit_192(s21_big_decimal *x, int index_bit, int value);
int s21_mul_big(s21_big_decimal value_1, s21_big_decimal value_2,
                s21_big_decimal *result);
s21_decimal add_decimal(s21_decimal first, s21_decimal second);
s21_decimal dec_bit_move(s21_decimal a, int index);
s21_big_decimal s21_Shift_left_big(s21_big_decimal value, int n);
void set_zero_big_mantissa(s21_big_decimal *dst);
int summator_mantiss_big(s21_big_decimal M1, s21_big_decimal M2,
                         s21_big_decimal *resultusion);

// lena's

int s21_add(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);
int s21_sub(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);
int s21_mul(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);
int s21_mod(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);

// Arithmetic Operators for big
int add_big(s21_big_decimal value_1, s21_big_decimal value_2,
            s21_big_decimal *result);
int sub_big(s21_big_decimal value_1, s21_big_decimal value_2,
            s21_big_decimal *result);
int mul_big(s21_big_decimal value_1, s21_big_decimal value_2,
            s21_big_decimal *result);
int mod_big(s21_big_decimal value_1, s21_big_decimal value_2,
            s21_big_decimal *result);

// Comparison Operators
int s21_is_less(s21_decimal, s21_decimal);
int s21_is_less_or_equal(s21_decimal, s21_decimal);
int s21_is_greater(s21_decimal, s21_decimal);
int s21_is_greater_or_equal(s21_decimal, s21_decimal);
int s21_is_equal(s21_decimal value_1, s21_decimal value_2);
int s21_is_not_equal(s21_decimal, s21_decimal);

// Comparison Operators for big
int is_big_greater(s21_big_decimal value_1b, s21_big_decimal value_2b);
int is_big_equal(s21_big_decimal a, s21_big_decimal b);

// Convertors and parsers
int s21_from_int_to_decimal(int src, s21_decimal *dst);
int s21_from_decimal_to_int(s21_decimal src, int *dst);

// Convertors and parsers for big
int from_decimal_to_big_decimal(s21_decimal src, s21_big_decimal *dst);
int from_big_to_decimal(s21_big_decimal src, s21_decimal *dst);
int from_big_to_decimal_simple(s21_big_decimal src, s21_decimal *dst);

// Another functions
int s21_floor(s21_decimal value, s21_decimal *result);
int s21_round(s21_decimal value, s21_decimal *result);
int s21_truncate(s21_decimal value, s21_decimal *result);
int s21_negate(s21_decimal value, s21_decimal *result);

// Another functions for big
int round_big(s21_big_decimal value, s21_big_decimal *result);

// Support functions
int check_bit(int value, int pos);
int set_bit0(int value, int pos);
int set_bit1(int value, int pos);
int change_bit(int value, int pos);
int find_sign(int src);
int find_scale(int src);
int gap_scale(s21_big_decimal value_1, s21_big_decimal value_2);
int left_shift_part(int value, int *temp0);  // сдвиг одного int
                                             // влево на 1 бит (умножение на 2)
int right_shift_part(int value, int *temp31);  // сдвиг одного int
                                               // вправо на 1 бит (деление на 2)
int left_shift_bits_10(s21_big_decimal value, s21_big_decimal *result,
                       int step);  // умножение  на 10 всего децимала
int right_shift_bits_10(
    s21_big_decimal value,
    s21_big_decimal *result);  // деление  на 10 всего децимала
int left_shift_bits(s21_big_decimal value,
                    s21_big_decimal *result);  // сдвиг всего децимала на 1 бит
                                               // влево (умножение на 2)
int right_shift_bits(s21_big_decimal value,
                     s21_big_decimal *result);  // сдвиг всего децимала на 1 бит
                                                // вправо (деление на 2)
int add_part(int value_1, int value_2, int *flag);
int sub_part(int value_1, int value_2, int *flag);
int normalise_big(s21_big_decimal value, s21_big_decimal *result, int gap);
int normalise_if_it_need(s21_decimal value_1, s21_big_decimal *value_1b,
                         s21_decimal value_2, s21_big_decimal *value_2b);
int normalise_big_if_it_need(s21_big_decimal *value_1b,
                             s21_big_decimal *value_2b);
int is_decimal_null(s21_decimal a);
int is_big_decimal_null(s21_big_decimal a);
int scale_to_big_decimal(s21_big_decimal src, s21_big_decimal *scale_big);
s21_decimal modulus_decimal(s21_decimal value);
int check_big(s21_big_decimal *result);
int check_decimal(s21_decimal *result);
int div_10_big(s21_big_decimal value_1, s21_big_decimal *result);

// Function for print
void print_big_decimal(s21_big_decimal src);
void print_decimal(s21_decimal src);
void print_int(int src);

// Constants

#endif  // SRC_S21_DECIMAL_H
