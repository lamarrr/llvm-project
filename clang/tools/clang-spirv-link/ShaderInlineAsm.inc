//===-- ShaderInlineAsm.inc - Assemble direct shader SPIR-V ---------------===//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

static std::pair<StringRef, StringRef> shaderAsmAssignment(StringRef Line) {
  Line = Line.trim();
  if (Line.starts_with("%")) {
    size_t End = Line.find_first_of(" \t=");
    if (End != StringRef::npos) {
      StringRef Tail = Line.substr(End).ltrim();
      if (Tail.consume_front("="))
        return {Line.take_front(End), Tail.ltrim()};
    }
  }
  return {{}, Line};
}

struct ShaderAsmOperand {
  std::string Kind, Literal;
  int Arg, Result;
  unsigned Alignment;
};
struct ShaderAsmSite {
  std::string Text;
  SmallVector<ShaderAsmOperand> Operands;
  unsigned Outputs;
};

static bool collectShaderAsm(Function &Entry,
                             SmallVectorImpl<ShaderAsmSite> &Sites) {
  for (Instruction &I : instructions(Entry)) {
    auto *Call = dyn_cast<CallInst>(&I);
    if (!Call || !Call->getCalledFunction() ||
        Call->getCalledFunction()
                ->getFnAttribute("spirv.cxx.intrinsic")
                .getValueAsString() != "InlineAsm")
      continue;
    auto *MD = Call->getMetadata("spirv.cxx.asm");
    if (!MD || MD->getNumOperands() != 3)
      return fail("missing direct SPIR-V asm metadata");
    ShaderAsmSite Site;
    Site.Text = cast<MDString>(MD->getOperand(0))->getString().str();
    Site.Outputs = mdNumber(MD, 2);
    for (auto &Op : cast<MDNode>(MD->getOperand(1))->operands()) {
      auto *Node = cast<MDNode>(Op);
      if (auto *Pointee = dyn_cast<ValueAsMetadata>(Node->getOperand(5))) {
        int Arg =
            mdconst::extract<ConstantInt>(Node->getOperand(1))->getSExtValue();
        if (Arg >= 0)
          if (auto *Pointer = dyn_cast<Instruction>(Call->getArgOperand(Arg)))
            Pointer->setMetadata("spirv.cxx.pointee",
                                 MDNode::get(Entry.getContext(), Pointee));
      }
      Site.Operands.push_back(
          {cast<MDString>(Node->getOperand(0))->getString().str(),
           cast<MDString>(Node->getOperand(4))->getString().str(),
           int(mdconst::extract<ConstantInt>(Node->getOperand(1))
                   ->getSExtValue()),
           int(mdconst::extract<ConstantInt>(Node->getOperand(2))
                   ->getSExtValue()),
           mdNumber(Node, 3)});
    }
    Sites.push_back(std::move(Site));
    Call->setArgOperand(
        0, ConstantInt::get(Call->getArgOperand(0)->getType(), Sites.size()));
  }
  return true;
}

static bool expandShaderAsm(SmallVectorImpl<char> &Binary,
                            ArrayRef<ShaderAsmSite> Sites,
                            StringRef ExecutableDirectory) {
  if (Sites.empty())
    return true;
  struct Instance {
    unsigned Site, Result, Type;
    SmallVector<unsigned> Args;
  };
  SmallVector<Instance> Instances;
  DenseMap<unsigned, unsigned> ValueTypes, Constants;
  DenseMap<unsigned, SmallVector<unsigned>> StructTypes;
  DenseSet<unsigned> Types;
  auto Word = [&](size_t Offset) {
    return support::endian::read32le(Binary.data() + Offset);
  };
  for (size_t Offset = 20; Offset < Binary.size();) {
    unsigned Count = Word(Offset) >> 16, Op = Word(Offset) & 65535;
    if (!Count || Offset + Count * 4 > Binary.size())
      return fail("malformed SPIR-V before inline assembly expansion");
    if (Op >= 19 && Op <= 39 && Count >= 2) {
      Types.insert(Word(Offset + 4));
      if (Op == 30)
        for (unsigned I = 2; I < Count; ++I)
          StructTypes[Word(Offset + 4)].push_back(Word(Offset + I * 4));
    }
    Offset += Count * 4;
  }
  SmallVector<char> Clean(Binary.begin(), Binary.begin() + 20);
  for (size_t Offset = 20; Offset < Binary.size();) {
    unsigned Count = Word(Offset) >> 16, Op = Word(Offset) & 65535;
    if (Count >= 3 && Types.contains(Word(Offset + 4)))
      ValueTypes[Word(Offset + 8)] = Word(Offset + 4);
    if (Op == 43 && Count == 4)
      Constants[Word(Offset + 8)] = Word(Offset + 12);
    if (Op == 12 && Count >= 6 && Word(Offset + 16) == 65535) {
      unsigned Site = Constants.lookup(Word(Offset + 20));
      if (!Site || Site > Sites.size())
        return fail("invalid direct SPIR-V asm site");
      Instance X{Site - 1, Word(Offset + 8), Word(Offset + 4), {}};
      for (unsigned I = 6; I < Count; ++I)
        X.Args.push_back(Word(Offset + I * 4));
      Instances.push_back(std::move(X));
      // Disassemble a grammar-valid temporary marker. It is never validated,
      // reflected or published; the marker is replaced with the actual code.
      char Undef[12];
      support::endian::write32le(Undef, (3u << 16) | 1u);
      support::endian::write32le(Undef + 4, Word(Offset + 4));
      support::endian::write32le(Undef + 8, Word(Offset + 8));
      Clean.append(Undef, Undef + 12);
    } else {
      Clean.append(Binary.begin() + Offset,
                   Binary.begin() + Offset + Count * 4);
    }
    Offset += Count * 4;
  }
  SmallString<256> Directory;
  if (auto EC = sys::fs::createUniqueDirectory("clang-spirv-asm", Directory))
    return fail(EC.message());
  auto Remove = scope_exit([&] { sys::fs::remove_directories(Directory); });
  std::string Input = (Twine(Directory) + "/input.spv").str();
  std::string Assembly = (Twine(Directory) + "/shader.spvasm").str();
  std::string Output = (Twine(Directory) + "/output.spv").str();
  std::error_code EC;
  {
    raw_fd_ostream File(Input, EC);
    if (EC)
      return fail(EC.message());
    File.write(Clean.data(), Clean.size());
  }
  auto Find = [&](StringRef Name) {
    auto Program = sys::findProgramByName(Name, {ExecutableDirectory});
    return Program ? Program : sys::findProgramByName(Name);
  };
  auto Disassembler = Find("spirv-dis"), Assembler = Find("spirv-as");
  if (!Disassembler || !Assembler)
    return fail("direct SPIR-V asm requires spirv-dis and spirv-as beside "
                "Clang or on PATH");
  if (sys::ExecuteAndWait(*Disassembler,
                          {*Disassembler, "--raw-id", Input, "-o", Assembly}))
    return fail("cannot disassemble direct SPIR-V asm intermediate");
  auto Buffer = MemoryBuffer::getFile(Assembly);
  if (!Buffer)
    return fail(Buffer.getError().message());
  SmallVector<StringRef> Lines;
  (*Buffer)->getBuffer().split(Lines, '\n');
  StringMap<std::string> Definitions;
  for (StringRef Line : Lines) {
    auto [ID, RHS] = Line.trim().split(" = ");
    if (!RHS.empty() && RHS.starts_with("OpType"))
      Definitions.try_emplace(RHS, ID.str());
  }
  SmallVector<std::string> Sections[6];
  StringMap<std::string> Replacements;
  auto ID = [](unsigned V) { return "%" + std::to_string(V); };
  for (unsigned N = 0; N < Instances.size(); ++N) {
    const auto &X = Instances[N];
    const auto &Site = Sites[X.Site];
    std::string Prefix = "%cxx_asm_" + std::to_string(N) + "_";
    SmallVector<unsigned> OutputTypes;
    int Results = 0;
    for (auto &Op : Site.Operands)
      Results = std::max(Results, Op.Result + 1);
    if (Results > 1) {
      OutputTypes = StructTypes.lookup(X.Type);
      if (int(OutputTypes.size()) != Results)
        return fail("incorrect direct SPIR-V asm output type");
    } else if (Results == 1) {
      OutputTypes.push_back(X.Type);
    }
    SmallVector<std::string> OutputIDs;
    for (int I = 0; I < Results; ++I)
      OutputIDs.push_back(
          Results == 1 ? ID(X.Result) : Prefix + "output_" + std::to_string(I));
    StringMap<std::string> Aliases;
    std::string Body;
    SmallVector<StringRef> Statements;
    StringRef(Site.Text).split(Statements, '\n');
    for (StringRef Statement : Statements) {
      std::string Expanded;
      Statement = Statement.trim();
      if (Statement.empty())
        continue;
      bool Quoted = false, Escaped = false;
      for (size_t I = 0; I < Statement.size();) {
        char Ch = Statement[I++];
        if (Quoted) {
          if (Ch == '$' && I < Statement.size() && Statement[I] == '$')
            ++I;
          Expanded += Ch;
          if (Ch == '"' && !Escaped)
            Quoted = false;
          Escaped = Ch == '\\' && !Escaped;
          continue;
        }
        if (Ch == '"') {
          Quoted = true;
          Expanded += Ch;
          continue;
        }
        if (Ch == ';') {
          Expanded += Statement.substr(I - 1);
          break;
        }
        if (Ch == '%' && I < Statement.size() &&
            (isAlnum(Statement[I]) || Statement[I] == '_')) {
          size_t Start = I;
          while (I < Statement.size() &&
                 (isAlnum(Statement[I]) || Statement[I] == '_'))
            ++I;
          std::string Name = Prefix + Statement.slice(Start, I).str();
          auto A = Aliases.find(Name);
          Expanded += A == Aliases.end() ? Name : A->second;
        } else if (Ch == '$') {
          if (I < Statement.size() && Statement[I] == '$') {
            Expanded += '$';
            ++I;
            continue;
          }
          bool Braced = I < Statement.size() && Statement[I] == '{';
          if (Braced)
            ++I;
          size_t Start = I;
          while (I < Statement.size() && isDigit(Statement[I]))
            ++I;
          unsigned Operand;
          if (Start == I ||
              Statement.slice(Start, I).getAsInteger(10, Operand) ||
              Operand >= Site.Operands.size())
            return fail("invalid direct SPIR-V asm operand placeholder");
          char Modifier = 0;
          if (Braced) {
            if (I + 2 >= Statement.size() || Statement[I++] != ':')
              return fail("invalid direct SPIR-V asm operand modifier");
            Modifier = Statement[I++];
            if (Statement[I++] != '}')
              return fail("invalid direct SPIR-V asm operand modifier");
          }
          const auto &Op = Site.Operands[Operand];
          unsigned Argument = 0;
          // Metadata indices include the site tag; the binary Args do not.
          if (Op.Arg > 0) {
            if (unsigned(Op.Arg) > X.Args.size())
              return fail("missing direct SPIR-V asm argument");
            Argument = X.Args[Op.Arg - 1];
          }
          bool DefinesOutput = Expanded.empty() &&
                               Statement.substr(I).ltrim().starts_with("=") &&
                               Operand < Site.Outputs;
          if (Modifier == 't') {
            unsigned Type = Op.Result >= 0 ? OutputTypes[Op.Result]
                                           : ValueTypes.lookup(Argument);
            if (!Type)
              return fail("missing SPIR-V type for asm operand");
            Expanded += ID(Type);
          } else if (Modifier == 'a') {
            if (Op.Kind != "m")
              return fail(
                  "SPIR-V asm alignment modifier requires an m operand");
            Expanded += std::to_string(Op.Alignment);
          } else if (Modifier)
            return fail("SPIR-V asm supports only t and a operand modifiers");
          else if (Op.Kind == "i")
            Expanded += Op.Literal;
          else if (Op.Result >= 0 && (DefinesOutput || !Argument))
            Expanded += OutputIDs[Op.Result];
          else if (Argument)
            Expanded += ID(Argument);
          else
            return fail("missing direct SPIR-V asm operand value");
        } else {
          Expanded += Ch;
        }
      }
      StringRef E(Expanded);
      auto [Defined, RHS] = shaderAsmAssignment(E);
      StringRef Operation = RHS.empty() ? E : RHS;
      StringRef Opcode = Operation.split(' ').first;
      int Section = -1;
      if (Opcode == "OpCapability")
        Section = 0;
      else if (Opcode == "OpExtension")
        Section = 1;
      else if (Opcode == "OpExtInstImport")
        Section = 2;
      else if (Opcode == "OpName" || Opcode == "OpMemberName" ||
               Opcode == "OpString" || Opcode.starts_with("OpSource"))
        Section = 3;
      else if (Opcode.starts_with("OpDecorate") ||
               Opcode == "OpMemberDecorate" || Opcode == "OpDecorationGroup" ||
               Opcode.starts_with("OpGroupDecorate"))
        Section = 4;
      else if (Opcode.starts_with("OpType") ||
               Opcode.starts_with("OpConstant") ||
               Opcode.starts_with("OpSpecConstant"))
        Section = 5;
      else if (Opcode == "OpMemoryModel" || Opcode == "OpEntryPoint" ||
               Opcode == "OpFunction" || Opcode == "OpFunctionEnd")
        return fail(
            "SPIR-V inline asm cannot redefine the module or its functions");
      if (Section == 5 && Opcode.starts_with("OpType") &&
          Opcode != "OpTypeStruct" && Opcode != "OpTypeArray" &&
          Opcode != "OpTypeRuntimeArray") {
        auto Existing = Definitions.find(Operation);
        if (Existing != Definitions.end()) {
          Aliases[Defined] = Existing->second;
          continue;
        }
        Definitions[Operation] = Defined.str();
      }
      std::string Emission =
          Defined.empty() ? Expanded : (Defined + " = " + RHS).str();
      if (Section >= 0) {
        if (!is_contained(Sections[Section], Emission))
          Sections[Section].push_back(Emission);
      } else {
        Body += Emission + "\n";
      }
    }
    // Declarations may be referenced by annotations before their definitions.
    // Resolve aliases after the whole template has been classified, too.
    auto ResolveAliases = [&](StringRef Input) {
      std::string Output;
      bool Quoted = false, Escaped = false;
      for (size_t I = 0; I < Input.size();) {
        char Ch = Input[I++];
        if (Quoted) {
          Output += Ch;
          if (Ch == '"' && !Escaped)
            Quoted = false;
          Escaped = Ch == '\\' && !Escaped;
        } else if (Ch == '"') {
          Quoted = true;
          Output += Ch;
        } else if (Ch == '%') {
          size_t Start = I - 1;
          while (I < Input.size() && (isAlnum(Input[I]) || Input[I] == '_'))
            ++I;
          StringRef ID = Input.slice(Start, I);
          auto Alias = Aliases.find(ID);
          Output += Alias == Aliases.end() ? ID.str() : Alias->second;
        } else
          Output += Ch;
      }
      return Output;
    };
    Body = ResolveAliases(Body);
    for (auto &Section : Sections)
      for (auto &Line : Section)
        Line = ResolveAliases(Line);
    if (Results > 1) {
      Body += ID(X.Result) + " = OpCompositeConstruct " + ID(X.Type);
      for (auto &Output : OutputIDs)
        Body += " " + Output;
      Body += "\n";
    }
    Replacements[ID(X.Result) + " = OpUndef " + ID(X.Type)] = std::move(Body);
  }
  std::string Text;
  bool Written[6] = {};
  auto Flush = [&](unsigned Section) {
    if (Written[Section])
      return;
    for (auto &Line : Sections[Section])
      Text += Line + "\n";
    Written[Section] = true;
  };
  for (StringRef Line : Lines) {
    StringRef L = Line.trim();
    auto [Defined, RHS] = shaderAsmAssignment(L);
    StringRef Opcode = (RHS.empty() ? L : RHS).split(' ').first;
    if (!L.empty() && !L.starts_with(";") && Opcode != "OpCapability")
      Flush(0);
    if (Opcode == "OpExtInstImport" || Opcode == "OpMemoryModel")
      Flush(1);
    if (Opcode == "OpMemoryModel")
      Flush(2);
    if (Opcode.starts_with("OpDecorate") ||
        Opcode.starts_with("OpMemberDecorate") || Opcode.starts_with("OpType"))
      Flush(3);
    if (Opcode.starts_with("OpType"))
      Flush(4);
    if (Opcode == "OpFunction")
      Flush(5);
    auto Replacement = Replacements.find(L);
    Text += Replacement == Replacements.end() ? Line.str() + "\n"
                                              : Replacement->second;
  }
  {
    raw_fd_ostream File(Assembly, EC);
    if (EC)
      return fail(EC.message());
    File << Text;
  }
  if (sys::ExecuteAndWait(*Assembler,
                          {*Assembler, "--target-env", "spv1.4",
                           "--preserve-numeric-ids", Assembly, "-o", Output}))
    return fail("direct SPIR-V asm assembly failed; output was not published");
  auto Result = MemoryBuffer::getFile(Output);
  if (!Result)
    return fail(Result.getError().message());
  StringRef Bytes = (*Result)->getBuffer();
  Binary.assign(Bytes.begin(), Bytes.end());
  return true;
}
