#!/usr/bin/env python3
"""Semantic, native codegen, validation, reflection and optional runtime checks."""
import argparse
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
p = argparse.ArgumentParser()
p.add_argument('--runtime', action='store_true')
p.add_argument('--host-cxx', default='c++')
a = p.parse_args()
bin = root / 'build/bin'
def run(*command, **kwargs):
    return subprocess.run([str(x) for x in command], cwd=root, check=True, **kwargs)
cc1 = [bin / 'clang', '-cc1', '-triple', 'spirv1.4-unknown-vulkan1.3',
       '-std=c++20', '-fspirv-shader']
for test in ('headerless.cpp', 'diagnostics.cpp'):
    run(*cc1, '-fsyntax-only', '-verify', root / 'clang/test/SemaSPIRVShader' / test)
with tempfile.TemporaryDirectory(prefix='shader-cxx-') as tmp:
    tmp = Path(tmp)
    ir = run(*cc1, '-emit-llvm', '-o', '-', 'clang/test/CodeGenSPIRVShader/entry.cpp',
             capture_output=True).stdout
    run(bin / 'FileCheck', 'clang/test/CodeGenSPIRVShader/entry.cpp', input=ir)
    for source, entry in [('scale.cpp', 'scale'), ('alignment.cpp', 'align'),
                          ('stages.cpp', 'vertex'), ('stages.cpp', 'fragment'),
                          ('math.cpp', 'math'), ('uniform.cpp', 'uniform')]:
        output = tmp / f'{entry}.spv'
        run(bin / 'clang++', '-fspirv-shader', '-std=c++20', f'-fspirv-entry={entry}',
            root / 'shader-cxx/examples' / source, '-o', output)
        assert struct.unpack('<II', output.read_bytes()[:8]) == (0x07230203, 0x00010400)
        run(bin / 'spirv-val', '--target-env', 'vulkan1.3', '--scalar-block-layout', output)
        reflection = json.loads(Path(str(output) + '.json').read_text())
        assert reflection['entry'] == entry
        if entry == 'scale':
            push = next(i for i in reflection['interfaces'] if i['kind'] == 'push_constant')
            assert push['size'] == 16 and push['alignment'] == 8
            assert [f['offset'] for f in push['members']] == [0, 8]
            assert push['members'][0]['kind'] == 'device_address'
        if entry == 'uniform':
            uniform = next(i for i in reflection['interfaces'] if i['kind'] == 'uniform')
            assert uniform['set'] == 2 and uniform['binding'] == 7
        if entry == 'align':
            assembly = run(bin / 'spirv-dis', output, capture_output=True, text=True).stdout
            accesses = [line for line in assembly.splitlines() if 'Aligned' in line]
            assert len(accesses) == 4 and ['Aligned 16' in line for line in accesses] == [True, True, False, False]
    optimized = tmp / 'scale-o2.spv'
    run(bin / 'clang++', '-O2', '-fspirv-shader', '-std=c++20',
        'shader-cxx/examples/scale.cpp', '-o', optimized)
    helper = tmp / 'helper.cpp'
    helper.write_text('template<class T> T twice(T x) { return x * 2; }\nfloat helper(float x) { return twice(x); }\n')
    shader = tmp / 'shader.cpp'
    shader.write_text('float helper(float);\nstruct P { __storage__ float *data; };\n'
                      '__compute__ __workgroup_size__(1,1,1) void linked(__push_constant__ const P *p) { *p->data = helper(*p->data); }\n')
    for source in (helper, shader):
        run(bin / 'clang++', '-fspirv-shader', '-std=c++20', '-c', source,
            '-o', source.with_suffix('.bc'))
    run(bin / 'clang-spirv-link', helper.with_suffix('.bc'), shader.with_suffix('.bc'),
        '--entry=linked', '-o', tmp / 'linked.spv')
    for name, source, diagnostic in [
        ('recursive', '__compute__ __workgroup_size__(1,1,1) void recursive() { recursive(); }',
         'recursion'),
        ('assembly', '__compute__ __workgroup_size__(1,1,1) void assembly() { asm volatile("OpNop"); }',
         'inline assembly lowering is not implemented'),
        ('ray', '__ray_generation__ void ray() {}', 'not implemented'),
    ]:
        path = tmp / f'{name}.cpp'
        path.write_text(source)
        output = tmp / f'{name}.spv'
        result = subprocess.run([str(bin / 'clang++'), '-std=c++20', '-fspirv-shader',
                                 str(path), '-o', str(output)], capture_output=True, text=True)
        assert result.returncode != 0 and diagnostic in result.stderr, result.stderr
        assert not output.exists() and not Path(str(output) + '.json').exists()
    if a.runtime:
        harness = tmp / 'run-scale'
        run(a.host_cxx, '-std=c++20', '-I.shader-deps/Vulkan-Headers/include',
            'shader-cxx/tests/run-scale.cpp', '-l:libvulkan.so.1', '-o', harness)
        env = os.environ.copy()
        env.setdefault('MESA_SHADER_CACHE_DISABLE', 'true')
        result = subprocess.run([str(harness), str(tmp / 'scale.spv')], env=env)
        if result.returncode == 77:
            print('Runtime skipped: Vulkan device lacks required features')
        elif result.returncode:
            raise SystemExit(result.returncode)
        else:
            run(harness, optimized, env=env)
print('Shader C++ checks passed')
