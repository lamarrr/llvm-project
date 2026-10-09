//===-- mat.hpp - Row-major matrix utilities for Shader C++ --------------===//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#pragma once

#include "vec.hpp"

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
};

template <unsigned Rows, unsigned Columns>
vec<float, Rows> operator*(mat<Rows, Columns> a, vec<float, Columns> b) {
  return vec<float, Rows>(::mul(a.data, b));
}

template <unsigned Rows, unsigned Columns>
vec<float, Columns> operator*(vec<float, Rows> a, mat<Rows, Columns> b) {
  return vec<float, Columns>(::mul(a, b.data));
}

template <unsigned Rows, unsigned Inner, unsigned Columns>
mat<Rows, Columns> operator*(mat<Rows, Inner> a, mat<Inner, Columns> b) {
  mat<Rows, Columns> result(0.0f);
  auto product = ::mul(a.data, b.data);
  result.template set_product<0>(product);
  return result;
}
