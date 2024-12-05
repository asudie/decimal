#include "s21_decimal.h"

int getExp(double src) {
  int result = 0;
  uint64_t* p;
  p = (uint64_t*)&src;

  for (int i = 0; i < 11; i++) {
    set_bit(&result, i, get_bit64(*p, 52 + i));
  }
  // 2^11 = 2048 /2 - 1 = 1023
  return (result - 1023);
}

void set_zero(s21_decimal* dst) { memset(dst, 0, sizeof(s21_decimal)); }

void set_zero_big(s21_big_decimal* dst) {
  memset(dst, 0, sizeof(s21_big_decimal));
}
int get_bit64(uint64_t src, int index_bit) {
  uint64_t mask = 1;
  mask = mask << index_bit;
  return !!(mask & src);
}

void set_bit_96(s21_decimal* x, int index_bit, int value) {
  set_bit(&(x->bits[index_bit / 32]), index_bit % 32, value);
}

s21_decimal s21_Shift_left(s21_decimal value, int n) {
  s21_decimal new_value = value;
  s21_decimal* p;
  p = &new_value;
  set_zero_mantissa(p);

  for (int i = 0; i < (96 - n); i++) {
    /*int x = i / 32;
    set_bit(&(p->bits[x]), (i + n) % 32, get_bit(value.bits[x], i %
    32));*/
    set_bit_96(p, i + n, get_bit_96(value, i));
  }
  return new_value;
}

void set_power(s21_decimal* x, uint8_t p) {
  for (int i = 16; i < 24; i++) {
    set_bit(&(x->bits[3]), i, get_bit(p, i - 16));
  }
}

int get_bit(uint32_t src, int index_bit) {
  return ((1 << index_bit) & src) >> index_bit;
}

void set_bit(int* dst, int index_bit, int value) {
  *dst &= (~(1 << index_bit));
  *dst |= (value << index_bit);
}

int get_bit_96(s21_decimal x, int index_bit) {
  return get_bit(x.bits[index_bit / 32], index_bit % 32);
}

void set_zero_mantissa(s21_decimal* dst) {
  for (int i = 0; i < 3; i++) {
    dst->bits[i] = 0;
  }
}

int dec_equal(s21_decimal a, s21_decimal b) {
  return !is_greater_sasha(a, b) && !is_greater_sasha(b, a);
}

int find_sign_sasha(unsigned int bits) {
  unsigned int result = 0;
  result = bits >> 31;
  return result;
}

s21_big_decimal from_d_to_big(s21_decimal value) {
  s21_big_decimal result;
  result = set_zero_big_decimal(result);
  for (int i = 0; i < 3; i++) {
    result.bits[i] = value.bits[i];
  }
  result.bits[7] = value.bits[3];
  return result;
}

void normalise(s21_big_decimal* a, s21_big_decimal* b) {
  int scale_a = 0;
  int scale_b = 0;

  scale_a = find_mant(a->bits[7]);
  scale_b = find_mant(b->bits[7]);

  s21_big_decimal ten = from_d_to_big(support_int_to_decimal(10));

  if (scale_a > scale_b) {
    int delta = scale_a - scale_b;
    for (int i = 0; i < (delta); ++i) {
      *b = mult_big_decimal(*b, ten);
      ++scale_b;
    }
    b->bits[7] = scale_b << 16;
  }
  if (scale_b > scale_a) {
    int delta = scale_b - scale_a;
    for (int i = 0; i < (delta); ++i) {
      *a = mult_big_decimal(*a, ten);
      ++scale_a;
    }
    a->bits[7] = scale_a << 16;
  }
}

s21_big_decimal set_zero_big_decimal(s21_big_decimal a) {
  for (int i = 0; i < 8; i++) {
    a.bits[i] = 0;
  }
  return a;
}

int find_mant(int bits) {
  int result = 0;
  int mask = 0b00000000111111110000000000000000;
  result = (bits & mask) >> 16;
  return result;
}

s21_decimal support_int_to_decimal(int value) {
  s21_decimal result;

  result = set_zero_decimal(result);
  result.bits[3] = value & 0b10000000000000000000000000000000;
  if (value < 0) value = -value;

  result.bits[0] = value;

  return result;
}

s21_big_decimal mult_big_decimal(s21_big_decimal a, s21_big_decimal b) {
  s21_big_decimal result = {0};
  for (int i = 0; i < 32 * 7; ++i) {
    if (a.bits[i / 32] & 1 << (i % 32)) {
      result = add_big_decimal(result, bit_move(b, i));
    }
  }
  return result;
}

s21_decimal set_zero_decimal(s21_decimal a) {
  for (int i = 0; i < 4; i++) {
    a.bits[i] = 0;
  }
  return a;
}

s21_big_decimal add_big_decimal(s21_big_decimal first, s21_big_decimal second) {
  s21_big_decimal result = {0};
  bool flag = 0;
  for (int i = 0; i < 7; i++) {
    result.bits[i] = add_chank(first.bits[i], second.bits[i], &flag);
  }
  result.bits[7] = first.bits[7];
  return result;
}

s21_big_decimal bit_move(s21_big_decimal a, int index) {
  for (int j = 0; j < index; ++j) {
    int temp = 0;
    for (int i = 0; i < 6; ++i) {
      int temp2 = (unsigned)a.bits[i] >> 31;
      a.bits[i] = (a.bits[i] << 1) | temp;
      temp = temp2;
    }
  }
  return a;
}

unsigned int add_chank(unsigned int a, unsigned int b, bool* flag) {
  int result = 0;
  for (int i = 0; i < 32; i++) {
    if (*flag == 1) {
      result += 1 << i;
      *flag = 0;
    }
    if ((a & ((unsigned)1 << i)) == (b & ((unsigned)1 << i))) {
      if ((a & ((unsigned)1 << i)) != 0) {
        *flag = 1;
      }
    } else {
      result += ((unsigned)1 << i);
      if (i == 31 &&
          ((result & ((unsigned)1 << i)) == 0)) {  // на случай биг децимала
        *flag = 1;
      }
    }
  }
  return result;
}

int get_power(s21_decimal x) {
  int result = 0;
  for (int i = 16; i < 24; i++) {
    set_bit(&(result), i - 16, get_bit(x.bits[3], i));
  }
  return result;
}

void set_bit_64(uint64_t* dst, int index_bit, int value) {
  uint64_t mask = 1;
  mask = mask << index_bit;
  *dst = *dst & (~mask);
  mask = value;
  mask = mask << index_bit;
  *dst = *dst | mask;
}

// void print_decimal_bit_by_bit(s21_decimal bits) {
//   for (int j = 127; j >= 0; j--) {
//     int number_bits = bits.bits[j / 32];
//     int n_number_bits = j / 32;
//     int index_bit = j % 32;
//     int result = ((1 << index_bit) & number_bits) >> index_bit;

//     printf("%d", result);
//     if (j == 32) {
//       printf(" bits*: %u ", n_number_bits);
//       printf("\n");
//     }
//     if (((j % 32) == 0) && (j != 127) && (j != 32)) {
//       printf(" bits#: %u ", n_number_bits);
//       printf("\n");
//     }
//   }
// }

void dec_normalise(s21_decimal* a, s21_decimal* b) {
  int scale_a = 0;
  int scale_b = 0;
  int sign_a = 0;
  int sign_b = 0;

  scale_a = find_mant(a->bits[3]);
  scale_b = find_mant(b->bits[3]);
  sign_a = find_sign_sasha(a->bits[3]);
  sign_b = find_sign_sasha(b->bits[3]);

  s21_decimal ten = (int_to_decimal(10));

  if (scale_a > scale_b) {
    int delta = scale_a - scale_b;
    for (int i = 0; i < (delta); ++i) {
      *b = mult_decimal(*b, ten);
      ++scale_b;
    }
    b->bits[3] = scale_b << 16;
  }
  if (scale_b > scale_a) {
    int delta = scale_b - scale_a;
    for (int i = 0; i < (delta); ++i) {
      *a = mult_decimal(*a, ten);
      ++scale_a;
    }
    a->bits[3] = scale_a << 16;
  }
  a->bits[3] = a->bits[3] | sign_a << 31;
  b->bits[3] = b->bits[3] | sign_b << 31;
}

int from_decimal_to_big_denis(s21_decimal src, s21_big_decimal* dst) {
  // счетчик для отсеченных разрядов
  int count = 0;
  set_zero_big(dst);

  for (int i = 0; i < 96; i++) {
    set_bit_192(dst, i, get_bit_96(src, i));
  }

  return count;
}

int div_big(s21_big_decimal delimoe, s21_big_decimal delitel,
            s21_big_decimal* result, s21_big_decimal* ostatok) {
  s21_big_decimal tek_ostatok = delimoe;
  s21_big_decimal sdvinytie_delitel = delitel;
  int sdvig_norm_delitel;
  set_zero_big(result);
  int count = 0;

  sdvig_norm_delitel = normalozator(&sdvinytie_delitel);
  for (int i = 0; i <= sdvig_norm_delitel; i++) {
    count = count + 1;
    int rez_bit = compare_big_mantis(tek_ostatok, sdvinytie_delitel);
    if (rez_bit == -1) {
      rez_bit = 0;
    } else {
      rez_bit = 1;
      otnimator_mantiss_big(tek_ostatok, sdvinytie_delitel, &tek_ostatok);
    }
    sdvinytie_delitel = s21_Shift_right_big(sdvinytie_delitel, 1);
    set_bit_192(result, 191 - i, rez_bit);
    if (compare_big_mantis(sdvinytie_delitel, delitel) == -1) {
      break;
    }
  }
  *result = s21_Shift_right_big(*result, 192 - count);
  *ostatok = tek_ostatok;

  if (is_zero_big(*ostatok) == 0) {
    return 1;
  } else {
    return 0;
  }
}

int is_zero_big(s21_big_decimal x) {
  int result = 1;
  for (int i = 0; i < 127; i++) {
    if (get_bit(x.bits[i / 32], i % 32) == 1) {
      // если встретили хоть одну 1, то меняем результат и выходим из
      // цикла
      result = 0;
      i = 127;
    }
  }
  return result;
}

int find_first_one_in_big(s21_big_decimal src) {
  for (int p = (192 + 16); p >= 0; p--) {
    if (get_bit_192(src, p) == 1) {
      return p;
    }
  }
  // если везде нули то вернем -1.
  return -1;
}
// int find_first_one(s21_decimal src) {
//   for (int p = (96 + 16); p >= 0; p--) {
//     if (get_bit_96(src, p) == 1) {
//       return p;
//     }
//   }
//   // если везде нули то вернем -1.
//   return -1;
// }

s21_big_decimal s21_Shift_left_big_10(s21_big_decimal value, int n) {
  s21_big_decimal new_value = value;
  s21_big_decimal ocnovanie;
  set_zero_big(&ocnovanie);
  ocnovanie.bits[0] = 10;

  for (int i = 0; i < n; i++) {
    s21_mul_big(new_value, ocnovanie, &new_value);
  }
  return new_value;
}

int from_big_to_decimal_denis(s21_big_decimal src, s21_decimal* dst) {
  // счетчик для отсеченных разрядов
  int count = 0;
  set_zero(dst);
  // bit_print_dec(dst);

  while (find_first_one_in_big(src) >= 96) {  // && scale !=0
    count++;
    src = s21_Shift_right_big_10(src, 1);
  }

  // if count > scale

  for (int i = 0; i < 96; i++) {
    set_bit_96(dst, i, get_bit_192(src, i));
  }
  // bit_print_dec(dst);
  return count;
}

s21_big_decimal s21_Shift_right_big_10(s21_big_decimal value, int n) {
  s21_big_decimal new_value = value;
  s21_big_decimal ocnovanie;
  s21_big_decimal ost;
  set_zero_big(&ocnovanie);
  ocnovanie.bits[0] = 10;

  for (int i = 0; i < n; i++) {
    div_big(new_value, ocnovanie, &new_value, &ost);
  }
  return new_value;
}

int get_bit_192(s21_big_decimal x, int index_bit) {
  return get_bit(x.bits[index_bit / 32], index_bit % 32);
}

int is_greater(s21_decimal a, s21_decimal b) {
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

int normalozator(s21_big_decimal* src) {
  int position = find_first_one_in_big(*src);
  *src = s21_Shift_left_big(*src, 191 - position);
  return 191 - position;
}

int compare_big_mantis(s21_big_decimal x1, s21_big_decimal x2) {
  int result = 0;
  // 16 взял чисто random это число ни к чему не привязано
  for (int p = (192 + 16); p >= 0; p--) {
    int X1 = get_bit(x1.bits[p / 32], p % 32);
    int X2 = get_bit(x2.bits[p / 32], p % 32);

    if (X1 > X2) {
      result = 1;
      p = -1;
    } else if (X1 < X2) {
      result = -1;
      p = -1;
    }
  }
  return result;
}

int otnimator_mantiss_big(s21_big_decimal M1, s21_big_decimal M2,
                          s21_big_decimal* resultusion) {
  int v_yme = 0, b1, b2;
  set_zero_big(resultusion);

  for (int i = 0; i <= 192; i++) {
    b1 = get_bit_192(M1, i);
    b2 = get_bit_192(M2, i);
    int otnimaemoe = b2 + v_yme;
    v_yme = b1 < otnimaemoe;
    set_bit_192(resultusion, i, (v_yme * 2) + b1 - otnimaemoe);
  }
  return v_yme;
}

s21_big_decimal s21_Shift_right_big(s21_big_decimal value, int n) {
  s21_big_decimal new_value = value;
  s21_big_decimal* p;
  p = &new_value;
  set_zero_big_mantissa(p);
  set_bit_192(p, 192, 0);
  set_bit_192(p, 192 + 1, 0);

  for (int i = n; i <= 193; i++) {
    set_bit_192(p, i - n, get_bit_192(value, i));
  }
  return new_value;
}

void set_bit_192(s21_big_decimal* x, int index_bit, int value) {
  set_bit(&(x->bits[index_bit / 32]), index_bit % 32, value);
}

int s21_mul_big(s21_big_decimal value_1, s21_big_decimal value_2,
                s21_big_decimal* result) {
  set_zero_big(result);

  for (int i = 0; i < 96; i++) {
    if (get_bit_192(value_2, i) == 1) {
      summator_mantiss_big(*result, s21_Shift_left_big(value_1, i), result);
    }
  }
  return EXIT_SUCCESS;
}

s21_decimal dec_bit_move(s21_decimal a, int index) {
  for (int j = 0; j < index; ++j) {
    int temp = 0;
    for (int i = 0; i < 2; ++i) {
      int temp2 = (unsigned)a.bits[i] >> 31;
      a.bits[i] = (a.bits[i] << 1) | temp;
      temp = temp2;
    }
  }
  return a;
}

s21_big_decimal s21_Shift_left_big(s21_big_decimal value, int n) {
  s21_big_decimal new_value = value;
  s21_big_decimal* p;
  p = &new_value;
  set_zero_big_mantissa(p);

  for (int i = 0; i < (192 - n + 1); i++) {
    set_bit_192(p, i + n, get_bit_192(value, i));
  }
  return new_value;
}

void set_zero_big_mantissa(s21_big_decimal* dst) {
  for (int i = 0; i < 6; i++) {
    dst->bits[i] = 0;
  }
}

int summator_mantiss_big(s21_big_decimal M1, s21_big_decimal M2,
                         s21_big_decimal* resultusion) {
  int v_yme = 0, b1, b2;
  set_zero_big(resultusion);

  for (int i = 0; i <= 192; i++) {
    b1 = get_bit_192(M1, i);
    b2 = get_bit_192(M2, i);
    set_bit_192(resultusion, i, (b1 + b2 + v_yme) % 2);
    // делим на 2 получаем старший разряд, а когда %2 получаем младший
    // разряд
    v_yme = (b1 + b2 + v_yme) / 2;
  }

  return v_yme;
}

s21_decimal int_to_decimal(int value) {
  s21_decimal result;

  result = set_zero_decimal(result);
  result.bits[3] = value & 0b10000000000000000000000000000000;
  if (value < 0) value = -value;

  result.bits[0] = value;

  return result;
}
