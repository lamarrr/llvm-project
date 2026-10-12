//===-- mat.hpp - Row-major matrix utilities for Shader C++ --------------===//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#pragma once

#include "vec.hpp"

namespace spirv {

template <unsigned Rows, unsigned Columns = Rows> struct mat {
  float data[Rows][Columns];

  constexpr mat(float diagonal = 1.0f) { set_diagonal<0>(diagonal); }

  template <class S>
    requires(requires(S s) { s.data[0]; })
  constexpr mat(S s) {
    set_columns<0>(s);
  }

  template <class A, class B, class... C> constexpr mat(A a, B b, C... c) {
    vec<float, Rows * Columns> values(a, b, c...);
    set_columns<0>(values);
  }

  template <unsigned I> constexpr void set_diagonal(float diagonal) {
    if constexpr (I < Rows * Columns) {
      constexpr unsigned Row = I / Columns;
      constexpr unsigned Col = I % Columns;
      data[Row][Col] = Row == Col ? diagonal : 0.0f;
      set_diagonal<I + 1>(diagonal);
    }
  }

  template <unsigned I, class S> constexpr void set_columns(S values) {
    if constexpr (I < Rows * Columns) {
      constexpr unsigned Row = I % Rows;
      constexpr unsigned Col = I / Rows;
      data[Row][Col] = values.data[I];
      set_columns<I + 1>(values);
    }
  }

  template <unsigned I, class S> constexpr void set_product(const S &values) {
    if constexpr (I < Rows * Columns) {
      constexpr unsigned Row = I / Columns;
      constexpr unsigned Col = I % Columns;
      data[Row][Col] = values[Row][Col];
      set_product<I + 1>(values);
    }
  }

  constexpr float (&operator[](unsigned row)) [Columns] {
    return data[row];
  } constexpr const float (&operator[](unsigned row) const)[Columns] {
    return data[row];
  }
#ifdef __SPIRV__
  constexpr decltype(auto) operator[](this auto &self, unsigned row) {
    using Row = float[Columns];
    if constexpr (__is_same(decltype(&self.data[0][0]), float __storage *) ||
                  __is_same(decltype(&self.data[0][0]),
                            const float __storage *)) {
      auto *elements = &self.data[0][0] + row * Columns;
      if constexpr (__is_same(decltype(elements), const float __storage *))
        return *reinterpret_cast<const Row __storage *>(elements);
      else
        return *reinterpret_cast<Row __storage *>(elements);
    } else
      return (self.data[row]);
  }
#endif
};

template <unsigned Rows, unsigned Columns>
constexpr vec<float, Rows> operator*(mat<Rows, Columns> a,
                                     vec<float, Columns> b) {
  if (__builtin_is_constant_evaluated()) {
    vec<float, Rows> result(0.0f);
    for (unsigned row = 0; row < Rows; ++row)
      for (unsigned column = 0; column < Columns; ++column)
        result[row] += a[row][column] * b[column];
    return result;
  }
  return vec<float, Rows>(::mul(a.data, b));
}

template <unsigned Rows, unsigned Columns>
constexpr vec<float, Columns> operator*(vec<float, Rows> a,
                                        mat<Rows, Columns> b) {
  if (__builtin_is_constant_evaluated()) {
    vec<float, Columns> result(0.0f);
    for (unsigned column = 0; column < Columns; ++column)
      for (unsigned row = 0; row < Rows; ++row)
        result[column] += a[row] * b[row][column];
    return result;
  }
  return vec<float, Columns>(::mul(a, b.data));
}

template <unsigned Rows, unsigned Inner, unsigned Columns>
constexpr mat<Rows, Columns> operator*(mat<Rows, Inner> a,
                                       mat<Inner, Columns> b) {
  mat<Rows, Columns> result(0.0f);
  if (__builtin_is_constant_evaluated()) {
    for (unsigned row = 0; row < Rows; ++row)
      for (unsigned column = 0; column < Columns; ++column)
        for (unsigned index = 0; index < Inner; ++index)
          result[row][column] += a[row][index] * b[index][column];
    return result;
  }
  auto product = ::mul(a.data, b.data);
  result.template set_product<0>(product);
  return result;
}

template <unsigned Rows, unsigned Columns>
constexpr mat<Rows, Columns> matrix_comp_mult(mat<Rows, Columns> first,
                                              mat<Rows, Columns> second) {
  mat<Rows, Columns> result(0.0f);
  for (unsigned row = 0; row < Rows; ++row)
    for (unsigned column = 0; column < Columns; ++column)
      result[row][column] = first[row][column] * second[row][column];
  return result;
}
template <unsigned Rows, unsigned Columns>
constexpr mat<Rows, Columns> outer_product(vec<float, Rows> column,
                                           vec<float, Columns> row) {
  mat<Rows, Columns> result(0.0f);
  for (unsigned row_index = 0; row_index < Rows; ++row_index)
    for (unsigned column_index = 0; column_index < Columns; ++column_index)
      result[row_index][column_index] = column[row_index] * row[column_index];
  return result;
}
template <unsigned Rows, unsigned Columns>
constexpr mat<Columns, Rows> transpose(mat<Rows, Columns> value) {
  mat<Columns, Rows> result(0.0f);
  for (unsigned row = 0; row < Rows; ++row)
    for (unsigned column = 0; column < Columns; ++column)
      result[column][row] = value[row][column];
  return result;
}
template <unsigned Size>
constexpr mat<Size, Size> inverse(mat<Size, Size> value) {
  if (__builtin_is_constant_evaluated()) {
    float augmented[Size][Size * 2]{};
    for (unsigned row = 0; row < Size; ++row)
      for (unsigned column = 0; column < Size; ++column) {
        augmented[row][column] = value[row][column];
        augmented[row][Size + column] = row == column ? 1.0f : 0.0f;
      }
    for (unsigned pivot = 0; pivot < Size; ++pivot) {
      unsigned pivot_row = pivot;
      while (pivot_row < Size && augmented[pivot_row][pivot] == 0.0f)
        ++pivot_row;
      if (pivot_row == Size)
        return mat<Size, Size>(0.0f);
      if (pivot_row != pivot)
        for (unsigned column = 0; column < Size * 2; ++column) {
          float temporary = augmented[pivot][column];
          augmented[pivot][column] = augmented[pivot_row][column];
          augmented[pivot_row][column] = temporary;
        }
      float divisor = augmented[pivot][pivot];
      for (unsigned column = 0; column < Size * 2; ++column)
        augmented[pivot][column] /= divisor;
      for (unsigned row = 0; row < Size; ++row) {
        if (row == pivot)
          continue;
        float factor = augmented[row][pivot];
        for (unsigned column = 0; column < Size * 2; ++column)
          augmented[row][column] -= factor * augmented[pivot][column];
      }
    }
    mat<Size, Size> result(0.0f);
    for (unsigned row = 0; row < Size; ++row)
      for (unsigned column = 0; column < Size; ++column)
        result[row][column] = augmented[row][Size + column];
    return result;
  }
  auto inverted = ::inverse(value.data);
  mat<Size, Size> result(0.0f);
  for (unsigned row = 0; row < Size; ++row)
    for (unsigned column = 0; column < Size; ++column)
      result[row][column] = inverted.data[row][column];
  return result;
}

} // namespace spirv
