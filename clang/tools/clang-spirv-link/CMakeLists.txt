set(LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  Analysis BitReader BitWriter CodeGen Core IRReader Linker MC Passes
  Support Target TargetParser TransformUtils
)
add_clang_tool(clang-spirv-link ClangSPIRVLink.cpp)

