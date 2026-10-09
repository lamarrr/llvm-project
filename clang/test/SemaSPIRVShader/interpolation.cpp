// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -fsyntax-only -verify %s
__vertex__
void vertex(__flat__ __output__(0) unsigned *id,
            __centroid__ __noperspective__ __output__(1) float *linear,
            __sample__ __output__(2) float *sample);
__fragment__
void fragment(__input__(0) __flat__ unsigned id,
              __input__(1) __noperspective__ __centroid__ float linear,
              __input__(2) __sample__ float sample);
__flat__ float global;                             // expected-error {{interpolation qualifiers must be declared on entry-point parameters}}
void conflict(__flat__ __noperspective__ float x); // expected-error {{conflicting interpolation qualifiers}}
void conflict2(__centroid__ __sample__ float x);   // expected-error {{conflicting interpolation qualifiers}}
void conflict3(__flat__ __sample__ float x);       // expected-error {{conflicting interpolation qualifiers}}
void conflict4(__centroid__ __flat__ float x);     // expected-error {{conflicting interpolation qualifiers}}
