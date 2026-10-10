%record = type { [2 x float] }
%params = type { target("spirv.$TypedPointerType", %record, 14) }
@g = addrspace(13) global %params zeroinitializer, !spirv.cxx.pointee !0

!0 = !{%params poison}
