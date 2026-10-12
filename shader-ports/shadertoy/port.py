#!/usr/bin/env python3
"""Port pinned Image passes to explicit Shader C++ translation units."""

import argparse
import hashlib
import json
import os
import re
import subprocess
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
HERE = SCRIPT_DIR
ROOT = SCRIPT_DIR.parents[1]

GLSL_TYPE_NAMES = {
    **{f"vec{n}": f"f32x{n}" for n in (2, 3, 4)},
    **{f"ivec{n}": f"i32x{n}" for n in (2, 3, 4)},
    **{f"uvec{n}": f"u32x{n}" for n in (2, 3, 4)},
    **{f"bvec{n}": f"b8x{n}" for n in (2, 3, 4)},
    **{f"mat{n}": f"f32x{n}x{n}" for n in (2, 3, 4)},
}


def canonical_type_names(source):
    return re.sub(
        r"\b(?:[iub]?vec[234]|mat[234])\b",
        lambda match: GLSL_TYPE_NAMES[match.group()],
        source,
    )


def translate(source, channels):
    source = re.sub(r"^\s*#version[^\n]*", "", source, flags=re.M)
    result = subprocess.run(
        [
            str(ROOT / "build/bin/clang"),
            "-E",
            "-P",
            "-x",
            "c++",
            "-undef",
            "-",
            "-D__VERSION__=300",
            "-DGL_ES=1",
            "-DGL_FRAGMENT_PRECISION_HIGH=1",
        ],
        input=source,
        text=True,
        capture_output=True,
        timeout=30,
    )
    if result.returncode:
        raise ValueError("GLSL preprocessing failed: " + result.stderr)
    source = result.stdout
    source = re.sub(r"\bprecision\s+\w+\s+\w+\s*;", "", source)
    source = re.sub(r"\b(?:lowp|mediump|highp)\s+", "", source)
    # Preserve GLSL's float literal type rather than introducing C++ doubles.
    source = re.sub(
        r"(?<![\w.])((?:\d+\.\d*|\.\d+)(?:[eE][+-]?\d+)?|\d+[eE][+-]?\d+)(?![\w.])",
        r"\1f",
        source,
    )
    source = re.sub(r"\b(?:out|inout)\s+(\w+)\s+(\w+)", r"\1 &\2", source)
    source = re.sub(r"\bin\s+(?=\w+\s+\w+)", "", source)

    # Multi-component swizzles become explicit ordinary C++ methods.
    def swizzle(match):
        letters = match[1]
        indices = ["xyzw".index(c) if c in "xyzw" else "rgba".index(c) for c in letters]
        return ".swizzle<" + ",".join(map(str, indices)) + ">()"

    source = re.sub(r"\.([xyzw]{2,4}|[rgba]{2,4})\b", swizzle, source)
    # A swizzle assignment must write its original object, not a temporary.
    pattern = r"(\b\w+(?:\.\w+|\[[^\]\n]+\])*)\.swizzle<([0-3,]+)>\(\)\s*(\+=|-=|\*=|/=|=(?!=))\s*([^;]+);"

    def assignment(match):
        base, indices, op, rhs = match.groups()
        if len(set(indices.split(","))) != len(indices.split(",")):
            raise ValueError("Repeated-component writable swizzle")
        if any(token in base for token in ["++", "--", "("]):
            raise ValueError("Side-effectful writable swizzle base")
        value = rhs if op == "=" else f"{base}.swizzle<{indices}>() {op[0]} ({rhs})"
        return f"{base}.set_swizzle<{indices}>({value});"

    source = re.sub(pattern, assignment, source)
    if re.search(r"\.swizzle<[^>]+>\(\)\s*(?:[+*/-]=|=(?!=)|\+\+|--)", source):
        raise ValueError("Unsupported writable swizzle expression")
    source = re.sub(
        r"\.([xyzwrgba])\b",
        lambda m: (
            ".data["
            + str("xyzw".index(m[1]) if m[1] in "xyzw" else "rgba".index(m[1]))
            + "]"
        ),
        source,
    )
    # Channel handles stay local and flow explicitly through helper functions.
    source = re.sub(r"\biChannel([0-3])\b", r"__ctx.channel\1", source)
    names = set(
        re.findall(
            r"\b(?:void|float|int|uint|bool|vec[234]|[iub]vec[234]|mat[234]|\w+)\s+(\w+)\s*\([^;{}]*\)\s*\{",
            source,
        )
    )
    names -= {"if", "for", "while", "switch"}
    if "mainImage" not in names:
        raise ValueError("No mainImage definition")
    for name in sorted(names, key=len, reverse=True):
        # Definitions/prototypes receive a reference; calls receive the context.
        signature = rf"(\b(?!(?:return|case|throw|new)\b)\w+\s+{re.escape(name)}\s*\()([^;{{}}]*)(\)\s*(?:\{{|;))"
        source = re.sub(
            signature,
            lambda m: (
                m[1] + "Context &__ctx" + (", " + m[2] if m[2].strip() else "") + m[3]
            ),
            source,
        )
        calls = rf"\b{re.escape(name)}\s*\((?!Context\s*&__ctx)"
        source = re.sub(calls, name + "(__ctx, ", source)
        source = source.replace(name + "(__ctx, )", name + "(__ctx)")
    source = re.sub(r"\buint\b", "unsigned", source)
    return source


def port(item, output):
    record = json.loads((HERE / "upstream" / item["file"]).read_text())
    source = (record.get("common_code") or "") + "\n" + record["image_code"]
    channels = sorted(set(map(int, re.findall(r"\biChannel([0-3])\b", source))))
    inputs = {int(i["channel"]): i for i in record.get("image_inputs", [])}
    types = {
        c: {"cubemap": "texture_cube", "volume": "texture_3d"}.get(
            inputs.get(c, {}).get("ctype"), "texture_2d"
        )
        for c in channels
    }
    body = translate(source, channels)
    wrappers = {}
    for c in channels:
        wrappers[c] = {
            "texture_2d": "Texture2D",
            "texture_3d": "Texture3D",
            "texture_cube": "TextureCube",
        }[types[c]]
    methods = ""
    for native, wrapper, dimension in [
        ("texture_2d", "Texture2D", 2),
        ("texture_3d", "Texture3D", 3),
        ("texture_cube", "TextureCube", 3),
    ]:
        methods += f"struct {wrapper} {{ {native}<vec4> image; sampler sampling; }};\n"
        coord = f"vec{dimension}"
        coords = ",".join(f"uv.data[{i}]" for i in range(dimension))
        methods += f"vec4 texture({wrapper} t,{coord} uv) {{return t.image.sample(t.sampling,{coords});}}\n"
        methods += f"vec4 textureLod({wrapper} t,{coord} uv,float lod) {{return t.image.sample_lod(t.sampling,{coords},lod);}}\n"
        size = 2 if native == "texture_cube" else dimension
        methods += (
            f"ivec{size} textureSize({wrapper} t,int lod) {{auto size=t.image.dimensions(static_cast<unsigned>(lod));return ivec{size}("
            + ",".join(f"size[{i}]" for i in range(size))
            + ");}\n"
        )
        if native != "texture_cube":
            xy = ",".join(f"uv.data[{i}]" for i in range(dimension))
            methods += f"vec4 texelFetch({wrapper} t,ivec{dimension} uv) {{return t.image.load({xy});}}\n"
    methods += "using sampler2D=Texture2D;using sampler3D=Texture3D;using samplerCube=TextureCube;\n"
    context = (
        "struct Context {\n"
        + "".join(f"  {wrappers[c]} channel{c};\n" for c in channels)
        + "};\n"
    )

    # Only the zero-mip fetch signature currently maps to the native load API.
    # Nonzero/dynamic mip calls retain three arguments and fail overload lookup.
    def zero_fetch(text):
        matches = list(re.finditer(r"\btexelFetch\s*\(", text))
        for match in reversed(matches):
            start = match.end()
            depth = 1
            end = start
            while end < len(text) and depth:
                if text[end] == "(":
                    depth += 1
                elif text[end] == ")":
                    depth -= 1
                end += 1
            args = text[start : end - 1]
            # The mip is the last operand; retain the coordinate expression.
            zero = re.search(r",\s*0[uU]?\s*$", args)
            if zero:
                text = text[:start] + args[: zero.start()] + text[end - 1 :]
        return text

    body = zero_fetch(body)
    globals_ = """vec3 iResolution; vec4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame; float iChannelTime[4]; vec3 iChannelResolution[4];
"""
    params = "".join(
        f", __binding__(0,{c}) {types[c]}<vec4> channel{c}, __binding__(1,{c}) sampler sampling{c}"
        for c in channels
    )
    init = ",".join("{" + f"channel{c},sampling{c}" + "}" for c in channels)
    wrapper = f"""__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) vec4 *output{params}) {{
  iResolution=vec3(p->resolution_time.data[0],p->resolution_time.data[1],p->resolution_time.data[2]);
  gl_FragCoord=vec4(frag_coord[0],frag_coord[1],frag_coord[2],frag_coord[3]);\n  iTime=p->resolution_time.data[3];iMouse=vec4(p->mouse.data[0],p->mouse.data[1],p->mouse.data[2],p->mouse.data[3]);iDate=vec4(p->date.data[0],p->date.data[1],p->date.data[2],p->date.data[3]);
  iTimeDelta=p->time_delta;iFrameRate=p->frame_rate;iFrame=p->frame;iSampleRate=p->sample_rate;
  for(unsigned i=0;i<4;++i) {{iChannelTime[i]=p->channel_time[i];iChannelResolution[i]=vec3(p->channel_resolution[i].data[0],p->channel_resolution[i].data[1],p->channel_resolution[i].data[2]);}}
  Context __ctx{{{init}}};vec4 color;
  mainImage(__ctx,color,vec2(frag_coord[0],frag_coord[1]));*output=color;
}}
"""
    support_include = os.path.relpath(SCRIPT_DIR / "support.h", output)
    text = (
        f'// Port of {item["url"]}; author: {item["author"]}.\n'
        f'// Dataset license metadata: {item["license"]}; '
        f'original retained in upstream/{item["file"]}.\n'
        f'#include "{support_include}"\n'
        'using namespace glsl;\n'
        + methods
        + context
        + globals_
        + body
        + "\n"
        + wrapper
    )
    text = canonical_type_names(text)
    formatted = subprocess.run(
        [
            str(ROOT / "build/bin/clang-format"),
            "--style=file:" + str(ROOT / "shader-cxx/.clang-format"),
        ],
        input=text,
        text=True,
        capture_output=True,
        check=True,
        timeout=30,
    ).stdout
    (output / (Path(item["file"]).stem + ".cpp")).write_text(formatted)
    return {
        "id": item["id"],
        "status": "translated",
        "channels": channels,
        "resource_types": types,
        "functions": len(set(re.findall(r"Context &__ctx", body))),
    }


def main():
    global HERE
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--corpus-dir", type=Path, default=SCRIPT_DIR)
    parser.add_argument("--output-dir", type=Path)
    args = parser.parse_args()
    HERE = args.corpus_dir.resolve()
    args.output_dir = (args.output_dir or HERE / "ports").resolve()
    args.output_dir.mkdir(parents=True, exist_ok=True)
    manifest = json.loads((HERE / "upstream/manifest.json").read_text())
    stems = [Path(item["file"]).stem.casefold() for item in manifest["files"]]
    if len(stems) != len(set(stems)):
        raise ValueError("Shader filenames must be unique ignoring case")
    results = []
    for item in manifest["files"]:
        data = (HERE / "upstream" / item["file"]).read_bytes()
        if hashlib.sha256(data).hexdigest() != item["sha256"]:
            raise ValueError("Source hash mismatch: " + item["id"])
        target = args.output_dir / (Path(item["file"]).stem + ".cpp")
        target.unlink(missing_ok=True)
        try:
            results.append(port(item, args.output_dir))
        except Exception as error:
            results.append(
                {"id": item["id"], "status": "translation_failed", "error": str(error)}
            )
    (args.output_dir / "translation.json").write_text(
        json.dumps(results, indent=2) + "\n"
    )
    print(
        "Translated",
        sum(r["status"] == "translated" for r in results),
        "of",
        len(results),
    )


if __name__ == "__main__":
    main()
