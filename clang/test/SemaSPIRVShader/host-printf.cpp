// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -std=c++20 -fshader-host-math -fsyntax-only -verify %s
// expected-no-diagnostics

extern "C" int printf(const char *, ...);

static_assert(__is_same(decltype(printf("number %d", 1)), int));

int host_printf() { return printf("number %d", 1); }
