#include "s21_decimal.h"

int s21_from_float_to_decimal(float src, s21_decimal *dst) {
  int err = 0;
  int scale = 0, E, E_shtrix;
  set_zero(dst);
  uint64_t *p;
  double tmp = src;
  E = getExp(tmp);
  E_shtrix = E;

  if (E < 23) {
    while (scale < 29 && E_shtrix < 24) {
      tmp = tmp * 10;
      E_shtrix = getExp(tmp);
      scale++;
    }
    scale--;
  }
  tmp = src;
  tmp = tmp * pow(10, scale);
  tmp = round(tmp);
  E_shtrix = getExp(tmp);
  p = (uint64_t *)&tmp;
  // E_shtrix - сколько бит в целой части числа tmp
  for (int i = 0; i < E_shtrix; i++) {
    int bit_value = get_bit64(*p, 52 - E_shtrix + i);
    set_bit_96(dst, i, bit_value);
  }
  set_bit_96(dst, E_shtrix, 1);
  *dst = s21_Shift_left(*dst, fmax(0, E - 23));
  // запись степени
  set_power(dst, scale);
  // запись знака
  set_bit_96(dst, 127, get_bit(src, 31));

  err = check_for_float(dst);
  if (err == 2) {
    set_zero(dst);
    err = 1;
  }

  return err;
}

int is_greater_sasha(s21_decimal a, s21_decimal b) {
  int result = 0;

  int sign_a = find_sign_sasha(a.bits[3]);
  int sign_b = find_sign_sasha(b.bits[3]);
  if (sign_a == 0 && sign_b == 1) {
    result = 1;
  } else if (sign_a == 1 && sign_b == 0) {
  } else {
    s21_big_decimal big_a = from_d_to_big(a);
    s21_big_decimal big_b = from_d_to_big(b);
    normalise(&big_a, &big_b);
    for (int i = 6; i >= 0; --i) {
      if (big_a.bits[i] > big_b.bits[i]) {
        if (sign_a == 0) {
          result = 1;
        } else {
          result = 0;
        }
        break;
      } else if (big_a.bits[i] < big_b.bits[i]) {
        if (sign_a == 1) {
          result = 1;
        } else {
          result = 0;
        }
        break;
      }
    }
  }
  return result;
}

int s21_from_decimal_to_float(s21_decimal src, float *dst) {
  int scale = 0;
  double mantiss = 0.0;
  int err = 0;  // fixed

  err = check_for_float(&src);
  if (err == 2) {
    *dst = mantiss;
    err = 1;
  }
  if (err == 0) {
    for (int i = 0; i < 96; i++) {
      if (get_bit_96(src, i)) {
        mantiss += pow(2, i);
      }
    }
    int sign = find_sign_sasha(src.bits[3]);
    scale = get_power(src);
    for (int i = 0; i < scale; i++) {
      mantiss /= 10.0;
    }
    *dst = (float)mantiss;
    if (sign) {
      *dst = -(*dst);
    }
  }

  return err;  // fixed
}

int check_for_float(s21_decimal *result) {
  s21_decimal s21_DECIMAL_MAX = {
      {0b11111111111111111111111111111111, 0b11111111111111111111111111111111,
       0b11111111111111111111111111111111, 0b00000000000000000000000000000000}};
  s21_decimal s21_DECIMAL_MIN = {
      {0b11111111111111111111111111111111, 0b11111111111111111111111111111111,
       0b11111111111111111111111111111111, 0b10000000000000000000000000000000}};

  int err = 1;
  int sign = find_sign(result->bits[3]);
  int(scale) = find_scale(result->bits[3]);
  if (scale > 28) {
    err = 2;
  } else if (sign) {
    if (s21_is_greater_or_equal(*result, s21_DECIMAL_MIN)) err = 0;
  } else {
    if (s21_is_greater_or_equal(s21_DECIMAL_MAX, *result)) err = 0;
  }

  return err;
}

int s21_div(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  // при делении достигаем 1 из 2-х целей, что наступит быстрее
  // 1. остаток от деления мантисс должен быть 0
  // 2. результат деления перестал вмещаться в 96 разрядов

  dec_normalise(&value_1, &value_2);

  int err = 0;
  if (is_decimal_null(value_2)) {
    err = 3;
  } else {
    // проверку на 0 надо еще написать
    s21_big_decimal mant_1, mant_2, result_big, ostatok_big;
    int S1, S2, Scale1, Scale2, otsechennyi_count, shift_count = 0;
    set_zero(result);
    set_zero_big(&result_big);

    // с начало выраниваем числа по 0 разряду
    Scale1 = get_power(value_1);
    Scale2 = get_power(value_2);
    // получаем знаки наших чисел
    S1 = get_bit_96(value_1, 127);
    S2 = get_bit_96(value_2, 127);
    from_decimal_to_big_denis(value_1, &mant_1);
    from_decimal_to_big_denis(value_2, &mant_2);

    div_big(mant_1, mant_2, &result_big, &ostatok_big);

    while (!(is_zero_big(ostatok_big)) &&
           find_first_one_in_big(result_big) < 96) {
      // mant_1 = mant_1 * 10;
      mant_1 = s21_Shift_left_big_10(mant_1, 1);
      div_big(mant_1, mant_2, &result_big, &ostatok_big);
      shift_count++;
    }

    // конвертируем  big_result в decimal
    otsechennyi_count = from_big_to_decimal_denis(result_big, result);
    set_bit_96(result, 127, (S1 + S2) % 2);
    // записали степень
    set_power(result, Scale1 - Scale2 - otsechennyi_count + shift_count);

    err = check_for_div(result);
  }

  return err;
}

int check_for_div(s21_decimal *result) {
  s21_decimal s21_DECIMAL_MAX = {
      {0b11111111111111111111111111111111, 0b11111111111111111111111111111111,
       0b11111111111111111111111111111111, 0b00000000000000000000000000000000}};
  s21_decimal s21_DECIMAL_MIN = {
      {0b11111111111111111111111111111111, 0b11111111111111111111111111111111,
       0b11111111111111111111111111111111, 0b10000000000000000000000000000000}};

  int err = 0;
  int sign = find_sign(result->bits[3]);
  if (sign) {
    if (s21_is_greater(*result, s21_DECIMAL_MIN) ||
        s21_is_equal(*result, s21_DECIMAL_MIN))
      err = 0;
    else
      err = 2;
  } else {
    if (s21_is_greater(s21_DECIMAL_MAX, *result) ||
        s21_is_equal(s21_DECIMAL_MAX, *result))
      err = 0;
    else
      err = 1;
  }
  return err;
}

int s21_from_int_to_decimal_denis(int src, s21_decimal *dst) {
  // заполним наш массив 0, чтобы быть уверен, что там ничего нет лишнего
  set_zero(dst);
  if (src < 0) {
    // в  31 бит 4-го элемента массива записали знак входного числа
    set_bit(&(dst->bits[3]), 31, 1);

    // на входе у нас int, диапозон значений может быть + и -
    // в мантиссу кладем положительное значение src
    // на входе int - а это 32 бита, поэтому весь bits[0] = src
    dst->bits[0] = -src;
  } else {
    dst->bits[0] = src;
  }
  return 0;
}

int s21_from_decimal_to_int_denis(s21_decimal src, int *dst) {
  s21_big_decimal result_big;
  int Scale = get_power(src);
  *dst = 0;
  from_decimal_to_big_denis(src, &result_big);
  result_big = s21_Shift_right_big_10(result_big, Scale);
  for (int e = 0; e < 32; e++) {
    set_bit(dst, e, get_bit_192(result_big, e));
  }
  // установка знака в виде дополнительного кода
  if (get_bit_96(src, 127) == 1) {
    *dst = -*dst;
  }

  return EXIT_SUCCESS;
}

int s21_is_equal_sasha(s21_decimal a, s21_decimal b) {
  return !is_greater(a, b) && !is_greater(b, a);
}

s21_decimal mult_decimal(s21_decimal a, s21_decimal b) {
  s21_decimal result = {0};
  for (int i = 0; i < 32 * 3; ++i) {
    if (a.bits[i / 32] & 1 << (i % 32)) {
      result = add_decimal(result, dec_bit_move(b, i));
    }
  }
  return result;
}

s21_decimal add_decimal(s21_decimal first, s21_decimal second) {
  s21_decimal result = {0};
  bool flag = 0;
  for (int i = 0; i < 3; i++) {
    result.bits[i] = add_chank(first.bits[i], second.bits[i], &flag);
  }
  result.bits[3] = first.bits[3];
  return result;
}

// Arithmetic Operators
int s21_add(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  s21_big_decimal result_b = {0};
  int sign_1 = find_sign(value_1.bits[3]);
  int sign_2 = find_sign(value_2.bits[3]);
  s21_big_decimal value_1b = {0};
  s21_big_decimal value_2b = {0};
  normalise_if_it_need(value_1, &value_1b, value_2, &value_2b);
  if (sign_1 == sign_2) {
    add_big(value_1b, value_2b, &result_b);
  } else {
    if (s21_is_greater_or_equal(modulus_decimal(value_1),
                                modulus_decimal(value_2)))
      sub_big(value_1b, value_2b, &result_b);
    else
      sub_big(value_2b, value_1b, &result_b);
  }
  int status = check_big(&result_b);
  if (status == 0) {
    from_big_to_decimal(result_b, result);
  }
  return status;
}

int s21_sub(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  s21_big_decimal result_b = {0};
  int sign_1 = find_sign(value_1.bits[3]);
  int sign_2 = find_sign(value_2.bits[3]);
  s21_big_decimal value_1b = {0};
  s21_big_decimal value_2b = {0};
  normalise_if_it_need(value_1, &value_1b, value_2, &value_2b);
  if (s21_is_greater_or_equal(modulus_decimal(value_1),
                              modulus_decimal(value_2))) {
    if (sign_1 != sign_2) {
      add_big(value_1b, value_2b, &result_b);
    } else {
      sub_big(value_1b, value_2b, &result_b);
    }
  } else {
    if (sign_1 != sign_2) {
      add_big(value_1b, value_2b, &result_b);
    } else {
      sub_big(value_2b, value_1b, &result_b);
      result_b.bits[7] = change_bit(result_b.bits[7], 31);
    }
  }
  int status = check_big(&result_b);
  if (status == 0) {
    from_big_to_decimal(result_b, result);
  }
  return status;
}

int s21_mul(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  s21_big_decimal result_b = {0};
  s21_big_decimal value_1b = {0};
  s21_big_decimal value_2b = {0};
  normalise_if_it_need(value_1, &value_1b, value_2, &value_2b);
  mul_big(value_1b, value_2b, &result_b);
  int status = check_big(&result_b);
  if (status == 0) {
    from_big_to_decimal(result_b, result);
  }
  return status;
}

int s21_mod(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  // взятие остатка
  s21_decimal result_0 = {0};
  int status = 0;
  if (!is_decimal_null(value_2)) {
    if (s21_is_equal(modulus_decimal(value_2), modulus_decimal(value_1))) {
      *result = result_0;
    } else if (s21_is_greater(modulus_decimal(value_2),
                              modulus_decimal(value_1))) {
      *result = value_1;
    } else {
      s21_big_decimal result_b = {0};
      s21_big_decimal value_1b = {0};
      s21_big_decimal value_2b = {0};
      normalise_if_it_need(value_1, &value_1b, value_2, &value_2b);
      mod_big(value_1b, value_2b, &result_b);
      int status = check_big(&result_b);
      if (status == 0) {
        from_big_to_decimal(result_b, result);
      }
    }
  } else {
    status = 3;
  }
  return status;
}

// Arithmetic Operators for big
int add_big(s21_big_decimal value_1, s21_big_decimal value_2,
            s21_big_decimal *result) {
  // сложение децималов, знак не обрабатывается
  int flag1 = 0;
  for (int i = 0; i < 7; i++) {
    result->bits[i] = add_part(value_1.bits[i], value_2.bits[i], &flag1);
  }
  // пока так, они должны быть одинаковы у обоих децималов
  result->bits[7] = value_1.bits[7];
  return 0;
}

int sub_big(s21_big_decimal value_1, s21_big_decimal value_2,
            s21_big_decimal *result) {
  // вычитание децималов, знак не обрабатывается
  int flag1 = 0;
  for (int i = 0; i < 7; i++) {
    result->bits[i] = sub_part(value_1.bits[i], value_2.bits[i], &flag1);
  }
  result->bits[7] =
      value_1.bits[7];  // пока так, они должны быть одинаковы у обоих децималов
  return 0;
}

int mul_big(s21_big_decimal value_1, s21_big_decimal value_2,
            s21_big_decimal *result) {
  // умножение децималов
  int sign_1 = find_sign(value_1.bits[7]);
  int sign_2 = find_sign(value_2.bits[7]);
  int new_scale = (find_scale(value_1.bits[7]) + find_scale(value_2.bits[7]));
  s21_big_decimal shift_res = value_1;
  for (int i = 0; i < 223; i++) {
    if (i != 0) left_shift_bits(shift_res, &shift_res);
    if (check_bit(value_2.bits[i / 32], i % 32))
      add_big(*result, shift_res, result);
  }
  result->bits[7] = (new_scale) << 16;
  if (sign_1 != sign_2)
    result->bits[7] |= set_bit1(result->bits[7], 31);
  else
    result->bits[7] &= set_bit0(result->bits[7], 31);
  return 0;
}

int mod_big(s21_big_decimal value_1, s21_big_decimal value_2,
            s21_big_decimal *result) {
  // взятие остатка
  normalise_big_if_it_need(&value_1, &value_2);
  s21_big_decimal result_b = {0};
  s21_big_decimal temp_div = {0};
  int sign = find_sign(value_1.bits[7]);
  value_1.bits[7] &= set_bit0(value_1.bits[7], 31);
  value_2.bits[7] &= set_bit0(value_2.bits[7], 31);
  if (is_big_equal(value_1, value_2)) {
    *result = result_b;
  } else if (is_big_greater(value_2, value_1)) {
    if (sign) value_1.bits[7] |= set_bit1(value_1.bits[7], 31);
    *result = value_1;
  } else {
    temp_div = value_2;
    while (1) {
      left_shift_bits(temp_div, &temp_div);
      if ((is_big_greater(temp_div, value_1))) {
        right_shift_bits(temp_div, &temp_div);
        break;
      }
    }
    result_b = value_1;
    while (1) {
      if (is_big_greater(value_2, result_b)) {
        break;
      } else {
        if (!is_big_greater(temp_div, result_b)) {
          sub_big(result_b, temp_div, &result_b);
        } else {
          right_shift_bits(temp_div, &temp_div);
        }
      }
    }
    *result = result_b;
    if (sign) result->bits[7] |= set_bit1(result->bits[7], 31);
  }
  return 0;
}

// Comparison Operators

int s21_is_less(s21_decimal value_1, s21_decimal value_2) {  // меньше
  return (s21_is_greater_or_equal(value_1, value_2)) == 1 ? 0 : 1;
}

int s21_is_less_or_equal(s21_decimal value_1,
                         s21_decimal value_2) {  // меньше или равно
  return (s21_is_greater(value_1, value_2)) == 1 ? 0 : 1;
}

int s21_is_greater(s21_decimal value_1, s21_decimal value_2) {  // больше
  int result = 0;
  if (is_decimal_null(value_1) && is_decimal_null(value_2)) {
    result = 0;
  } else {
    if (find_sign(value_1.bits[3]) < find_sign(value_2.bits[3])) {
      // первое положительное
      result = 1;
    } else if (find_sign(value_1.bits[3]) > find_sign(value_2.bits[3])) {
      result = 0;
    } else {
      // знаки одинаковые
      s21_big_decimal value_1b = {0};
      s21_big_decimal value_2b = {0};
      normalise_if_it_need(value_1, &value_1b, value_2, &value_2b);
      for (int i = 223; i >= 0; i--) {
        if (find_sign(value_1.bits[3])) {
          if (check_bit(value_1b.bits[i / 32], i % 32) <
              check_bit(value_2b.bits[i / 32], i % 32)) {
            result = 1;
            break;
          }
          if (check_bit(value_1b.bits[i / 32], i % 32) >
              check_bit(value_2b.bits[i / 32], i % 32)) {
            result = 0;
            break;
          }
        } else {
          if (check_bit(value_1b.bits[i / 32], i % 32) <
              check_bit(value_2b.bits[i / 32], i % 32)) {
            result = 0;
            break;
          }
          if (check_bit(value_1b.bits[i / 32], i % 32) >
              check_bit(value_2b.bits[i / 32], i % 32)) {
            result = 1;
            break;
          }
        }
      }
    }
  }
  return result;
}

int s21_is_greater_or_equal(s21_decimal value_1,
                            s21_decimal value_2) {  // больше или равно
  return (s21_is_equal(value_1, value_2) == 1 ||
          s21_is_greater(value_1, value_2) == 1)
             ? 1
             : 0;
}

int s21_is_equal(s21_decimal value_1, s21_decimal value_2) {  // равно
  int eq;
  if (is_decimal_null(value_1) && is_decimal_null(value_2)) {
    eq = 1;
  } else {
    if (find_sign(value_1.bits[3]) != find_sign(value_1.bits[3])) {
      eq = 0;
    } else {
      s21_big_decimal value_1b = {0};
      s21_big_decimal value_2b = {0};
      normalise_if_it_need(value_1, &value_1b, value_2, &value_2b);
      eq = is_big_equal(value_1b, value_2b);
    }
  }
  return eq;
}

int s21_is_not_equal(s21_decimal value_1, s21_decimal value_2) {  // не равно
  return (s21_is_equal(value_1, value_2)) == 1 ? 0 : 1;
}

// Comparison Operators for big

int is_big_greater(s21_big_decimal value_1b,
                   s21_big_decimal value_2b) {  // больше
  int result = 0;
  int sign_1 = find_sign(value_1b.bits[7]);
  int sign_2 = find_sign(value_2b.bits[7]);
  if (is_big_decimal_null(value_1b) && is_big_decimal_null(value_2b)) {
    result = 0;
  } else {
    if (sign_1 < sign_2) {  // первое положительное
      result = 1;
    } else if (sign_1 > sign_2) {
      result = 0;
    } else {  // знаки одинаковые
      s21_big_decimal *norm;
      int gap = gap_scale(value_1b, value_2b);
      if (gap > 0) {
        norm = &value_2b;
        normalise_big(*norm, &value_2b, gap);
      } else if (gap < 0) {
        norm = &value_1b;
        normalise_big(*norm, &value_1b, -gap);
      }
      for (int i = 223; i >= 0; i--) {
        if (sign_1) {
          if (check_bit(value_1b.bits[i / 32], i % 32) <
              check_bit(value_2b.bits[i / 32], i % 32)) {
            result = 1;
            break;
          }
          if (check_bit(value_1b.bits[i / 32], i % 32) >
              check_bit(value_2b.bits[i / 32], i % 32)) {
            result = 0;
            break;
          }
        } else {
          if (check_bit(value_1b.bits[i / 32], i % 32) <
              check_bit(value_2b.bits[i / 32], i % 32)) {
            result = 0;
            break;
          }
          if (check_bit(value_1b.bits[i / 32], i % 32) >
              check_bit(value_2b.bits[i / 32], i % 32)) {
            result = 1;
            break;
          }
        }
      }
    }
  }
  return result;
}

int is_big_equal(s21_big_decimal a, s21_big_decimal b) {  // равно
  int eq = 1;
  if (is_big_decimal_null(a) && is_big_decimal_null(b)) {
    eq = 1;
  } else {
    if (find_sign(a.bits[7]) != find_sign(b.bits[7])) {
      eq = 0;
    } else {
      normalise_big_if_it_need(&a, &b);
      int i = 0;
      while (i < 7) {
        if (a.bits[i] != b.bits[i]) {
          eq = 0;
          break;
        }
        i++;
      }
    }
  }
  return eq;
}

// Convertors and parsers

int s21_from_int_to_decimal(int src, s21_decimal *dst) {  // перевод в децимал
  s21_decimal zero = {0};
  *dst = zero;
  if (src > 0) dst->bits[0] = src;
  if (src < 0) {
    dst->bits[0] = -src;
    dst->bits[3] = 0b10000000000000000000000000000000;
  }
  return 0;
}

int s21_from_decimal_to_int(s21_decimal src, int *dst) {  // перевод в инт
  int result = 0;
  int sign = find_sign(src.bits[3]);
  s21_decimal limit = {
      {0b10000000000000000000000000000000, 0, 0, 0}};  // отрицательный
  if (sign) {
    if (s21_is_greater(modulus_decimal(src), limit)) result = 1;
  } else {
    if (s21_is_greater_or_equal(modulus_decimal(src), limit)) result = 1;
  }
  if (!(result)) {
    s21_truncate(src, &src);
    if (sign) {
      *dst = -(src.bits[0]);
    } else {
      *dst = src.bits[0];
    }
  }
  return result;
}

// Convertors and parsers for big

int from_decimal_to_big_decimal(
    s21_decimal src, s21_big_decimal *dst) {  // перевод в биг децимал
  dst->bits[0] = src.bits[0];
  dst->bits[1] = src.bits[1];
  dst->bits[2] = src.bits[2];
  dst->bits[3] = 0;
  dst->bits[4] = 0;
  dst->bits[5] = 0;
  dst->bits[6] = 0;
  dst->bits[7] = src.bits[3];
  return 0;
}

int from_big_to_decimal(s21_big_decimal src, s21_decimal *dst) {
  s21_big_decimal temp = {0};
  // s21_big_decimal temp = {1,0,0,0,0,0,0,0};
  s21_decimal zero = {0};
  if (is_big_decimal_null(src)) {
    *dst = zero;
  } else {
    int sign = find_sign(src.bits[7]);
    int scale = find_scale(src.bits[7]);
    int scale_new = scale;
    temp = src;
    int i = 0;
    for (i = 6; i > 2; i--) {
      while (temp.bits[i] != 0) {
        div_10_big(temp, &temp);
        scale_new--;
      }
    }
    while (scale_new > 28) {
      if (!(is_big_decimal_null(temp))) {
        div_10_big(temp, &temp);
        scale_new--;
      } else {
        break;
      }
    }
    if (scale_new > 28) {
      *dst = zero;
    } else {
      if (scale_new != scale) {
        src.bits[7] = (scale - scale_new) << 16;
        if (sign) src.bits[7] |= set_bit1(src.bits[7], 31);
        round_big(src, &temp);
      }
      dst->bits[0] = temp.bits[0];
      dst->bits[1] = temp.bits[1];
      dst->bits[2] = temp.bits[2];
      dst->bits[3] = (scale_new) << 16;
      if (sign) dst->bits[3] |= set_bit1(dst->bits[3], 31);
    }
  }
  return 0;
}

int from_big_to_decimal_simple(s21_big_decimal src, s21_decimal *dst) {
  dst->bits[0] = src.bits[0];
  dst->bits[1] = src.bits[1];
  dst->bits[2] = src.bits[2];
  dst->bits[3] = src.bits[7];
  return 0;
}

// Another functions

int s21_floor(s21_decimal value, s21_decimal *result) {
  int status;
  status = check_decimal(&value);
  if (!(status)) {
    s21_decimal temp = {0};
    s21_decimal one_bit = {{0b00000000000000000000000000000001, 0, 0, 0}};
    int sign = find_sign(value.bits[3]);
    s21_truncate(value, &temp);
    if (!(s21_is_equal(temp, value)) && (sign)) {
      one_bit.bits[3] |= set_bit1(one_bit.bits[3], 31);
      s21_add(temp, one_bit, result);
    } else {
      *result = temp;
    }
  }
  return status;
}

int s21_round(s21_decimal value,
              s21_decimal *result) {  // округление банковское
  s21_decimal zero = {0};
  int status;
  status = check_decimal(&value);
  int sign = find_sign(value.bits[3]);
  int scale = find_scale(value.bits[3]);
  if (!status) {
    if (is_decimal_null(value)) {
      *result = zero;
    } else if (!(scale)) {
      *result = value;
    } else {
      s21_decimal temp_dec = {0};
      s21_big_decimal temp = {0};
      s21_big_decimal temp_result = {0};
      s21_big_decimal modulo = {0};
      s21_big_decimal scale_big = {0};
      s21_big_decimal value_b = {0};
      s21_big_decimal reference = {{0b00000000000000000000000000000101, 0, 0, 0,
                                    0, 0, 0,
                                    0b00000000000000010000000000000000}};
      s21_big_decimal one_bit = {
          {0b00000000000000000000000000000001, 0, 0, 0, 0, 0, 0, 0}};
      from_decimal_to_big_decimal(value, &value_b);
      scale_to_big_decimal(value_b, &scale_big);
      value_b.bits[7] = 0;
      mod_big(value_b, scale_big, &modulo);
      modulo.bits[7] = scale << 16;
      s21_truncate(value, &temp_dec);
      from_decimal_to_big_decimal(temp_dec, &temp);
      if (sign) {
        one_bit.bits[7] |= set_bit1(one_bit.bits[7], 31);
        modulo.bits[7] &= set_bit0(modulo.bits[7], 31);
        if (is_big_greater(modulo, reference)) {
          add_big(temp, one_bit, &temp_result);
        } else if (is_big_greater(reference, modulo)) {
          temp_result = temp;
        } else {
          if (check_bit(temp.bits[0], 0)) {
            add_big(temp, one_bit, &temp_result);
          } else {
            temp_result = temp;
          }
        }
      } else {
        if (is_big_greater(modulo, reference)) {
          add_big(temp, one_bit, &temp_result);
        } else if (is_big_greater(reference, modulo)) {
          temp_result = temp;
        } else {
          if (check_bit(temp.bits[0], 0))
            add_big(temp, one_bit, &temp_result);
          else
            temp_result = temp;
        }
      }
      from_big_to_decimal_simple(temp_result, result);
    }
  }
  return status;
}

int s21_truncate(s21_decimal value,
                 s21_decimal *result) {  // выделяет целую часть децимала
  s21_decimal zero = {0};
  int status;
  status = check_decimal(&value);
  int scale = find_scale(value.bits[3]);
  if (!status) {
    if (is_decimal_null(value)) {
      *result = zero;
    } else if (scale) {
      s21_big_decimal value_big = {0};
      s21_big_decimal temp = {0};
      from_decimal_to_big_decimal(value, &value_big);
      temp = value_big;
      while (scale) {
        div_10_big(temp, &temp);
        scale--;
      }
      from_big_to_decimal_simple(temp, result);
    } else {
      *result = value;
    }
  }
  return status;
}

int s21_negate(s21_decimal value, s21_decimal *result) {
  int status;
  status = check_decimal(&value);
  if (!status) {
    *result = value;
    int sign = find_sign(value.bits[3]);
    if (sign)
      result->bits[3] &= set_bit0(result->bits[3], 31);
    else
      result->bits[3] |= set_bit1(result->bits[3], 31);
  }
  return status;
}

// Another functions for big

int round_big(s21_big_decimal value,
              s21_big_decimal *result) {  // округление банковское
  s21_big_decimal temp = {0};
  s21_big_decimal temp_result = {0};
  s21_big_decimal modulo = {0};
  s21_big_decimal scale_big = {0};
  s21_big_decimal reference = {{0b00000000000000000000000000000101, 0, 0, 0, 0,
                                0, 0, 0b00000000000000010000000000000000}};
  s21_big_decimal one_bit = {
      {0b00000000000000000000000000000001, 0, 0, 0, 0, 0, 0, 0}};
  int sign = find_sign(value.bits[7]);
  int scale = find_scale(value.bits[7]);
  temp = value;
  scale_to_big_decimal(value, &scale_big);
  value.bits[7] = 0;
  mod_big(value, scale_big, &modulo);
  modulo.bits[7] = scale << 16;
  while (scale) {
    div_10_big(temp, &temp);
    scale--;
  }
  if (sign) {
    one_bit.bits[7] |= set_bit1(one_bit.bits[7], 31);
    modulo.bits[7] &= set_bit0(modulo.bits[7], 31);
    if (is_big_greater(modulo, reference)) {
      add_big(temp, one_bit, &temp_result);
    } else if (is_big_greater(reference, modulo)) {
      temp_result = temp;
    } else {
      if (check_bit(temp.bits[0], 0)) {
        add_big(temp, one_bit, &temp_result);
      } else {
        temp_result = temp;
      }
    }
  } else {
    if (is_big_greater(modulo, reference)) {
      add_big(temp, one_bit, &temp_result);
    } else if (is_big_greater(reference, modulo)) {
      temp_result = temp;
    } else {
      if (check_bit(temp.bits[0], 0))
        add_big(temp, one_bit, &temp_result);
      else
        temp_result = temp;
    }
  }
  *result = temp_result;
  return 0;
}

// Support functions

int check_bit(int value,
              int pos) {  // вернет 1, если на нужной позиции 1, 0 если 0
  return ((value & (1 << pos)) != 0);
}

int set_bit0(int value, int pos) {  // устанавливает бит на нужной позиции в 0
  return (value & ~(1 << pos));
}

int set_bit1(int value, int pos) {  // устанавливает бит на нужной позиции в 1
  return (value | (1 << pos));
}

int change_bit(int value, int pos) {  // меняет бит на противоположный
  return (value ^ (1 << pos));
}

int find_sign(int src) {  // возвращает знак
  bool result = 0;
  result = src >> 31;
  return result;
}

int find_scale(int src) {  // ищет степень 10
  int result = 0;
  int mask_scale = 0b00000000111111110000000000000000;
  result = (mask_scale & src) >> 16;
  return result;
}

int gap_scale(s21_big_decimal value_1,
              s21_big_decimal value_2) {  // вычисляет разницу степеней 10
  return find_scale(value_1.bits[7]) - find_scale(value_2.bits[7]);
}

int left_shift_part(int value,
                    int *temp0) {  // сдвиг влево одного инта на 1 бит
  int res = 0;
  int temp31 = check_bit(value, 31);
  res = value << 1;
  if (*temp0) res |= set_bit1(res, 0);
  *temp0 = temp31;
  return res;
}

int right_shift_part(int value,
                     int *temp31) {  // сдвиг вправо одного инта на 1 бит
  int res = 0;
  int temp0 = check_bit(value, 0);
  res = value >> 1;
  if (*temp31)
    res |= set_bit1(res, 31);
  else
    res &= set_bit0(res, 31);
  *temp31 = temp0;
  return res;
}

int left_shift_bits_10(s21_big_decimal value, s21_big_decimal *result,
                       int step) {  // умножение децимала на любой инт, сдвигом.
  s21_big_decimal zero = {0};
  *result = zero;
  while (step != 0) {
    int temp0 = 0;
    if (step % 2 != 0) add_big(value, *result, result);
    step >>= 1;
    for (int i = 0; i < 7; i++)
      value.bits[i] = left_shift_part(value.bits[i], &temp0);
  }
  result->bits[7] = value.bits[7];
  return 0;
}

int right_shift_bits_10(s21_big_decimal value,
                        s21_big_decimal *result) {  // деление на 10
  s21_big_decimal temp = {0};
  s21_big_decimal multiplier_02 = {{0b00000000000000000000000000000010, 0, 0, 0,
                                    0, 0, 0,
                                    0b00000000000000010000000000000000}};
  mul_big(value, multiplier_02, &temp);
  right_shift_bits(temp, result);
  return 0;
}

int left_shift_bits(s21_big_decimal value,
                    s21_big_decimal *result) {  // сдвиг на 1 бит влево!
  int temp0 = 0;
  for (int i = 0; i < 7; i++)
    result->bits[i] = left_shift_part(value.bits[i], &temp0);
  result->bits[7] = value.bits[7];
  return 0;
}

int right_shift_bits(s21_big_decimal value,
                     s21_big_decimal *result) {  // сдвиг на 1 бит вправо!
  int temp31 = 0;
  for (int i = 6; i >= 0; i--)
    result->bits[i] = right_shift_part(value.bits[i], &temp31);
  result->bits[7] = value.bits[7];
  return 0;
}

int add_part(int value_1, int value_2, int *flag) {  // сложение интов
  int res = 0;
  for (int i = 0; i < 32; i++) {
    if (!(check_bit(value_1, i)) && !(check_bit(value_2, i))) {
      if (*flag) {
        *flag = 0;
        res |= set_bit1(res, i);
      }
    } else if (check_bit(value_1, i) != (check_bit(value_2, i))) {
      if (!*flag) {
        res |= set_bit1(res, i);
      }
    } else if (check_bit(value_1, i) && (check_bit(value_2, i))) {
      if (*flag)
        res |= set_bit1(res, i);
      else
        *flag = 1;
    }
  }
  return res;
}

int sub_part(int value_1, int value_2, int *flag) {  // вычитание интов
  int res = 0;
  for (int i = 0; i < 32; i++) {
    if (!(check_bit(value_1, i)) && !(check_bit(value_2, i))) {
      if (*flag) {
        res |= set_bit1(res, i);
      }
    } else if (check_bit(value_1, i) > (check_bit(value_2, i))) {
      if (!*flag) {
        res |= set_bit1(res, i);
      } else {
        *flag = 0;
      }
    } else if (check_bit(value_1, i) < (check_bit(value_2, i))) {
      if (!*flag) {
        res |= set_bit1(res, i);
        *flag = 1;
      }
    } else if (check_bit(value_1, i) && (check_bit(value_2, i))) {
      if (*flag) {
        res |= set_bit1(res, i);
      }
    }
  }
  return res;
}

int normalise_big(s21_big_decimal value, s21_big_decimal *result,
                  int gap) {  // приводит децимал к нужной степени 10
  if (gap == 0) {
    *result = value;
  } else {
    int sign = find_sign(value.bits[7]);
    int new_scale = (find_scale(value.bits[7]) + gap);
    value.bits[7] = 0;
    while (gap > 0) {
      left_shift_bits_10(value, result, 10);
      value = *result;
      gap--;
    }
    result->bits[7] = (new_scale) << 16;
    if (sign) result->bits[7] |= set_bit1(result->bits[7], 31);
  }
  return 0;
}

int normalise_if_it_need(s21_decimal value_1, s21_big_decimal *value_1b,
                         s21_decimal value_2, s21_big_decimal *value_2b) {
  // проверяет и нормализует один из децималов
  s21_big_decimal *norm;
  from_decimal_to_big_decimal(value_1, value_1b);
  from_decimal_to_big_decimal(value_2, value_2b);
  int gap = gap_scale(*value_1b, *value_2b);
  if (gap > 0) {
    norm = value_2b;
    normalise_big(*norm, value_2b, gap);
  } else if (gap < 0) {
    norm = value_1b;
    normalise_big(*norm, value_1b, -gap);
  }
  return 0;
}

int normalise_big_if_it_need(s21_big_decimal *value_1b,
                             s21_big_decimal *value_2b) {
  // проверяет и нормализует один из децималов
  s21_big_decimal *norm;
  int gap = gap_scale(*value_1b, *value_2b);
  if (gap > 0) {
    norm = value_2b;
    normalise_big(*norm, value_2b, gap);
  } else if (gap < 0) {
    norm = value_1b;
    normalise_big(*norm, value_1b, -gap);
  }
  return 0;
}

int is_decimal_null(s21_decimal a) {  // проверка на ноль
  int res = 1;
  int i = 0;
  while (i < 3) {
    if (a.bits[i] != 0) {
      res = 0;
      break;
    }
    i++;
  }
  return res;
}

int is_big_decimal_null(s21_big_decimal a) {  // проверка на ноль
  int res = 1;
  int i = 0;
  while (i < 7) {
    if (a.bits[i] != 0) {
      res = 0;
      break;
    }
    i++;
  }
  return res;
}

int scale_to_big_decimal(s21_big_decimal src, s21_big_decimal *result) {
  int scale = find_scale(src.bits[7]);
  s21_big_decimal scale_temp = {
      {0b00000000000000000000000000000001, 0, 0, 0, 0, 0, 0, 0}};
  src.bits[7] = 0;
  while (scale > 0) {
    left_shift_bits_10(scale_temp, result, 10);
    scale_temp = *result;
    scale--;
  }
  return 0;
}

s21_decimal modulus_decimal(s21_decimal value) {
  s21_decimal modulus_value = {0};
  modulus_value = value;
  modulus_value.bits[3] &= set_bit0(modulus_value.bits[3], 31);
  return modulus_value;
}

int check_big(s21_big_decimal *result) {
  s21_big_decimal s21_BIG_DECIMAL_MAX = {
      {0b11111111111111111111111111111111, 0b11111111111111111111111111111111,
       0b11111111111111111111111111111111, 0, 0, 0, 0, 0}};
  s21_big_decimal s21_BIG_DECIMAL_MIN = {
      {0b11111111111111111111111111111111, 0b11111111111111111111111111111111,
       0b11111111111111111111111111111111, 0, 0, 0, 0,
       0b10000000000000000000000000000000}};

  int err = 0;
  int sign = find_sign(result->bits[7]);
  if (sign) {
    if (is_big_greater(*result, s21_BIG_DECIMAL_MIN) ||
        is_big_equal(*result, s21_BIG_DECIMAL_MIN))
      err = 0;
    else
      err = 2;
  } else {
    if (is_big_greater(s21_BIG_DECIMAL_MAX, *result) ||
        is_big_equal(s21_BIG_DECIMAL_MAX, *result))
      err = 0;
    else
      err = 1;
  }
  return err;
}
int check_decimal(s21_decimal *result) {
  s21_decimal s21_DECIMAL_MAX = {
      {0b11111111111111111111111111111111, 0b11111111111111111111111111111111,
       0b11111111111111111111111111111111, 0b00000000000000000000000000000000}};
  s21_decimal s21_DECIMAL_MIN = {
      {0b11111111111111111111111111111111, 0b11111111111111111111111111111111,
       0b11111111111111111111111111111111, 0b10000000000000000000000000000000}};

  int err = 1;
  int sign = find_sign(result->bits[3]);
  if (sign) {
    if (s21_is_greater_or_equal(*result, s21_DECIMAL_MIN)) err = 0;
  } else {
    if (s21_is_greater_or_equal(s21_DECIMAL_MAX, *result)) err = 0;
  }
  return err;
}

int div_10_big(s21_big_decimal value, s21_big_decimal *result) {
  // взятие целой части
  int scale = find_scale(value.bits[7]);
  int sign = find_sign(value.bits[7]);
  s21_big_decimal result_sum = {0};
  s21_big_decimal div = {
      {0b00000000000000000000000000001010, 0, 0, 0, 0, 0, 0, 0}};
  s21_big_decimal result_b = {0};
  s21_big_decimal one_bit = {
      {0b00000000000000000000000000000001, 0, 0, 0, 0, 0, 0, 0}};
  s21_big_decimal temp_div = {0};
  value.bits[7] = 0;
  if (is_big_greater(div, value)) {
    *result = result_b;
  } else if (is_big_equal(div, value)) {
    *result = one_bit;
    if (scale) result->bits[7] = (scale - 1) << 16;
    if (sign) result->bits[7] |= set_bit1(result->bits[7], 31);
  } else {
    temp_div = div;
    while (1) {
      left_shift_bits(temp_div, &temp_div);
      left_shift_bits(one_bit, &one_bit);
      if ((is_big_greater(temp_div, value))) {
        right_shift_bits(temp_div, &temp_div);
        right_shift_bits(one_bit, &one_bit);
        break;
      }
    }
    result_b = value;
    while (1) {
      if (is_big_greater(div, result_b)) {
        break;
      } else {
        if (!is_big_greater(temp_div, result_b)) {
          sub_big(result_b, temp_div, &result_b);

          add_big(result_sum, one_bit, &result_sum);
        } else {
          right_shift_bits(temp_div, &temp_div);
          right_shift_bits(one_bit, &one_bit);
        }
      }
    }
    *result = result_sum;
    if (scale) result->bits[7] = (scale - 1) << 16;
    if (sign) result->bits[7] |= set_bit1(result->bits[7], 31);
  }
  return 0;
}

// Function for print

void print_big_decimal(s21_big_decimal src) {
  for (int i = 0; i < 8; i++) {
    for (int j = 31; j >= 0; j--) {
      printf("%d", check_bit(src.bits[i], j));
    }
    printf("\n");
  }
  printf("\n");
}

void print_decimal(s21_decimal src) {
  for (int i = 0; i < 4; i++) {
    for (int j = 31; j >= 0; j--) {
      printf("%d", check_bit(src.bits[i], j));
    }
    printf("\n");
  }
  printf("\n");
}

void print_int(int src) {
  for (int j = 31; j >= 0; j--) {
    printf("%d", check_bit(src, j));
  }
  printf("\n");
}
