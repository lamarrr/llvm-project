#!/usr/bin/env python3
"""Test every implemented shader feature; advanced stages have rejection tests."""
import argparse
import json
import os
import re
from pathlib import Path
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
BIN = ROOT / 'build/bin'
parser = argparse.ArgumentParser()
parser.add_argument('--runtime', action='store_true')
parser.add_argument('--host-cxx', default='c++')
args = parser.parse_args()
checks = 0


def expect(condition, message):
    # Checks remain active under python -O.
    if not condition:
        raise AssertionError(message)


def run(*command, **kwargs):
    return subprocess.run([str(x) for x in command], cwd=ROOT, check=True, **kwargs)


def validate(output, entry):
    global checks
    expect(struct.unpack('<II', output.read_bytes()[:8]) ==
           (0x07230203, 0x00010400), f'{entry}: wrong SPIR-V version')
    run(BIN / 'spirv-val', '--target-env', 'vulkan1.3', '--scalar-block-layout', output)
    reflection = json.loads(Path(str(output) + '.json').read_text())
    expect(reflection['entry'] == entry, f'{entry}: reflection names wrong entry')
    expect(reflection['target'] == 'spirv1.4-unknown-vulkan1.3', 'wrong profile')
    assembly = run(BIN / 'spirv-dis', output, capture_output=True, text=True).stdout
    for capability in ('PhysicalStorageBufferAddresses', 'VulkanMemoryModel'):
        expect(f'OpCapability {capability}' in assembly, f'missing {capability}')
    for extension in ('SPV_KHR_physical_storage_buffer', 'SPV_KHR_vulkan_memory_model'):
        expect(extension in reflection['spirv_extensions'], f'missing {extension}')
    expect({'bufferDeviceAddress', 'descriptorIndexing', 'vulkanMemoryModel',
            'scalarBlockLayout'} <= set(reflection['required_features']), 'missing profile features')
    expect('OpAsmINTEL' not in assembly, 'unexpected vendor inline assembly')
    checks += 1
    return reflection, assembly


# Fail when a newly declared public builtin/intrinsic lacks independent tests.
# Expected types, values and operations remain explicitly written in fixtures.
builtin_source = (ROOT / 'clang/include/clang/Basic/SPIRVShaderBuiltins.def').read_text()
public_builtins = set(re.findall(r'^SPIRV_SHADER_BUILTIN\((\w+),', builtin_source, re.M))
catalog = (ROOT / 'clang/test/SemaSPIRVShader/catalog.cpp').read_text()
tested_builtins = set(re.findall(r'^(?:SCALAR|ARRAY)\((\w+),', catalog, re.M))
expect(public_builtins == tested_builtins,
       f'Scope catalog coverage differs: {public_builtins ^ tested_builtins}')
sema_source = (ROOT / 'clang/lib/Sema/SemaSPIRVShader.cpp').read_text()
math_names = set(re.findall(r'^  SHADER_MATH\((\w+)\);', sema_source, re.M))
math_fixture = (ROOT / 'shader-cxx/tests/fixtures/math-all.cpp').read_text()
math_calls = set(re.findall(r'\b(\w+f)\(', math_fixture))
expect(math_names == math_calls, f'Math coverage differs: {math_names ^ math_calls}')
fixture_paths = {path.relative_to(ROOT / 'shader-cxx').as_posix()
                 for path in (ROOT / 'shader-cxx/tests/fixtures').glob('*.cpp')}


cc1 = [BIN / 'clang', '-cc1', '-triple', 'spirv1.4-unknown-vulkan1.3',
       '-std=c++20', '-fspirv-shader']
for test in sorted((ROOT / 'clang/test/SemaSPIRVShader').glob('*.cpp')):
    if test.name == 'host-gating.cpp':
        run(BIN / 'clang', '-cc1', '-triple', 'x86_64-unknown-linux-gnu',
            '-std=c++20', '-fsyntax-only', '-verify', test)
    else:
        run(*cc1, '-fsyntax-only', '-verify', test)
    checks += 1
for test in sorted((ROOT / 'clang/test/CodeGenSPIRVShader').glob('*.cpp')):
    ir = run(*cc1, '-emit-llvm', '-o', '-', test, capture_output=True).stdout
    run(BIN / 'FileCheck', test, input=ir)
    checks += 1

with tempfile.TemporaryDirectory(prefix='shader-cxx-') as directory:
    tmp = Path(directory)
    cases = [
        ('examples/scale.cpp', 'scale'), ('examples/alignment.cpp', 'align'),
        ('examples/stages.cpp', 'vertex'), ('examples/stages.cpp', 'fragment'),
        ('examples/math.cpp', 'math'), ('examples/uniform.cpp', 'uniform'),
        ('tests/fixtures/math-all.cpp', 'math_all'),
        ('tests/fixtures/pointers.cpp', 'pointers'),
        ('tests/fixtures/layout.cpp', 'layout'), ('tests/fixtures/shared.cpp', 'shared'),
        ('tests/fixtures/scope.cpp', 'scope'),
        ('tests/fixtures/fragment-scope.cpp', 'fragment_scope'),
        ('tests/fixtures/vertex-scope.cpp', 'vertex_scope'),
        ('tests/fixtures/global-interfaces.cpp', 'global_interfaces'),
        ('tests/fixtures/invocation-state.cpp', 'invocation_state'),
        ('tests/fixtures/cxx20.cpp', 'cxx20'),
    ]
    expect(fixture_paths == {source for source, _ in cases if source.startswith('tests/fixtures/')},
           'a feature fixture is missing from the compilation matrix')
    for source, entry in cases:
        output = tmp / f'{entry}.spv'
        run(BIN / 'clang++', '-fspirv-shader', '-std=c++20', f'-fspirv-entry={entry}',
            ROOT / 'shader-cxx' / source, '-o', output)
        reflection, assembly = validate(output, entry)
        if entry == 'scale':
            push = next(i for i in reflection['interfaces'] if i['kind'] == 'push_constant')
            expect(push['size'] == 16 and push['alignment'] == 8, 'push ABI mismatch')
            expect([f['offset'] for f in push['members']] == [0, 8], 'push offsets mismatch')
            expect(push['members'][0]['kind'] == 'device_address', 'pointer must be BDA')
            expect(reflection['workgroup_size'] == [64, 1, 1], 'wrong workgroup size')
        if entry == 'uniform':
            uniform = next(i for i in reflection['interfaces'] if i['kind'] == 'uniform')
            expect((uniform['set'], uniform['binding']) == (2, 7), 'binding mismatch')
            for decoration in ('DescriptorSet 2', 'Binding 7', 'Block'):
                expect(decoration in assembly, f'missing {decoration}')
        if entry == 'align':
            accesses = [line for line in assembly.splitlines() if 'Aligned' in line]
            expect(len(accesses) == 4 and
                   ['Aligned 16' in line for line in accesses] == [True, True, False, False],
                   'alignment must weaken after increment')
        if entry in ('vertex', 'fragment'):
            expect('Location 0' in assembly, 'missing varying location')
            expect(reflection['stage'] == entry, 'wrong stage')
            inputs = {i.get('builtin') for i in reflection['interfaces']}
            expect(({42, 43} if entry == 'vertex' else {17}) <= inputs, 'missing stage builtins')
            expect(('OpEntryPoint Vertex' if entry == 'vertex' else 'OpEntryPoint Fragment')
                   in assembly, 'wrong execution model')
            if entry == 'fragment':
                expect('OriginUpperLeft' in assembly and 'OpTypeBool' in assembly,
                       'fragment origin or boolean interface is wrong')
        if entry == 'math_all':
            for operation in ('Sqrt', 'Sin', 'Cos', 'Exp', 'Exp2', 'Log', 'Log2',
                              'FAbs', 'Floor', 'Ceil', 'Trunc', 'NMin', 'NMax', 'Fma'):
                expect(f' {operation} ' in assembly, f'missing math intrinsic {operation}')
        if entry == 'pointers':
            for operation in ('OpConvertPtrToU', 'OpPtrAccessChain', 'OpUGreaterThan', 'OpISub'):
                expect(operation in assembly, f'missing pointer operation {operation}')
            expect('OpPtrEqual' not in assembly and 'OpPtrNotEqual' not in assembly,
                   'physical pointers must compare as addresses in SPIR-V 1.4')
        if entry == 'layout':
            push = next(i for i in reflection['interfaces'] if i['kind'] == 'push_constant')
            fields = push['members']
            expect((push['size'], push['alignment']) == (48, 16), 'record size/ABI alignment mismatch')
            expect([f['offset'] for f in fields[:4]] == [0, 8, 12, 28], 'record offsets mismatch')
            expect(fields[2]['members'][0]['stride'] == 4 and fields[3]['stride'] == 4,
                   'nested array stride mismatch')
        if entry == 'shared':
            expect('OpTypePointer Workgroup' in assembly and ' Workgroup' in assembly,
                   'shared memory must use workgroup storage')
        if entry == 'scope':
            ids = {i.get('builtin') for i in reflection['interfaces']}
            expect({24, 26, 27, 28, 29, 36, 38, 40, 41} <= ids, 'missing compute builtins')
            expect(25 not in ids, 'workgroup_size must be synthesized from execution dimensions')
            expect(reflection['workgroup_size'] == [2, 3, 4], 'wrong dimensions')
        if entry == 'global_interfaces':
            expect({'input', 'output'} == {i['kind'] for i in reflection['interfaces']},
                   'global interface storage classes are wrong')
            expect({2, 3} == {i['location'] for i in reflection['interfaces']},
                   'global interface locations are wrong')
        if entry == 'invocation_state':
            expect('state' not in {i['name'] for i in reflection['interfaces']},
                   'ordinary global leaked into shader interface')
        if entry == 'vertex_scope':
            expect({42, 43, 4424, 4425, 4426, 4440} <=
                   {i.get('builtin') for i in reflection['interfaces']}, 'vertex builtins missing')
            expect({'shaderDrawParameters', 'multiview'} <=
                   set(reflection['required_features']), 'vertex feature requirements missing')
        if entry == 'fragment_scope':
            expect({7, 15, 16, 17, 18, 19, 23} <=
                   {i.get('builtin') for i in reflection['interfaces']}, 'fragment builtins missing')
            expect({'sampleRateShading', 'geometryShader'} <=
                   set(reflection['required_features']), 'missing fragment features')
            expect('SampleRateShading' in assembly and ' Flat' in assembly,
                   'sample builtins require capability and integer interpolation decoration')

    # Optimization can change pointer spellings and loop induction variables.
    # Validate every positive fixture through that separate compilation path.
    for source, entry in cases:
        output = tmp / f'{entry}-o2.spv'
        run(BIN / 'clang++', '-O2', '-fspirv-shader', '-std=c++20',
            f'-fspirv-entry={entry}', ROOT / 'shader-cxx' / source, '-o', output)
        validate(output, entry)
    optimized = tmp / 'scale-o2.spv'
    helper = tmp / 'helper.cpp'
    helper.write_text('template<class T> T twice(T x) { return x * 2; }\n'
                      'float helper(float x) { return twice(x); }\n')
    shader = tmp / 'shader.cpp'
    shader.write_text('float helper(float);\nstruct P { __storage__ float *data; };\n'
                      '__compute__ __workgroup_size__(1,1,1) void linked(__push_constant__ const P *p) '
                      '{ *p->data = helper(*p->data); }\n')
    for source in (helper, shader):
        run(BIN / 'clang++', '-fspirv-shader', '-std=c++20', '-c', source,
            '-o', source.with_suffix('.bc'))
    run(BIN / 'clang-spirv-link', helper.with_suffix('.bc'), shader.with_suffix('.bc'),
        '--entry=linked', '-o', tmp / 'linked.spv')
    _, assembly = validate(tmp / 'linked.spv', 'linked')
    expect('OpFunctionCall' not in assembly, 'helpers were not inlined')
    run(BIN / 'clang-spirv-link', helper.with_suffix('.bc'), shader.with_suffix('.bc'),
        '--entry=linked', '--emit-llvm', '-o', tmp / 'linked.ll')
    expect('define void @linked()' in (tmp / 'linked.ll').read_text(), 'LLVM inspection failed')
    checks += 1

    def reject(name, source, diagnostic, *options):
        global checks
        path = tmp / f'{name}.cpp'
        path.write_text(source)
        output = tmp / f'{name}.spv'
        result = subprocess.run([str(BIN / 'clang++'), '-std=c++20', '-fspirv-shader',
                                 *options, str(path), '-o', str(output)],
                                capture_output=True, text=True)
        expect(result.returncode != 0 and diagnostic in result.stderr,
               f'{name}: expected {diagnostic!r}, got:\n{result.stderr}')
        expect('PLEASE submit a bug report' not in result.stderr and 'LLVM ERROR' not in result.stderr,
               f'{name}: failure must be a diagnostic, not a crash')
        expect(not output.exists() and not Path(str(output) + '.json').exists(),
               f'{name}: failed compile published output')
        checks += 1

    compute = '__compute__ __workgroup_size__(1,1,1) '
    negatives = [
        ('recursive', compute + 'void recursive() { recursive(); }', 'recursion'),
        ('mutual', 'void b(); void a(){ b(); } void b(){ a(); }' + compute + 'void entry(){a();}', 'recursion'),
        ('assembly', compute + 'void entry(){asm volatile("OpNop");}', 'inline assembly lowering is not implemented'),
        ('unresolved', 'void missing();' + compute + 'void entry(){missing();}', 'unresolved shader function'),
        ('indirect', 'struct P{void (*f)();};' + compute + 'void entry(__push_constant__ const P *p){p->f();}', 'unresolved indirect call'),
        ('external', 'extern unsigned missing;' + compute + 'void entry(){unsigned x=missing;}', 'unresolved external shader variable'),
        ('global-init', 'int init(); int x=init();' + compute + 'void entry(){}', 'dynamic global initialization'),
        ('missing-size', '__compute__ void entry(){}', 'requires __workgroup_size__'),
        ('vertex-size', '__vertex__ __workgroup_size__(1,1,1) void entry(){}', 'does not have workgroup dimensions'),
        ('plain-param', compute + 'void entry(int x){}', 'entry parameters must be qualified'),
        ('push-count', compute + 'void entry(__push_constant__ int *a,__push_constant__ int *b){}', 'only one push-constant'),
        ('uniform-binding', compute + 'void entry(__uniform__ int *p){}', 'uniform interfaces require __binding__'),
        ('push-binding', compute + 'void entry(__binding__(0,1) __push_constant__ int *p){}', '__binding__ applies to descriptor resources'),
        ('output-value', '__vertex__ void entry(__output__(0) float x){}', 'output parameters must be pointers'),
        ('binding-arity', '__binding__(0) int x;', 'requires exactly 2 arguments'),
        ('binding-extra', '__binding__(0,1,2) int x;', 'requires exactly 2 arguments'),
        ('binding-negative', '__binding__(0,-1) int x;', 'non-negative integral'),
        ('binding-overflow', '__binding__(0,0x100000000ULL) int x;', '32-bit'),
        ('binding-runtime', 'int x; __binding__(0,x) int y;', 'integer constant'),
        ('binding-conflict', '__binding__(0,1) __binding__(0,2) int x;', 'conflicting descriptor bindings'),
        ('workgroup-zero', '__workgroup_size__(1,0,1) void f();', 'must be positive'),
        ('workgroup-negative', '__workgroup_size__(-1,1,1) void f();', 'non-negative integral'),
        ('workgroup-conflict', '__workgroup_size__(1,1,1) __workgroup_size__(2,1,1) void f();', 'conflicting workgroup sizes'),
        ('stage-conflict', '__compute__ __vertex__ void f();', 'conflicting entry-point stages'),
        ('return-type', '__compute__ int f();', 'returning void'),
        ('variadic', '__compute__ void f(int,...);', 'non-variadic free functions'),
        ('method', 'struct S{__compute__ void f();};', 'non-variadic free functions'),
        ('alignment-zero', '__storage__ __align__(0) float *p;', 'positive power of two'),
        ('alignment-negative', '__storage__ __align__((-2147483647-1)) float *p;', 'non-negative integral'),
        ('alignment-nonpower', '__storage__ __align__(3) float *p;', 'positive power of two'),
        ('memory-conflict', '__storage__ __uniform__ float *p;', 'conflicting'),
        ('input-negative', 'void f(__input__(-1) float x);', 'non-negative integral'),
        ('input-write', 'void f(__input__(0) float *p){*p=1;}', 'read-only'),
        ('alias-index', 'void f(){unsigned x=vertex_index;}', 'undeclared identifier'),
        ('array-w', 'void f(){unsigned x=global_invocation_id.w;}', 'is not a structure'),
        ('array-shorthand', 'void f(){unsigned x[3]; unsigned y=x.x;}', 'is not a structure'),
        ('flat-pending', '__fragment__ void f(__input__(0) __flat__ unsigned x){}', 'unknown type name'),
        ('no-entry', 'void helper(){}', 'no matching C++ shader entry point'),
        ('multiple', compute + 'void a(){}' + compute + 'void b(){}', 'multiple entry points'),
    ]
    for name, source, diagnostic in negatives:
        reject(name, source, diagnostic)
    for stage in ('tessellation_control', 'tessellation_evaluation', 'task', 'mesh',
                  'ray_generation', 'intersection', 'any_hit', 'closest_hit', 'miss', 'callable'):
        size = '__workgroup_size__(1,1,1)' if stage in ('task', 'mesh') else ''
        reject(stage, f'__{stage}__ {size} void entry(){{}}', f"stage '{stage}' is not implemented")
    reject('old-standard', compute + 'void entry(){}', 'requires C++20 or later', '-std=c++17')
    reject('wrong-profile', compute + 'void entry(){}', 'requires C++20 or later',
           '--target=spirv1.5-unknown-vulkan1.3')
    reject('unknown-entry', compute + 'void entry(){}', 'no matching C++ shader entry point',
           '-fspirv-entry=absent')

    incompatible = tmp / 'host.ll'
    incompatible.write_text('target triple = "x86_64-unknown-linux-gnu"\ndefine void @host() { ret void }\n')
    result = subprocess.run([str(BIN / 'clang-spirv-link'), str(incompatible),
                             '-o', str(tmp / 'host.spv')], capture_output=True, text=True)
    expect(result.returncode != 0 and 'incompatible target triple' in result.stderr,
           'mixed target modules accepted')
    expect(not (tmp / 'host.spv').exists(), 'incompatible input published output')
    checks += 1

    # Force validator rejection and verify neither artifact nor temporary output survives.
    output = tmp / 'rejected.spv'
    validator = tmp / 'reject-validator'
    validator.write_text('#!/bin/sh\nexit 1\n')
    validator.chmod(0o755)
    result = subprocess.run([str(BIN / 'clang-spirv-link'), str(helper.with_suffix('.bc')),
                             str(shader.with_suffix('.bc')), '--entry=linked',
                             f'--validator={validator}', '-o', str(output)],
                            capture_output=True, text=True)
    expect(result.returncode != 0 and 'Vulkan shader validation failed' in result.stderr,
           'validator failure not propagated')
    expect(not list(tmp.glob('rejected.spv*')), 'validator failure published artifacts')
    checks += 1

    # Failed validation must also leave a previous successful artifact intact.
    output.write_bytes((tmp / 'scale.spv').read_bytes())
    sidecar = Path(str(output) + '.json')
    sidecar.write_bytes(Path(str(tmp / 'scale.spv') + '.json').read_bytes())
    original_binary, original_json = output.read_bytes(), sidecar.read_bytes()
    result = subprocess.run([str(BIN / 'clang-spirv-link'), str(helper.with_suffix('.bc')),
                             str(shader.with_suffix('.bc')), '--entry=linked',
                             f'--validator={validator}', '-o', str(output)],
                            capture_output=True, text=True)
    expect(result.returncode != 0 and output.read_bytes() == original_binary and
           sidecar.read_bytes() == original_json, 'failed validation replaced existing output')
    expect(len(list(tmp.glob('rejected.spv*'))) == 2, 'temporary SPIR-V output leaked')
    checks += 1

    if args.runtime:
        harness = tmp / 'run-scale'
        run(args.host_cxx, '-std=c++20', '-I.shader-deps/Vulkan-Headers/include',
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
            checks += 2
            for entry, scenario in [('align', 'align'), ('math_all', 'math'),
                                    ('pointers', 'pointers'), ('shared', 'shared'),
                                    ('linked', 'linked'), ('invocation_state', 'invocation_state'),
                                    ('cxx20', 'scale')]:
                run(harness, tmp / f'{entry}.spv', entry, scenario, env=env)
                checks += 1
                if entry != 'linked':
                    run(harness, tmp / f'{entry}-o2.spv', entry, scenario, env=env)
                    checks += 1
print(f'Shader C++ checks passed ({checks} cases)')
