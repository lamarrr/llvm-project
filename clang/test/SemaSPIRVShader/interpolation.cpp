// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -fsyntax-only -verify %s
__vertex
void vertex(__flat __output(0) unsigned *id,
            __centroid __noperspective __output(1) float *linear,
            __sample __output(2) float *sample);
__fragment
void fragment(__input(0) __flat unsigned id,
              __input(1) __noperspective __centroid float linear,
              __input(2) __sample float sample);
__flat float global;                             // expected-error {{interpolation qualifiers must be declared on entry-point parameters}}
void conflict(__flat __noperspective float x); // expected-error {{conflicting interpolation qualifiers}}
void conflict2(__centroid __sample float x);   // expected-error {{conflicting interpolation qualifiers}}
void conflict3(__flat __sample float x);       // expected-error {{conflicting interpolation qualifiers}}
void conflict4(__centroid __flat float x);     // expected-error {{conflicting interpolation qualifiers}}
