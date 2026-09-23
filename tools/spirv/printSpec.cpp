// Copyright 2014-2026 LunarG, Inc.
// SPDX-License-Identifier: Apache-2.0

//
// Author: John Kessenich, LunarG
//

//
// 1) Programmatically fill in instruction/operand information.
//    This can be used for disassembly, printing documentation, etc.
//
// 2) Print documentation from this parameterization.
//

#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4129)

#include "doc.h"
#include "printSpec.h"

#include "unified1/spirv.hpp"

#include <cstdlib>
#include <stdio.h>
#include <string>
#include <algorithm>
#include <set>
#include <map>
#include <cstring>

#define LINE_BREAK " +\n"
#define GAP "\n\n"
#define NOTE GAP "*Note:* "
#define DEPRECATED_STANDALONE "<<Deprecation, Deprecated>> "
#define DEPRECATED(INSTEAD) "<<Deprecation, Deprecated>> (" INSTEAD "). "

namespace spv {

extern EnumValues ExecutionModelParams;
extern EnumValues AddressingParams;
extern EnumValues MemoryParams;
extern EnumValues ExecutionModeParams;
extern EnumValues StorageParams;
extern EnumValues SamplerAddressingModeParams;
extern EnumValues SamplerFilterModeParams;
extern EnumValues ImageFormatParams;
extern EnumValues ImageChannelOrderParams;
extern EnumValues ImageChannelTypeParams;
extern EnumValues ImageOperandsParams;
extern EnumValues FPFastMathParams;
extern EnumValues FPRoundingModeParams;
extern EnumValues LinkageTypeParams;
extern EnumValues DecorationParams;
extern EnumValues BuiltInParams;
extern EnumValues DimensionalityParams;
extern EnumValues FuncParamAttrParams;
extern EnumValues AccessQualifierParams;
extern EnumValues GroupOperationParams;
extern EnumValues LoopControlParams;
extern EnumValues SelectionControlParams;
extern EnumValues FunctionControlParams;
extern EnumValues MemorySemanticsParams;
extern EnumValues MemoryAccessParams;
extern EnumValues ScopeParams;
extern EnumValues KernelEnqueueFlagsParams;
extern EnumValues KernelProfilingInfoParams;
extern EnumValues CapabilityParams;
extern EnumValues FPDenormModeParams;
extern EnumValues FPOperationModeParams;
extern EnumValues PackedVectorFormatParams;
extern EnumValues FPEncodingParams;
// TODO(vlom): this should be described in the grammar,
// although we do not have a proper field for this as parameters isn't correct.
std::map<FPEncoding, std::set<int>> FPEncodingWidths;

std::set<Op> InstrPageBreaks;

#define SEE_CLIENT_API "See the client API specification for more detail. "
#define SEE_CLIENT_API_ABOUT(about) "See the client API specification for more detail " about ". "

#define V_FP               "must be a vector of <<Floating,_floating-point type_>>. "
#define V_FP_32            "must be a vector of 32-bit <<Floating,_floating-point type_>>. "
#define S_FP_(rep)         "must be a <<Floating,_floating-point type_>> scalar" rep ". "
#define S_FP               S_FP_("")
#define S_754_FP_32        "must be a 32-bit <<Floating,_floating-point type_>> scalar using the IEEE 754 encoding. "
#define S_FP_32            "must be a 32-bit <<Floating,_floating-point type_>> scalar. "
#define M_FP               "must be an <<OpTypeMatrix,*OpTypeMatrix*>> whose _Column Type_ is a vector of <<Floating,_floating-point type_>>. "
#define SV_FP_(size, rep)  "must be a scalar or vector of " size " <<Floating,_floating-point type_>>" rep ". "
#define SV_FP              SV_FP_("", "")
#define SV_FP_32           SV_FP_(" 32-bit", "")
#define SV_754_FP          SV_FP_("", " using the IEEE 754 encoding")
#define SV_754_FP_32       SV_FP_("32-bits", " using the IEEE 754 encoding")
#define SV_FP_I            "must be a scalar or vector of <<Floating,_floating-point type_>> or <<Integer,_integer type_>>. "
#define SV_FP_I_32         "must be a scalar or vector of 32-bit <<Floating,_floating-point type_>> or 32-bit <<Integer,_integer type_>>. "
#define SV_FP_I_B          "must be a scalar or vector of <<Floating,_floating-point type_>>, <<Integer,_integer type_>>, or <<Boolean,_Boolean type_>>. "
#define V4_FP_I            "must be a vector of four components of <<Floating,_floating-point type_>> or <<Integer,_integer type_>>. "
#define PSV_FP_I           "must be a pointer to a scalar or vector of <<Floating,_floating-point type_>> or <<Integer,_integer type_>>. "
#define S_I                "must be an <<Integer,_integer type_>> scalar. "
#define S_I_32             "must be a 32-bit <<Integer,_integer type_>> scalar. "
#define V4_U               "must be a vector of four components of <<Integer,_integer type_>> scalar, whose _Width_ operand is 32 and whose _Signedness_ operand is 0. "
#define S_FP_I             "must be a scalar of <<Integer,_integer type_>> or <<Floating,_floating-point type_>>. "
#define SV_I_(size)        "must be a " size "scalar or vector of <<Integer,_integer type_>>. "
#define SV_I               SV_I_("")
#define SV_I_32            SV_I_("32-bit ")
#define S_U                "must be a scalar of <<Integer,_integer type_>>, whose _Signedness_ operand is 0. "
#define SV_U               "must be a scalar or vector of <<Integer,_integer type_>>, whose _Signedness_ operand is 0. "
#define S_B                "must be a <<Boolean,_Boolean type_>> scalar. "
#define SV_B               "must be a scalar or vector of <<Boolean,_Boolean type_>>. "
#define RESULT_V_FP        "_Result Type_ " V_FP
#define RESULT_S_FP        "_Result Type_ " S_FP
#define RESULT_M_FP        "_Result Type_ " M_FP
#define RESULT_SV_FP_I     "_Result Type_ " SV_FP_I
#define RESULT_V4_FP_I     "_Result Type_ " V4_FP_I
#define RESULT_SV_FP       "_Result Type_ " SV_FP
#define RESULT_SV_754_FP   "_Result Type_ " SV_754_FP
#define RESULT_SV_I        "_Result Type_ " SV_I
#define RESULT_S_I         "_Result Type_ " S_I
#define RESULT_S_U         "_Result Type_ " S_U
#define RESULT_SV_U        "_Result Type_ " SV_U
#define RESULT_S_B         "_Result Type_ " S_B
#define RESULT_SV_B        "_Result Type_ " SV_B
#define RESULT_S_FP_I      "_Result Type_ " S_FP_I
#define RESULT_PTR         "_Result Type_ must be an <<OpTypePointer,*OpTypePointer*>>. "
#define RESULT_PHYS_PTR    "_Result Type_ must be a <<PhysicalPointerType, physical pointer type>>. "
#define RESULT_STRUCT_SV_I "_Result Type_ must be from <<OpTypeStruct,*OpTypeStruct*>>. The struct must have two members, and the two members must be the same type. The member type " SV_I
#define RESULT_STRUCT_SV_U "_Result Type_ must be from <<OpTypeStruct,*OpTypeStruct*>>. The struct must have two members, and the two members must be the same type. The member type " SV_U

#define SAME_COMP_RESULT   "It must have the same number of components as _Result Type_. "
#define SAME_WIDTHS_RESULT "The component width must equal the component width in _Result Type_. "
#define DIFF_WIDTHS_RESULT "The component width must not equal the component width in _Result Type_. "
#define DIFF_TYPES_RESULT  "The component type must not equal the component type in _Result Type_. "

#define SAME_POINTERS      "The types of _Operand 1_ and _Operand 2_ must be <<OpTypePointer, *OpTypePointer*>> of the same type. "

#define OPERAND_UNARY_INTEGER     "_Operand's_ type " SV_I
#define OPERANDS_INTEGERS(ops)    "The type of each " ops " " SV_I ops " must have the same number of components. "
#define MATCHING_BINARY_INTEGERS  "The type of _Operand 1_ and _Operand 2_ " SV_I " They must have the same number of components as _Result Type_. They must have the same component width as _Result Type_. "
#define MATCHING_REL_BINARY_SV_I  "The type of _Operand 1_ and _Operand 2_ " SV_I " They must have the same component width, and they must have the same number of components as _Result Type_. "
#define MATCHING_REL_BINARY_SV_FP "The type of _Operand 1_ and _Operand 2_ " SV_FP " They must have the same type, and they must have the same number of components as _Result Type_. "
#define MATCHING_WIDTH_TYPE(op)   "The number of components and bit width of the type of " op " must be the same as in _Result Type_. "
#define MATCHING(operand)         "The type of " operand " must be the same as _Result Type_. "
#define MATCHINGS(operand)        "The types of " operand " must be the same as _Result Type_. "
#define MATCHING_OPERANDS(operand1, operand2)        "The type of " operand1 " must be the same as " operand2 ". "
#define MATCHING_BINARY_OPERANDS  "The types of _Operand 1_ and _Operand 2_ both must be the same as _Result Type_. "
#define MATCHING_COLUMN           "must be an <<OpTypeMatrix, *OpTypeMatrix*>> whose _Column Type_ is _Result Type_. "
#define MATCHING_VECTOR_COMPS     "must be a vector with the same _Component Type_ as the _Component Type_ in _Result Type_. "
#define MATCHING_MATRIX_COMPS     "must be a matrix with the same _Component Type_ as the _Component Type_ in _Result Type_. "
#define MATCHING_STRUCT_BINARY_INTEGERS "_Operand 1_ and _Operand 2_ must have the same type as the members of _Result Type_. "

#define PER_COMPONENT            "Results are computed per component. "
#define PER_BIT                  "Results are computed per component, and within each component, per bit. "
#define SCALAR_BOOL_RESULT       "_Result Type_ must be a <<Boolean,_Boolean type_>> scalar. "
#define MATCHING_COMP_COUNT      "It must have the same number of components as _Result Type_. "
#define OPERAND2_ZERO_UNDEFINED  "The resulting value of a component is <<Poison,poison>> if that component of _Operand 2_ is 0. "
 #define OPERAND2_UNDEFINED_BEHAVIOR "<<UndefinedBehavior,Behavior is undefined>> if any component of _Operand 2_ is 0. "
#define OPERAND2_UNDEFINED_SDIV  "<<UndefinedBehavior,Behavior is undefined>> if any component of _Operand 2_ is -1 and the same component of _Operand 1_ is the minimum representable " \
                                 "value for the operands' type, causing signed overflow. "
#define MODREM_RESULT(signOp)    "Otherwise, the result is the <<Remainder, _remainder_>> _r_ of _Operand 1_ divided by _Operand 2_ " \
                                 "where if _r_ {ne} 0, the sign of _r_ is the same as the sign of " signOp ". "

#define FRAGMENT_ONLY           GAP "This instruction is only valid in the *Fragment* <<Execution_Model, Execution Model>>. "
#define IMPLICIT_DERIVATIVE     FRAGMENT_ONLY "In addition, it consumes an implicit derivative that can be affected by code motion. "

#define REMOVABLE "This has no semantic impact and can safely be removed from a module. "

#define SPECIFIED_AS_ID(SAME_AS)  "Same as the " SAME_AS ", but using an _<id>_ operand instead of a literal. " \
                                  "The operand is consumed as unsigned and " S_I
#define SPECIFIED_AS_IDS(SAME_AS) "Same as the " SAME_AS ", but using _<id>_ operands instead of literals. " \
                                  "The operands are consumed as unsigned and each " S_I

#define ADDITIONAL_OPERAND_BITS  "Bits that are set indicate whether an additional operand follows, as described by the table. " \
                                 "If there are multiple following operands indicated, they are ordered: " \
                                 "Those indicated by smaller-numbered bits appear first. "

#define EXPLICITLY_LAID_OUT_STORAGE_CLASS "<<CompositeType, Composite>> objects in this storage class must have a type with an <<ExplicitLayout,explicit layout>>. "

#define TANGLED_EXEC_WAIT_FOR(SCOPE)                                           \
  "An invocation will not execute a <<DynamicInstance, dynamic instance>> of " \
  "this instruction (_X'_) until all invocations in " SCOPE " "                \
  "have executed all <<DynamicInstance, dynamic instances>> that are "         \
  "<<ProgramOrder, program-ordered before>> _X'_. "

#define QUAD_TANGLED_EXEC_WAIT  TANGLED_EXEC_WAIT_FOR("its quad")

#define DERIVATIVE_TANGLED_EXEC_WAIT  TANGLED_EXEC_WAIT_FOR("its <<DerivativeGroup, derivative group>>")

#define TANGLED_EXEC_WAIT TANGLED_EXEC_WAIT_FOR("its <<ScopeRestrictedTangle, scope restricted tangle>>")

// Allocates a new C-style string and fills it with the concatenation
// of prefix and suffix.
char* StrConcat(const char* prefix, const char* suffix)
{
    char* result = reinterpret_cast<char*>(malloc(strlen(prefix) + strlen(suffix) + 1));
    strcpy(result, prefix);
    strcat(result, suffix);
    return result;
}

// Set up all the parameterizing descriptions of the opcodes, operands, etc.
void ParameterizeSpec()
{
    // Enumerant capabilities

    OperandClassParams[OperandSource].desc = "The source language is for debug purposes only, with no semantics that affect the meaning of other parts of the module.";
    //OperandClassParams[OperandExecutionModel].desc = ;
    //OperandClassParams[OperandAddressing].desc = ;
    //OperandClassParams[OperandMemory].desc = ;
    OperandClassParams[OperandExecutionMode].desc = "Declare the modes an <<EntryPoint,entry point>> executes in. "
                                                    "All *Extra Operands* that are _<id>s_ must be the _<id>s_ of <<ConstantInstruction, constant instructions>> unless otherwise stated. "
                                                    "It is invalid to apply the same execution mode more than once to any entry point unless explicitly allowed below for a specific execution mode.";

    OperandClassParams[OperandStorage].desc =
        "Class of storage for declared variables. "
        "<<Intermediate, Intermediate values>> do not form a storage class, "
        "and unless stated otherwise, "
        "storage class-based restrictions are not restrictions on intermediate objects and their types.";
    OperandClassParams[OperandDimensionality].desc = "Dimensionality of an image. "
                                                     "Some uses require capabilities beyond the enabling capabilities, for example "
                                                     "where the type's _Sampled_ operand is 2, or _Arrayed_ operand is 1. "
                                                     "See the <<Capabilities,capabilities>> section for more detail. ";
    OperandClassParams[OperandSamplerAddressingMode].desc = "Addressing mode for creating constant samplers.";
    OperandClassParams[OperandSamplerFilterMode].desc = "Filter mode for creating constant samplers.";
    OperandClassParams[OperandSamplerImageFormat].desc = "Declarative image format.";
    OperandClassParams[OperandImageChannelOrder].desc =
        "The image channel orders that result from <<OpImageQueryOrder,*OpImageQueryOrder*>>.";
    OperandClassParams[OperandImageChannelDataType].desc =
        "Image channel data types that result from <<OpImageQueryFormat,*OpImageQueryFormat*>>.";
    OperandClassParams[OperandImageOperands].desc =
        "Provides additional operands to sampling, or getting texels from, an image. "
        ADDITIONAL_OPERAND_BITS
        "At least one bit must be set (*None* is invalid).";

    OperandClassParams[OperandFPFastMath].desc = "Enables fast math operations which are otherwise unsafe. "
                                        GAP
                                        "Only valid on"
                                        GAP
                                        " - <<OpFAdd,*OpFAdd*>>, <<OpFSub,*OpFSub*>>, <<OpFMul,*OpFMul*>>, "
                                        "   <<OpFDiv,*OpFDiv*>>, <<OpFRem,*OpFRem*>>, and <<OpFMod,*OpFMod*>> instructions\n"
                                        " - <<Unified, Missing before>> *version 1.6*:\n"
                                        "   ** the <<OpFNegate,*OpFNegate*>> instruction\n"
                                        "   ** the <<OpOrdered,*OpOrdered*>>, <<OpUnordered,*OpUnordered*>>, "
                                        "      <<OpFOrdEqual,*OpFOrdEqual*>>, <<OpFUnordEqual,*OpFUnordEqual*>>, "
                                        "      <<OpFOrdNotEqual,*OpFOrdNotEqual*>>, <<OpFUnordNotEqual,*OpFUnordNotEqual*>>, "
                                        "      <<OpFOrdLessThan,*OpFOrdLessThan*>>, <<OpFUnordLessThan,*OpFUnordLessThan*>>, "
                                        "      <<OpFOrdGreaterThan,*OpFOrdGreaterThan*>>, <<OpFUnordGreaterThan,*OpFUnordGreaterThan*>>, "
                                        "      <<OpFOrdLessThanEqual,*OpFOrdLessThanEqual*>>, <<OpFUnordLessThanEqual,*OpFUnordLessThanEqual*>>, "
                                        "      <<OpFOrdGreaterThanEqual,*OpFOrdGreaterThanEqual*>>, "
                                        "      and <<OpFUnordGreaterThanEqual,*OpFUnordGreaterThanEqual*>> instructions\n"
                                        "   ** <<OpExtInst,*OpExtInst*>> extended instructions, where expressly permitted by the extended instruction set in use.";

    OperandClassParams[OperandFPRoundingMode].desc =
        "Associate a rounding mode to a floating-point conversion instruction.\n\n";

    OperandClassParams[OperandLinkageType].desc = "Associate a linkage type to functions or global variables. See <<Linkage, linkage>>.";

    OperandClassParams[OperandAccessQualifier].desc = "Defines the access permissions.\n\n";

    OperandClassParams[OperandFuncParamAttr].desc = "Adds additional information to the return type and to each parameter of a function. "
        GAP
        "Only one of *Zext* and *Sext* can be used to decorate the same _<id>_, "
        "and no attribute may be used multiple times on the same _<id>_. "
        "Otherwise, multiple function parameter attributes can be applied to the same _<id>_.";

    OperandClassParams[OperandDecoration].desc =
        "Decorations add additional information to an _<id>_ or member of a structure. "
        GAP
        "It is invalid to decorate any given _<id>_ or structure member more than one time with the same <<Decoration, decoration>>, "
        "unless explicitly allowed below for a specific decoration.";

    OperandClassParams[OperandBuiltIn].desc =
        "Used when <<Decoration,*Decoration*>> is *BuiltIn*. "
        "Apply to:"
        GAP
        " - The result _<id>_ of the *OpVariable* declaration of the built-in variable,\n"
        " - A structure-type member, if the built-in is a member of a structure, or\n"
        " - " DEPRECATED_STANDALONE ": a <<ConstantInstruction, constant instruction>>, when the built-in is a constant.\n"
        GAP
        "As stated per entry below, these have additional semantics and constraints specified by the client API. "
        GAP
        "For all the declarations of all the global variables and constants "
        "statically referenced by the entry-point's call tree, "
        "within any specific storage class it is invalid to decorate with a specific *BuiltIn* more than once. "
        GAP
        "Application to a <<ConstantInstruction, constant instruction>> has previously "
        "been used to define the workgroup size with specialization constants in some client APIs. "
        "As of version 1.6, all client APIs should instead use the *LocalSizeId* <<Execution_Mode,execution mode>>. ";

    //OperandClassParams[OperandSelect].desc = ;

    OperandClassParams[OperandLoop].desc = ADDITIONAL_OPERAND_BITS;

    //OperandClassParams[OperandFunction].desc = ;

    OperandClassParams[OperandMemorySemantics].desc =
        "The value's type must be a 32-bit integer scalar. "
        "This value is expected to be formed only from the bits in the table below, "
        "where at most one of these four bits can be set: "
        "*Acquire*, *Release*, *AcquireRelease*, or *SequentiallyConsistent*. "
        "If validation rules or the client API require a constant _<id>_, "
        "it is invalid for the value to not be formed this expected way. "
        "If non-constant _<id>_ are allowed, <<UndefinedBehavior,behavior is undefined>> when the value is not formed this expected way. "
        GAP
        "Requesting both *Acquire* and *Release* semantics is done by setting the *AcquireRelease* bit, not by setting two bits. "
        GAP
        "Memory semantics define memory-order constraints, and on what storage classes those constraints apply to. "
        "The memory order constrains the allowed orders in which memory operations in this <<Invocation,invocation>> "
        "are made visible to another invocation. "
        "The storage classes specify to which subsets of memory these constraints are to be applied. "
        "Storage classes not selected are not being constrained. ";

    OperandClassParams[OperandMemoryOperands].desc =
        "Provides additional operands to the listed memory instructions. "
        ADDITIONAL_OPERAND_BITS
        "An instruction needing two masks must first provide the first mask followed by the first mask's additional operands, "
        "and then provide the second mask followed by the second mask's additional operands. ";

    OperandClassParams[OperandScope].desc = "Must be an _<id>_ of a 32-bit integer scalar. "
        "Its value is expected to be one of the values in the table below. "
        "If validation rules or the client API require a constant _<id>_, it is invalid for it to not be one of these values. "
        "If non-constant _<id>_ are allowed, <<UndefinedBehavior,behavior is undefined>> if _<id>_ is not one of these values. "
        GAP
        "If labeled as a memory scope, it specifies the distance of synchronization from the current <<Invocation,invocation>>. "
        "If labeled as an execution scope, it specifies the set of executing invocations taking part in the operation. "
        "Other usages (neither memory nor execution) of scope are possible, and each such usage defines what scope "
        "means in its context.";

    OperandClassParams[OperandGroupOperation].desc = "Defines the class of operation for <<Group, group>> and <<Non-Uniform, non-uniform group>> instructions.";

    OperandClassParams[OperandKernelEnqueueFlags].desc = "Specify when the child kernel begins execution. "
        NOTE "Implementations are not required to honor this flag. "
        "Implementations may not schedule kernel launch earlier than the point specified by this flag, however. Used by <<OpEnqueueKernel,*OpEnqueueKernel*>>.";

    OperandClassParams[OperandKernelProfilingInfo].desc = "Specifies the profiling information to be queried. "
        "Used by <<OpCaptureEventProfilingInfo,*OpCaptureEventProfilingInfo*>>.";

    OperandClassParams[OperandCapability].desc = "Capabilities a module can declare it uses. "
        GAP
        "All used capabilities need to be declared, either explicitly with <<OpCapability, *OpCapability*>> or "
        "implicitly through the *Implicitly Declares* column: "
        "If a capability defined with <<ValidDefined, statically expressed rules>> is used, "
        "it is invalid to not declare it. "
        "If a capability defined in terms of dynamic behavior is used, "
        "<<UndefinedBehavior,behavior is undefined>> unless the capability is declared. "
        "The *Implicitly Declares* column lists additional capabilities that are all implicitly declared when the "
        "*Capability* entry is explicitly or implicitly declared. "
        "It is not necessary, but allowed, to explicitly declare an implicitly declared capability. "
        GAP
        "See the <<Capabilities,capabilities>> section for more detail.";

    // See
    // https://github.com/intel/llvm/blob/39fa9b0cbfbae88327118990a05c5b387b56d2ef/sycl/doc/extensions/SPIRV/SPV_INTEL_float_controls2.asciidoc
    OperandClassParams[OperandFPDenormMode].desc = "Floating point denormalized handling mode.";
    OperandClassParams[OperandFPOperationMode].desc = "Floating point operation mode.";

    OperandClassParams[OperandFPEncoding].desc = "Specifies an alternative floating point encoding. "
                                                 GAP
                                                 "The _Width(s)_ column specifies the set of valid width the encoding operand can be used with. "
                                                 "If no value is provided, the valid widths for the operand are defined by the client API. "
                                                 "Otherwise, the _Width_ operand of <<OpTypeFloat, *OpTypeFloat*>> must match one the specified values.";

    FPEncodingWidths[FPEncodingBFloat16KHR].insert(16);
    FPEncodingWidths[FPEncodingFloat8E4M3EXT].insert(8);
    FPEncodingWidths[FPEncodingFloat8E5M2EXT].insert(8);

    AddressingParams[AddressingModelPhysical32].desc = "Indicates a 32-bit module, where the address width is equal to 32 bits.";

    AddressingParams[AddressingModelPhysical64].desc = "Indicates a 64-bit module, where the address width is equal to 64 bits.";

    AddressingParams[AddressingModelPhysicalStorageBuffer64].desc = "Indicates that pointers with a "
        "<<Storage_Class, storage class>> of *PhysicalStorageBuffer* are physical pointer types with an "
        "address width of 64 bits, while pointers to all other storage classes are logical.";

    MemoryParams[MemoryModelSimple].desc = DEPRECATED("use *GLSL450*") LINE_BREAK "Memory model is undefined.";

    MemoryParams[MemoryModelGLSL450].desc = "Memory model needed by later versions of GLSL and ESSL. Works across multiple versions.";

    MemoryParams[MemoryModelOpenCL].desc = "OpenCL memory model.";

    MemoryParams[MemoryModelVulkan].desc = "*Vulkan memory model*, as specified by the client API. "
        "This memory model must be declared if and only if the *VulkanMemoryModel* <<Capability, capability>> "
        "is declared.";

    ExecutionModelParams[ExecutionModelVertex].desc = "Vertex shading stage.";

    ExecutionModelParams[ExecutionModelTessellationControl].desc = "Tessellation control (or hull) shading stage.";

    ExecutionModelParams[ExecutionModelTessellationEvaluation].desc = "Tessellation evaluation (or domain) shading stage.";

    ExecutionModelParams[ExecutionModelGeometry].desc = "Geometry shading stage.";

    ExecutionModelParams[ExecutionModelFragment].desc = "Fragment shading stage.";

    ExecutionModelParams[ExecutionModelGLCompute].desc = "Graphical compute shading stage.";

    ExecutionModelParams[ExecutionModelKernel].desc = "Compute kernel.";

    StorageParams[StorageClassUniformConstant].desc = "Shared externally, visible across all <<Invocation,invocations>>. "
                                                      "Graphics uniform memory. OpenCL constant memory. "
                                                      "Variables declared with this storage class are read-only. "
                                                      "They may have initializers, as allowed by the client API.";

    StorageParams[StorageClassInput].desc =
        "Input from pipeline. Visible only by the current <<Invocation,invocation>>. "
        "Variables declared with this storage class are read-only, and must not have initializers.";

    StorageParams[StorageClassUniform].desc = "Shared externally, visible across all <<Invocation,invocations>>. " EXPLICITLY_LAID_OUT_STORAGE_CLASS;

    StorageParams[StorageClassOutput].desc = "Output to pipeline. Visible only by the current <<Invocation,invocation>>.";

    StorageParams[StorageClassWorkgroup].desc =  "Visible across all <<Invocation,invocations>> within a workgroup.";

    StorageParams[StorageClassCrossWorkgroup].desc = "Visible across all <<Invocation,invocations>>.";

    StorageParams[StorageClassPrivate].desc = "Visible only by the current <<Invocation,invocation>>.";

    StorageParams[StorageClassFunction].desc = "Visible only by the current <<Invocation,invocation>>. For memory allocation within a function with specific lifetime. See <<OpVariable, *OpVariable*>> for more information.";

    StorageParams[StorageClassGeneric].desc = "For generic pointers, which overload the *Function*, *Workgroup*, and *CrossWorkgroup* <<Storage_Class,Storage Classes>>.";

    StorageParams[StorageClassPushConstant].desc =
        "For holding push-constant memory, visible across all <<Invocation,invocations>>. "
        "Intended to contain a small bank of values pushed from the client API. "
        "Variables declared with this storage class are read-only, and must not have initializers. "
        EXPLICITLY_LAID_OUT_STORAGE_CLASS;

    StorageParams[StorageClassAtomicCounter].desc = "For holding atomic counters. Visible only by the current <<Invocation,invocation>>.";

    StorageParams[StorageClassImage].desc = "For holding <<ImageTerm,image>> memory.";

    StorageParams[StorageClassStorageBuffer].desc = "Shared externally, readable and writable, "
        "visible across all <<Invocation,invocations>>. "
        EXPLICITLY_LAID_OUT_STORAGE_CLASS;

    StorageParams[StorageClassPhysicalStorageBuffer].desc = "Shared externally, readable and writable, "
        "visible across all <<Invocation,invocations>>. Uses physical addressing. "
        EXPLICITLY_LAID_OUT_STORAGE_CLASS;

    // Sampler Filter & Addressing mode capabilities
    SamplerAddressingModeParams[SamplerAddressingModeNone].desc = "The image coordinates used to sample elements of the image refer to a location inside "
                                                                  "the image, otherwise the results are <<Poison,_poison_>>. ";
    SamplerAddressingModeParams[SamplerAddressingModeClampToEdge].desc = "Out-of-range image coordinates are clamped to the extent.";
    SamplerAddressingModeParams[SamplerAddressingModeClamp].desc = "Out-of-range image coordinates result in a border color.";
    SamplerAddressingModeParams[SamplerAddressingModeRepeat].desc = "Out-of-range image coordinates are wrapped to the valid range. Must only be used with normalized coordinates.";
    SamplerAddressingModeParams[SamplerAddressingModeRepeatMirrored].desc = "Flip the image coordinate at every integer junction. Must only be used with normalized coordinates.";

    SamplerFilterModeParams[SamplerFilterModeNearest].desc = "Use filter nearest mode when performing a read image operation.";
    SamplerFilterModeParams[SamplerFilterModeLinear].desc = "Use filter linear mode when performing a read image operation.";

    // Image lookup operands
    ImageOperandsParams[ImageOperandsBiasShift].desc = "A following operand is the bias added to the implicit level of detail. Only valid with implicit-lod instructions. It " S_754_FP_32
                                                       "This must only be used with an <<OpTypeImage,*OpTypeImage*>> that has a <<Dim,_Dim_>> operand of *1D*, *2D*, *3D*, or *Cube*, "
                                                       "and the _MS_ operand must be 0.";
    ImageOperandsParams[ImageOperandsLodShift].desc =  "A following operand is the explicit level-of-detail to use. Only valid with explicit-lod instructions. "
                                                       "For sampling operations, it " S_754_FP_32
                                                       "For fetch operations, it " S_I_32
                                                       "This must only be used with an <<OpTypeImage,*OpTypeImage*>> that has a <<Dim,_Dim_>> operand of *1D*, *2D*, *3D*, or *Cube*, "
                                                       "and the _MS_ operand must be 0.";
    ImageOperandsParams[ImageOperandsGradShift].desc =
        "Two following operands are _dx_ followed by _dy_. These are explicit derivatives in the _x_ and _y_ "
        "direction to use in computing level of detail. "
        "Each is a scalar or vector containing (_du/dx_[, _dv/dx_] [, _dw/dx_]) and (_du/dy_[, _dv/dy_] [, _dw/dy_]). "
        "The number of components of each must equal the number of components in _Coordinate_, "
        "minus the _array layer_ component, if present. "
        "Only valid with explicit-lod instructions. They " SV_754_FP_32
        "This must only be used with an <<OpTypeImage,*OpTypeImage*>> that has an _MS_ operand of 0. "
        "It is invalid to set both the *Lod* and *Grad* bits.";
    ImageOperandsParams[ImageOperandsConstOffsetShift].desc =
        "A following operand is added to (_u_, _v_, _w_) before texel lookup. "
        "It must be an _<id>_ of a <<ConstantInstruction,_constant instruction_>> "
        "with a 32-bit scalar or vector <<Integer, integer type>>. "
        "It is invalid for these to be outside a target-dependent allowed range. "
        "The number of components must equal the number of components in _Coordinate_, minus the _array layer_ "
        "component, if present. "
        "Not valid with the *Cube* <<Dim, dimension>>. "
        "An instruction must specify at most one of the *ConstOffset*, *Offset*, and *ConstOffsets* image operands.";
    ImageOperandsParams[ImageOperandsOffsetShift].desc =
        "A following operand is added to (_u_, _v_, _w_) before texel lookup. "
        "It " SV_I_32
        "It is invalid for these to be outside a target-dependent allowed range. "
        "The number of components must equal the number of components in _Coordinate_, minus the _array layer_ component, if present. "
        "Not valid with the *Cube* <<Dim, dimension>>. "
        "An instruction must specify at most one of the *ConstOffset*, *Offset*, and *ConstOffsets* image operands.";
    ImageOperandsParams[ImageOperandsConstOffsetsShift].desc =
        "A following operand is _Offsets_. _Offsets_ must be an _<id>_ of a "
        "<<ConstantInstruction,_constant instruction_>> "
        "making an array of size four of vectors of two 32-bits integer components. "
        "Each gathered texel is identified by adding one of these array elements to the (_u_, _v_) sampled location. "
        "It is invalid for these to be outside a target-dependent allowed range. "
        "Only valid with <<OpImageGather,*OpImageGather*>>, <<OpImageDrefGather,*OpImageDrefGather*>>, "
        "<<OpImageSparseGather,*OpImageSparseGather*>>, or <<OpImageSparseDrefGather,*OpImageSparseDrefGather*>>. "
        "Not valid with the *Cube* <<Dim, dimension>>. "
        "An instruction must specify at most one of the *ConstOffset*, *Offset*, and *ConstOffsets* image operands.";
    ImageOperandsParams[ImageOperandsSampleShift].desc =
        "A following operand is the sample number of the sample to use. Only valid with <<OpImageFetch,*OpImageFetch*>>, "
        "<<OpImageRead,*OpImageRead*>>, <<OpImageWrite,*OpImageWrite*>>, <<OpImageSparseFetch, *OpImageSparseFetch*>>, and "
        "<<OpImageSparseRead, *OpImageSparseRead*>>. "
        "The *Sample* operand must be used if and only if the underlying <<OpTypeImage, *OpTypeImage*>> has _MS_ of 1. "
        "It " S_I_32;

    ImageOperandsParams[ImageOperandsMinLodShift].desc = "A following operand is the minimum level-of-detail to use when accessing the image. "
                                                         "Only valid with *Implicit* instructions and *Grad* instructions. It " S_754_FP_32
                                                         "This must only be used with an <<OpTypeImage,*OpTypeImage*>> that has a <<Dim,_Dim_>> operand of *1D*, *2D*, *3D*, or *Cube*, "
                                                         "and the _MS_ operand must be 0.";

    ImageOperandsParams[ImageOperandsMakeTexelAvailableShift].desc =
        "Perform an availability operation on the texel locations after the store. "
        "A following operand is the memory <<Scope_-id-, scope>> that controls the availability operation. "
        "Requires *NonPrivateTexel* to also be set. "
        "Only valid with <<OpImageWrite,*OpImageWrite*>>.";

    ImageOperandsParams[ImageOperandsMakeTexelVisibleShift].desc =
        "Perform a visibility operation on the texel locations before the load. "
        "A following operand is the memory <<Scope_-id-, scope>> that controls the visibility operation. "
        "Requires *NonPrivateTexel* to also be set. "
        "Only valid with <<OpImageRead,*OpImageRead*>> and <<OpImageSparseRead,*OpImageSparseRead*>>.";

    ImageOperandsParams[ImageOperandsNonPrivateTexelShift].desc =
        "The image access obeys inter-thread ordering, as specified by the client API.";

    ImageOperandsParams[ImageOperandsVolatileTexelShift].desc =
        "This access cannot be eliminated, duplicated, or combined with other accesses.";

#define SIGN_ZERO_EXTEND_INSTRUCTIONS "<<OpImageRead, *OpImageRead*>>, <<OpImageSparseRead, *OpImageSparseRead*>>, " \
                                      "<<OpImageWrite, *OpImageWrite*>>, and <<OpImageSampleExplicitLod, *OpImageSampleExplicitLod*>>"

#define TEXEL_VALUE_TYPE " - for sparse images, the texel value type is the second member of the result type.\n" \
                         " - for <<OpImageWrite, *OpImageWrite*>> the texel value type is type of the _Texel_ operand.\n" \
                         " - otherwise, the texel value type is the result type.\n"

    ImageOperandsParams[ImageOperandsSignExtendShift].desc = "The texel value is converted to the target value via sign extension. "
                                                             "Only valid if the texel value type is a scalar or vector of "
                                                             "<<Integer,_integer type_>>:"
                                                             GAP
                                                             TEXEL_VALUE_TYPE
                                                             GAP
                                                             "It is invalid to set both the *ZeroExtend* and *SignExtend* bits.";
    ImageOperandsParams[ImageOperandsZeroExtendShift].desc = "The texel value is converted to the target value via zero extension. "
                                                             "Only valid if the texel value type is a scalar or vector of "
                                                             "<<Integer,_integer type_>> with signedness of 0:"
                                                             GAP
                                                             TEXEL_VALUE_TYPE
                                                             GAP
                                                             "It is invalid to set both the *ZeroExtend* and *SignExtend* bits.";
    ImageOperandsParams[ImageOperandsNontemporalShift].desc = "Hints that the accessed texels are not likely to be accessed again in the near future.";

    // fast math flags capabilities
    #define FAST_MATH_UNDEF "If this assumption does not hold then the operation returns <<Poison,poison>>. "

    FPFastMathParams[FPFastMathModeNotNaNShift].desc = "Assume parameters and result are not NaN. " FAST_MATH_UNDEF;
    FPFastMathParams[FPFastMathModeNotInfShift].desc = "Assume parameters and result are not +/- Inf. " FAST_MATH_UNDEF;
    FPFastMathParams[FPFastMathModeNSZShift].desc  = "Treat the sign of a zero parameter or result as insignificant.";
    FPFastMathParams[FPFastMathModeAllowRecipShift].desc = "Allow the usage of reciprocal rather than perform a division.";
    FPFastMathParams[FPFastMathModeFastShift].desc = "Allow algebraic transformations according to real-number associative and distributive algebra. This flag implies all the others.";

    // fp rounding mode capabilities
    FPRoundingModeParams[FPRoundingModeRTE].desc = "Round to nearest even.";
    FPRoundingModeParams[FPRoundingModeRTZ].desc = "Round towards zero.";
    FPRoundingModeParams[FPRoundingModeRTP].desc = "Round towards positive infinity.";
    FPRoundingModeParams[FPRoundingModeRTN].desc = "Round towards negative infinity.";

    // linkage types
    LinkageTypeParams[LinkageTypeExport].desc = "Accessible by other modules as well.";
    LinkageTypeParams[LinkageTypeImport].desc = "A declaration of a global variable or a function that exists in another module.";

    // function argument types
    FuncParamAttrParams[FunctionParameterAttributeZext].desc =
        "Zero extend the value, if needed.";
    FuncParamAttrParams[FunctionParameterAttributeSext].desc =
        "Sign extend the value, if needed.";
    FuncParamAttrParams[FunctionParameterAttributeByVal].desc =
        "Pass the parameter by value to the function. "
        "Only valid for pointer parameters (not for ret value). ";
    FuncParamAttrParams[FunctionParameterAttributeSret].desc =
        "The parameter is the "
        "address of a structure that is the return value of the function in the source program. "
        "Only applicable to the first parameter, which must be a pointer parameter.";
    FuncParamAttrParams[FunctionParameterAttributeNoAlias].desc =
        "The memory pointed to by a pointer parameter is not accessed via pointer values that are not derived from "
        "this pointer parameter. "
        "Only valid for pointer parameters. Not valid on return values.";
    FuncParamAttrParams[FunctionParameterAttributeNoCapture].desc =
        "The parameter is not copied into a location that is accessible after returning from the callee. "
        "Only valid for pointer parameters. Not valid on return values.";
    FuncParamAttrParams[FunctionParameterAttributeNoWrite].desc =
        "The parameter is not used to write to the memory pointed to. "
        "Only valid for pointer parameters. Not valid on return values.";
    FuncParamAttrParams[FunctionParameterAttributeNoReadWrite].desc =
        "The parameter is not dereferenced, either to read or write the memory pointed to. "
        "Only valid for pointer parameters. Not valid on return values.";

    // function argument types
    AccessQualifierParams[AccessQualifierReadOnly].desc =  "A read-only object.";
    AccessQualifierParams[AccessQualifierWriteOnly].desc = "A write-only object.";
    AccessQualifierParams[AccessQualifierReadWrite].desc = "A readable and writable object.";

    ExecutionModeParams[ExecutionModeInvocations].desc =
        "_Number of invocations_ is an unsigned 32-bit integer number of times to invoke the geometry stage for each "
        "input primitive received. "
        "The default is to run once for each input primitive. "
        "It is invalid to specify a value greater than the target-dependent maximum. "
        "Only valid with the *Geometry* <<Execution_Model,Execution Model>>.";

    ExecutionModeParams[ExecutionModeSpacingEqual].desc = "Requests the tessellation primitive generator to divide edges into a collection of equal-sized segments. "
                                                          "Only valid with one of the tessellation <<Execution_Model,Execution Models>>.";

    ExecutionModeParams[ExecutionModeSpacingFractionalEven].desc = "Requests the tessellation primitive generator to divide edges into an even number of equal-length segments plus two additional shorter fractional segments. "
                                                                   "Only valid with one of the tessellation <<Execution_Model,Execution Models>>.";

    ExecutionModeParams[ExecutionModeSpacingFractionalOdd].desc = "Requests the tessellation primitive generator to divide edges into an odd number of equal-length segments plus two additional shorter fractional segments. "
                                                                  "Only valid with one of the tessellation <<Execution_Model,Execution Models>>.";

    ExecutionModeParams[ExecutionModeVertexOrderCw].desc = "Requests the tessellation primitive generator to generate triangles in clockwise order. "
                                                           "Only valid with one of the tessellation <<Execution_Model,Execution Models>>.";

    ExecutionModeParams[ExecutionModeVertexOrderCcw].desc = "Requests the tessellation primitive generator to generate triangles in counter-clockwise order. "
                                                            "Only valid with one of the tessellation <<Execution_Model,Execution Models>>.";

    ExecutionModeParams[ExecutionModePixelCenterInteger].desc = "Pixels appear centered on whole-number pixel offsets. E.g., the coordinate (0.5, 0.5) appears to move to (0.0, 0.0). "
                                                            "Only valid with the *Fragment* <<Execution_Model,Execution Model>>. "
                                                            "If a *Fragment* entry point does not have this set, pixels appear centered at offsets of (0.5, 0.5) from whole numbers";

    ExecutionModeParams[ExecutionModeOriginUpperLeft].desc = "The coordinates decorated by *FragCoord* appear to originate in the upper left, "
                                                             "and increase toward the right and downward. "
                                                             "Only valid with the *Fragment* <<Execution_Model,Execution Model>>.";

    ExecutionModeParams[ExecutionModeOriginLowerLeft].desc = "The coordinates decorated by *FragCoord* appear to originate in the lower left, "
                                                             "and increase toward the right and upward. "
                                                             "Only valid with the *Fragment* <<Execution_Model,Execution Model>>.";

    ExecutionModeParams[ExecutionModeEarlyFragmentTests].desc = "Fragment tests are to be performed before fragment shader execution. "
                                                                "Only valid with the *Fragment* <<Execution_Model,Execution Model>>.";

    ExecutionModeParams[ExecutionModePointMode].desc = "Requests the tessellation primitive generator to generate a point for each distinct vertex in the subdivided primitive, "
                                                       "rather than to generate lines or triangles. "
                                                       "Only valid with one of the tessellation <<Execution_Model,Execution Models>>.";

    ExecutionModeParams[ExecutionModeXfb].desc = "This stage runs in transform feedback-capturing mode and this module is responsible for describing the transform-feedback setup. "
                                                 "See the *XfbBuffer*, *Offset*, and *XfbStride* <<Decoration,*Decorations*>>.";

    ExecutionModeParams[ExecutionModeDepthReplacing].desc =
        "This mode declares that this entry point dynamically writes the *FragDepth*-decorated variable. "
        "<<UndefinedBehavior,Behavior is undefined>> if this mode is declared and an invocation does not write to *FragDepth*, or vice versa. "
        "Only valid with the *Fragment* <<Execution_Model, Execution Model>>.";

#define DEPTH_HINT(hint) "Indicates that per-fragment tests may assume that any" \
        " *FragDepth* <<BuiltIn, built in>>-decorated value written by the shader is " hint " the fragment's " \
        "interpolated depth value (given by the _z_ component of the *FragCoord* <<BuiltIn, built in>>-decorated variable). " \
        "Other stages of the pipeline use the written value as normal. " \
        "Only valid with the *Fragment* <<Execution_Model, execution model>>. "

    ExecutionModeParams[ExecutionModeDepthGreater].desc = DEPTH_HINT("greater-than-or-equal to");

    ExecutionModeParams[ExecutionModeDepthLess].desc = DEPTH_HINT("less-than-or-equal to");

    ExecutionModeParams[ExecutionModeDepthUnchanged].desc = DEPTH_HINT("the same as");

    ExecutionModeParams[ExecutionModeLocalSize].desc =
        "Indicates the workgroup size in the _x_, _y_, and _z_ dimensions. "
        "_x size_, _y size_, and _z size_ are unsigned 32-bit integers. "
        "Only valid with the *GLCompute* or *Kernel* <<Execution_Model,Execution Models>>.";
    ExecutionModeParams[ExecutionModeLocalSizeId].desc = SPECIFIED_AS_IDS("*LocalSize* <<Execution_Mode, Mode>>");

    ExecutionModeParams[ExecutionModeLocalSizeHint].desc =
        "A hint to the compiler, which indicates the most likely to be used workgroup size in the _x_, _y_, and _z_ "
        "dimensions. "
        "_x size_, _y size_, and _z size_ are unsigned 32-bit integers. "
        "Only valid with the *Kernel* <<Execution_Model,Execution Model>>.";
    ExecutionModeParams[ExecutionModeLocalSizeHintId].desc = SPECIFIED_AS_IDS("*LocalSizeHint* <<Execution_Mode, Mode>>");

    ExecutionModeParams[ExecutionModeInputPoints].desc = "Stage input primitive is _points_. "
                                                         "Only valid with the *Geometry* <<Execution_Model,Execution Model>>.";

    ExecutionModeParams[ExecutionModeInputLines].desc = "Stage input primitive is _lines_. "
                                                        "Only valid with the *Geometry* <<Execution_Model,Execution Model>>.";

    ExecutionModeParams[ExecutionModeInputLinesAdjacency].desc = "Stage input primitive is _lines adjacency_. "
                                                                 "Only valid with the *Geometry* <<Execution_Model,Execution Model>>.";

    ExecutionModeParams[ExecutionModeTriangles].desc = "For a geometry stage, input primitive is _triangles_. "
                                                       "For a tessellation stage, requests the tessellation primitive generator to generate triangles. "
                                                       "Only valid with the *Geometry* or one of the tessellation <<Execution_Model,Execution Models>>.";

    ExecutionModeParams[ExecutionModeInputTrianglesAdjacency].desc = "Geometry stage input primitive is _triangles adjacency_. "
                                                                     "Only valid with the *Geometry* <<Execution_Model,Execution Model>>.";

    ExecutionModeParams[ExecutionModeQuads].desc = "Requests the tessellation primitive generator to generate _quads_. "
                                                   "Only valid with one of the tessellation <<Execution_Model,Execution Models>>.";

    ExecutionModeParams[ExecutionModeIsolines].desc = "Requests the tessellation primitive generator to generate _isolines_. "
                                                      "Only valid with one of the tessellation <<Execution_Model,Execution Models>>.";

    ExecutionModeParams[ExecutionModeOutputVertices].desc =
        "_Vertex Count_ is an unsigned 32-bit integer. "
        "For a geometry stage, it is the maximum number of vertices the shader will ever emit in a single <<Invocation,invocation>>. "
        "For a tessellation-control stage, it is the number of vertices in the output patch produced by the tessellation "
        "control shader, which also specifies the number of times the tessellation control shader is invoked. "
        "Only valid with the *Geometry* or one of the tessellation <<Execution_Model,Execution Models>>.";

    ExecutionModeParams[ExecutionModeOutputPoints].desc = "Stage output primitive is _points_. "
                                                          "Only valid with the *Geometry* <<Execution_Model,Execution Model>>.";

    ExecutionModeParams[ExecutionModeOutputLineStrip].desc = "Stage output primitive is _line strip_. "
                                                             "Only valid with the *Geometry* <<Execution_Model,Execution Model>>.";

    ExecutionModeParams[ExecutionModeOutputTriangleStrip].desc = "Stage output primitive is _triangle strip_. "
                                                                 "Only valid with the *Geometry* <<Execution_Model,Execution Model>>.";

    ExecutionModeParams[ExecutionModeVecTypeHint].desc =
        "A hint to the compiler, which indicates that most operations used in the entry point are explicitly vectorized "
        "using a particular vector type. "
        "The 16 high-order bits of the _Vector Type_ operand specify the _number of components_ of the vector. "
        "The 16 low-order bits of the _Vector Type_ operand specify the _data type_ of the vector. "
        GAP "These are the legal _data type_ values: "
        LINE_BREAK "_0_ represents an 8-bit integer value. "
        LINE_BREAK "_1_ represents a 16-bit integer value. "
        LINE_BREAK "_2_ represents a 32-bit integer value. "
        LINE_BREAK "_3_ represents a 64-bit integer value. "
        LINE_BREAK "_4_ represents a 16-bit IEEE 754 float value. "
        LINE_BREAK "_5_ represents a 32-bit IEEE 754 float value. "
        LINE_BREAK "_6_ represents a 64-bit IEEE 754 float value. "
        GAP "Only valid with the *Kernel* <<Execution_Model,Execution Model>>.";

    ExecutionModeParams[ExecutionModeContractionOff].desc = "Indicates that floating-point-expressions contraction is disallowed. "
                                                            "Only valid with the *Kernel* <<Execution_Model,Execution Model>>.";

    ExecutionModeParams[ExecutionModeInitializer].desc = "Indicates that this entry point is a module initializer.";

    ExecutionModeParams[ExecutionModeFinalizer].desc = "Indicates that this entry point is a module finalizer.";

    ExecutionModeParams[ExecutionModeSubgroupSize].desc =
        "Indicates that this entry point requires the specified _Subgroup Size_. "
        "_Subgroup Size_ is an unsigned 32-bit integer.";

    ExecutionModeParams[ExecutionModeSubgroupsPerWorkgroup].desc =
        "Indicates that this entry point requires the specified number of _Subgroups Per Workgroup_. "
        "_Subgroups Per Workgroup_ is an unsigned 32-bit integer.";
    ExecutionModeParams[ExecutionModeSubgroupsPerWorkgroupId].desc = SPECIFIED_AS_ID(" *SubgroupsPerWorkgroup* <<Execution_Mode, mode>>");

    #define DENORM(BEHAVIOR) "Denormalized values input into or potentially generated by instructions should be " \
                             BEHAVIOR ". "

    #define ROUNDING(MODE1, MODE2) "The default rounding mode for floating-point arithmetic and conversions instructions "  \
                                   "is " MODE1 ". If an instruction is decorated with *FPRoundingMode* or defines a " \
                                   "rounding mode in its description, that rounding mode is applied and " MODE2 " is ignored. "

    #define FLOAT_CONTROL_OPERAND \
        "Only affects instructions operating on a floating-point type using the IEEE 754 encoding whose component width is _Target Width_. " \
        "_Target Width_ is an unsigned 32-bit integer. " \
        "May be applied at most once per _Target Width_ to any entry point. " \
        "Does not affect instructions decorated with *RelaxedPrecision*. "

    ExecutionModeParams[ExecutionModeDenormPreserve].desc = DENORM("preserved") SEE_CLIENT_API GAP FLOAT_CONTROL_OPERAND;
    ExecutionModeParams[ExecutionModeDenormFlushToZero].desc = DENORM("flushed to zero") SEE_CLIENT_API GAP FLOAT_CONTROL_OPERAND;

    ExecutionModeParams[ExecutionModeSignedZeroInfNanPreserve].desc = "The implementation does not perform optimizations on "
                             "floating-point instructions that do not preserve sign of a zero, or assume that operands and "
                             "results are not NaNs or infinities. "
                             "Bit patterns for NaNs might not be preserved. "
                         GAP FLOAT_CONTROL_OPERAND;

    ExecutionModeParams[ExecutionModeRoundingModeRTE].desc = ROUNDING("round to nearest even", "*RoundingModeRTE*")
                         GAP FLOAT_CONTROL_OPERAND;
    ExecutionModeParams[ExecutionModeRoundingModeRTZ].desc = ROUNDING("round toward zero", "*RoundingModeRTZ*")
                         GAP FLOAT_CONTROL_OPERAND;

    #define OBJECT_OR_MEMBER "Apply to an object or a member of a structure type. "
    #define MEMORY_OBJECT_OR_MEMBER "Must only be used on a <<MemoryObjectDeclaration, memory object declaration>> or a member of a structure type. "
    #define IMAGE_OR_UNIFORM                                                                                               \
        "Must be applied only to <<MemoryObjectDeclaration, memory object declarations>> or members of a structure type. " \
        "Any such memory object declaration, or any memory object declaration that contains such a structure type, "       \
        "must be one of: "                                                                                                 \
        GAP                                                                                                                \
        " - An image with _Sampled_ Operand of 2 and _Dim_ other than _SubpassData_ (see <<OpTypeImage, *OpTypeImage*>>).\n" \
        " - A block in the *StorageBuffer* <<Storage_Class, storage class>>, or in the "                                   \
        "   *Uniform* <<Storage_Class, storage class>> with the *BufferBlock* decoration.\n"                               \

    #define OR_PRIVATE_FUNCTION                                                                                            \
        " - <<Unified, Missing before>> *version 1.4*: An object in the *Private* or *Function* storage classes.\n"

    #define MEMBER "Applies only to a member of a structure type. "
    #define MATRIX "Only valid on a matrix or array whose most basic element is a matrix. "
    #define ALIAS_SECTION "See the <<AliasingSection,Aliasing>> section for more detail. "
    #define INPUT_OUTPUT "Only valid for the *Input* and *Output* <<Storage_Class,Storage Classes>>. "
    #define INPUT_OUTPUT_UNIFORMCONST "Only valid for the *Input*, *Output*, and *UniformConstant* <<Storage_Class,Storage Classes>>. "

    DecorationParams[DecorationRelaxedPrecision].desc = "Allow reduced precision operations. To be used as described in <<RelaxedPrecisionSection, Relaxed Precision>>.";

    #define BLOCK_DECORATION "Apply only to a structure type to establish it is a memory interface block. "

    DecorationParams[DecorationBlock].desc = BLOCK_DECORATION;

    DecorationParams[DecorationBufferBlock].desc =
        DEPRECATED("use *Block*-decorated *StorageBuffer* <<Storage_Class, Storage Class>> objects")
        LINE_BREAK
        BLOCK_DECORATION
        "When the type is used for a variable in the *Uniform* Storage Class the memory interface "
        "is a *StorageBuffer*-like interface, distinct from those variables decorated with *Block*. "
        "In all other Storage Classes the decoration is meaningless.";


    DecorationParams[DecorationRowMajor].desc = MEMBER MATRIX "Indicates that components within a row are contiguous in memory. "
        "Must not be used with *ColMajor* on the same matrix or matrix aggregate.";

    DecorationParams[DecorationColMajor].desc = MEMBER MATRIX "Indicates that components within a column are contiguous in memory. "
        "Must not be used with *RowMajor* on the same matrix or matrix aggregate.";

    DecorationParams[DecorationGLSLShared].desc = "Apply only to a structure type to get GLSL *shared* memory layout.";

    DecorationParams[DecorationGLSLPacked].desc = "Apply only to a structure type to get GLSL *packed* memory layout.";

    DecorationParams[DecorationNoPerspective].desc = MEMORY_OBJECT_OR_MEMBER
        "Requests linear, non-perspective correct, interpolation. "
        INPUT_OUTPUT;

    DecorationParams[DecorationFlat].desc = MEMORY_OBJECT_OR_MEMBER
        "Indicates no interpolation is done. "
        "The non-interpolated value comes from a vertex, as specified by the client API. "
        INPUT_OUTPUT;

    DecorationParams[DecorationPatch].desc = MEMORY_OBJECT_OR_MEMBER
        "Indicates a tessellation patch. "
        INPUT_OUTPUT
        "Invalid to use on objects or types referenced by non-tessellation <<Execution_Model,Execution Models>>.";

    DecorationParams[DecorationCentroid].desc = MEMORY_OBJECT_OR_MEMBER
        "If used with multi-sampling rasterization, allows a single interpolation location for an entire pixel. "
        "The interpolation location lies in both the pixel and in the primitive being rasterized. "
        INPUT_OUTPUT;

    DecorationParams[DecorationSample].desc = MEMORY_OBJECT_OR_MEMBER
        "If used with multi-sampling rasterization, requires per-sample interpolation. "
        "The interpolation locations are the locations of the samples lying in both the pixel and in the primitive being rasterized. "
        INPUT_OUTPUT;

    DecorationParams[DecorationInvariant].desc =
        "Apply only to a variable or member of a block-decorated structure type to indicate that expressions computing its "
        "value be computed invariantly with respect to other shaders computing the same expressions.";

    DecorationParams[DecorationRestrict].desc =
        "Apply only to a <<MemoryObjectDeclaration, memory object declaration>>, to indicate the compiler may compile as if there is no aliasing. "
        ALIAS_SECTION;

    DecorationParams[DecorationAliased].desc =
        "Apply only to a <<MemoryObjectDeclaration, memory object declaration>>, to indicate the compiler is to generate accesses to the variable that work correctly in the presence of aliasing. "
        ALIAS_SECTION;

    DecorationParams[DecorationVolatile].desc =
        IMAGE_OR_UNIFORM
        GAP
        "This indicates the memory holding the variable is volatile memory. "
        "Accesses to volatile memory cannot be eliminated, duplicated, or combined with other accesses. "
        "Volatile applies only to a single invocation and does not guarantee each invocation performs the access. "
        GAP
        "*Volatile* is not allowed if the declared <<Memory_Model, memory model>> is *Vulkan*. "
        "The <<Memory_Operands, memory operand>> bit *Volatile*, "
        "the <<Image_Operands, image operand>> bit *VolatileTexel*, or  "
        "the <<Memory_Semantics_-id-, memory semantic>> bit *Volatile* can be used instead.";

    DecorationParams[DecorationConstant].desc = "Indicates that a global variable is constant and *never* modified. Only allowed on global variables.";

    DecorationParams[DecorationCoherent].desc =
        IMAGE_OR_UNIFORM
        GAP
        "This indicates the memory backing the object is coherent. "
        GAP
        "*Coherent* is not allowed if the declared <<Memory_Model, memory model>> is *Vulkan*. "
        "The <<Memory_Operands, memory operand>> bits *MakePointerAvailable* and *MakePointerVisible* or "
        "the <<Image_Operands, image operand>> bits *MakeTexelAvailable* and *MakeTexelVisible* can be used instead.";

    DecorationParams[DecorationNonWritable].desc =
        IMAGE_OR_UNIFORM OR_PRIVATE_FUNCTION
        GAP
        "This indicates that this module does not write to the memory holding the variable. "
        "It does not prevent the use of initializers on a declaration.";

    DecorationParams[DecorationNonReadable].desc =
        IMAGE_OR_UNIFORM
        GAP
        "This indicates that this module does not read from the memory holding the variable. "
        "For image variables, it does not prevent query operations from reading metadata associated with the image.";

    DecorationParams[DecorationUniform].desc =
        "Apply only to an object. "
        "Asserts that, for each <<DynamicInstance,dynamic instance>> of the instruction that computes the result, "
        "all invocations in the same <<Tangle, tangle>> within the invocation's *Subgroup* scope compute the same result value.";

    DecorationParams[DecorationUniformId].desc =
        "Apply only to an object. "
        "Asserts that, for each <<DynamicInstance,dynamic instance>> of the instruction that computes the result, "
        "all invocations in the same <<Tangle, tangle>> within the invocation's _Execution_ scope compute the same result value. "
        "_Execution_ must not be *Invocation*.";

    DecorationParams[DecorationCPacked].desc =
        "Apply only to a structure type, to marks it as \"packed\", indicating that the alignment of the structure is one "
        "and that there is no padding between structure members.";

    DecorationParams[DecorationSaturatedConversion].desc =
        "Indicates that a conversion to an integer type which is outside the representable range of _Result Type_ "
        "is clamped to the nearest representable value of _Result Type_. "
        "_NaN_ is converted to _0_. "
        GAP
        "This decoration must be applied only to conversion instructions to integer types, not including the "
        "<<OpSatConvertUToS, *OpSatConvertUToS*>> and <<OpSatConvertSToU, *OpSatConvertSToU*>> instructions.";

    DecorationParams[DecorationStream].desc = MEMORY_OBJECT_OR_MEMBER
        "_Stream Number_ is an unsigned 32-bit integer indicating the stream number to put an output on. "
        "Only valid for the *Output* <<Storage_Class,Storage Class>> and the *Geometry* <<Execution_Model,Execution Model>>.";

    DecorationParams[DecorationLocation].desc =
        "Apply only to a variable or a structure-type member. "
        "_Location_ is an unsigned 32-bit integer that "
        "forms the main linkage for <<Storage_Class,Storage Class>> *Input* and *Output* variables:"
        GAP
        " - between the client API and vertex-stage inputs,\n"
        " - between consecutive programmable stages, or\n"
        " - between fragment-stage outputs and the client API.\n"
        GAP
        "It can also tag variables or structure-type members in the *UniformConstant* <<Storage_Class,Storage Class>> "
        "for linkage with the client API. "
        LINE_BREAK
        INPUT_OUTPUT_UNIFORMCONST;

    DecorationParams[DecorationComponent].desc = MEMORY_OBJECT_OR_MEMBER
        "_Component_ is an unsigned 32-bit integer indicating which component within a *Location* is taken by "
        "the decorated entity. "
        INPUT_OUTPUT;

    DecorationParams[DecorationIndex].desc =
        "Apply only to a variable. "
        "_Index_ is an unsigned 32-bit integer identifying a blend equation input index, "
        "used as specified by the client API. "
        "Only valid for the *Output* <<Storage_Class,Storage Class>> and the *Fragment* <<Execution_Model,Execution Model>>.";

    DecorationParams[DecorationBinding].desc =
        "Apply only to a variable. "
        "_Binding Point_ is an unsigned 32-bit integer "
        "forming part of the linkage between the client API and SPIR-V memory buffers, images, etc. "
        SEE_CLIENT_API;

    DecorationParams[DecorationDescriptorSet].desc =
        "Apply only to a variable. "
        "_Descriptor Set_ is an unsigned 32-bit integer "
        "forming part of the linkage between the client API and SPIR-V memory buffers, images, etc. "
        SEE_CLIENT_API;

    DecorationParams[DecorationOffset].desc =
        "Apply only to a structure-type member. "
        "_Byte Offset_ is an unsigned 32-bit integer. "
        "It dictates the byte offset of the member relative to the beginning of the structure. "
        "It can be used, for example, by both uniform and transform-feedback buffers. "
        "It must not cause any overlap of the structure's members, or overflow of a transform-feedback buffer's *XfbStride*.";

    DecorationParams[DecorationXfbBuffer].desc = MEMORY_OBJECT_OR_MEMBER
        "_XFB Buffer_ is an unsigned 32-bit integer indicating which transform-feedback buffer an output is written to. "
        "Only valid for the *Output* <<Storage_Class,Storage Classes>> of <<VertexProcessor,_vertex processing_>> <<Execution_Model,Execution Models>>.";

    DecorationParams[DecorationXfbStride].desc =
        "Apply to anything *XfbBuffer* is applied to. "
        "_XFB Stride_ is an unsigned 32-bit integer specifying the stride, in bytes, of transform-feedback buffer vertices. "
        "If the transform-feedback buffer is capturing any double-precision components, the stride must be a multiple of 8, "
        "otherwise it must be a multiple of 4.";

    DecorationParams[DecorationArrayStride].desc =
        "Apply to an array type to specify the stride, in bytes, of the array's elements. "
        "Can also apply to a pointer type to an array element. "
        "_Array Stride_ is an unsigned 32-bit integer specifying the stride of the "
        "array that the element resides in. "
        "Must not be applied to any other type.";

    DecorationParams[DecorationMatrixStride].desc = MEMBER MATRIX
        "_Matrix Stride_ is an unsigned 32-bit integer specifying the stride of "
        "the rows in a *RowMajor*-decorated matrix or columns in a *ColMajor*-decorated matrix.";

    DecorationParams[DecorationBuiltIn].desc = "Indicates which built-in variable an object represents. See <<BuiltIn, BuiltIn>> for more information.";

    DecorationParams[DecorationFuncParamAttr].desc =
        "Indicates a function return value or parameter attribute. "
        "Multiple uses of this decoration are allowed on the same _<id>_, "
        "as described in the <<Function_Parameter_Attribute, function parameter attributes>>.";

    DecorationParams[DecorationFPRoundingMode].desc = "Indicates a floating-point rounding mode.";

    DecorationParams[DecorationFPFastMathMode].desc = "Indicates a floating-point fast math flag.";

    DecorationParams[DecorationLinkageAttributes].desc =
        "Associate linkage attributes to values. "
        "_Name_ is a string specifying what name the _Linkage Type_ applies to. "
        "Only valid on <<OpFunction, *OpFunction*>> or global (module scope) <<OpVariable, *OpVariable*>>. See <<Linkage, linkage>>.";

    DecorationParams[DecorationSpecId].desc =
        "Apply only to a scalar specialization constant. "
        "_Specialization Constant ID_ is an unsigned 32-bit integer forming the external linkage for setting a specialized value. "
        "See <<SpecializationSection, specialization>>.";

    DecorationParams[DecorationNoContraction].desc =
        "Apply only to an <<Arithmetic, arithmetic instruction>> to indicate the operation cannot be combined with another instruction to form a single operation. "
        "For example, if applied to an <<OpFMul,*OpFMul*>>, that multiply can't be combined with an addition to yield a fused multiply-add operation. "
        "Furthermore, such operations are not allowed to reassociate; e.g., add(a + add(b+c)) cannot be transformed to add(add(a+b) + c).";

    DecorationParams[DecorationInputAttachmentIndex].desc =
        "Apply only to a variable. "
        "_Attachment Index_ is an unsigned 32-bit integer providing an input-target index (as specified by the client API). "
        "Only valid in the *Fragment* <<Execution_Model,Execution Model>> and for variables of type <<OpTypeImage,*OpTypeImage*>> "
        "with a <<Dim,_Dim_>> operand of *SubpassData*.";

    DecorationParams[DecorationAlignment].desc =
        "Apply only to a pointer. "
        "_Alignment_ is an unsigned 32-bit integer declaring a known minimum alignment the pointer has.";
    DecorationParams[DecorationAlignmentId].desc = SPECIFIED_AS_ID("*Alignment* <<Decoration, decoration>>");

    DecorationParams[DecorationMaxByteOffset].desc =
        "Apply only to a pointer. "
        "_Max Byte Offset_ is an unsigned 32-bit integer declaring a known maximum byte offset "
        "this pointer will be incremented by from the point of the decoration. "
        "This is a guaranteed upper bound when applied to <<OpFunctionParameter, *OpFunctionParameter*>>.";
    DecorationParams[DecorationMaxByteOffsetId].desc = SPECIFIED_AS_ID("*MaxByteOffset* <<Decoration, decoration>>");

    DecorationParams[DecorationNoSignedWrap].desc = "Apply to an instruction to indicate that it does not cause signed integer wrapping to occur, "
                                                    "in the form of overflow or underflow. "
                                                    GAP
                                                    "It must decorate only the following instructions:"
                                                    GAP
                                                    " - *OpIAdd*\n"
                                                    " - *OpISub*\n"
                                                    " - *OpIMul*\n"
                                                    " - *OpShiftLeftLogical*\n"
                                                    " - *OpSNegate*\n"
                                                    " - *OpExtInst* for instruction numbers specified in the extended instruction-set"
                                                    "    specifications as accepting this decoration.\n"
                                                    GAP
                                                    "If an instruction decorated with *NoSignedWrap* does overflow or underflow, "
                                                    "<<UndefinedBehavior,behavior is undefined>>.";

    DecorationParams[DecorationNoUnsignedWrap].desc = "Apply to an instruction to indicate that it does not cause unsigned integer wrapping to occur, "
                                                    "in the form of overflow or underflow. "
                                                    GAP
                                                    "It must decorate only the following instructions:"
                                                    GAP
                                                    " - *OpIAdd*\n"
                                                    " - *OpISub*\n"
                                                    " - *OpIMul*\n"
                                                    " - *OpShiftLeftLogical*\n"
                                                    " - *OpExtInst* for instruction numbers specified in the extended instruction-set"
                                                    "    specifications as accepting this decoration.\n"
                                                    GAP
                                                    "If an instruction decorated with *NoUnsignedWrap* does overflow or underflow, "
                                                    "<<UndefinedBehavior,behavior is undefined>>.";

    DecorationParams[DecorationCounterBuffer].desc = "The _<id>_ of a counter buffer associated with the decorated buffer. "
                                                     "It must decorate only a variable in the *Uniform* <<Storage_Class, storage class>>. "
                                                     "_Counter Buffer_ must be a variable in the *Uniform* storage class.";

    DecorationParams[DecorationUserSemantic].desc =
        "_Semantic_ is a string describing a user-defined semantic intent of what it decorates. "
        "User-defined semantics are case insensitive. "
        "It must decorate only a variable or a member of a structure type. "
        GAP
        "If decorating a variable, the variable must be in the *Input* or *Output* <<Storage_Class, storage classes>>. "
        "If decorating a structure member, memory object declarations that contain such structure type can be in any <<Storage_Class, storage class>>. "
        GAP
        "A variable or a structure member can be decorated more than one time with this decoration, but at most "
        "once for any particular string operand.";

    DecorationParams[DecorationRestrictPointer].desc =
        "Apply only to a <<MemoryObjectDeclaration,memory object declaration>>, "
        "to indicate the compiler may compile as if there is no aliasing of the pointer stored in the variable. "
        "See the <<AliasingSection, aliasing section>> for more detail.";
    DecorationParams[DecorationAliasedPointer].desc =
        "Apply only to a <<MemoryObjectDeclaration,memory object declaration>>, "
        "to indicate the compiler is to generate accesses to the pointer stored in the variable that work correctly "
        "in the presence of aliasing. "
        "See the <<AliasingSection, aliasing section>> for more detail.";
    DecorationParams[DecorationNonUniform].desc =
        "Apply only to an object. Asserts that the value backing the decorated "
        "_<id>_ is <<Uniformity, not dynamically uniform>>. "
        SEE_CLIENT_API;

    BuiltInParams[BuiltInPosition].desc                  = "Output vertex position from a <<VertexProcessor, vertex processing>> <<Execution_Model,Execution Model>>. "
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInPointSize].desc                 = "Output point size from a <<VertexProcessor, vertex processing>> <<Execution_Model,Execution Model>>. "
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInClipDistance].desc              = "Array of clip distances. "
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInCullDistance].desc              = "Array of clip distances. "
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInVertexId].desc                  = "Input vertex ID to a *Vertex* <<Execution_Model,Execution Model>>. "
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInInstanceId].desc                = "Input instance ID to a *Vertex* <<Execution_Model,Execution Model>>. "
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInPrimitiveId].desc               = "Primitive ID in a *Geometry* <<Execution_Model,Execution Model>>. "
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInInvocationId].desc              = "Invocation ID, input to *Geometry* and *TessellationControl* <<Execution_Model,Execution Model>>. "
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInLayer].desc                     = "Layer selection for multi-layer framebuffer. "
                                                           SEE_CLIENT_API
                                                           GAP
                                                           "The *Geometry* <<Capability, capability>> allows for a *Layer* output by a "
                                                           "*Geometry* <<Execution_Model, Execution Model>>, input to a "
                                                           "*Fragment* <<Execution_Model, Execution Model>>. "
                                                           GAP
                                                           "The *ShaderLayer* <<Capability, capability>> allows for *Layer* "
                                                           "output by a *Vertex* or *Tessellation* <<Execution_Model, Execution Model>>.";
    BuiltInParams[BuiltInViewportIndex].desc             = "Viewport selection for viewport transformation when using multiple viewports. "
                                                           SEE_CLIENT_API
                                                           GAP
                                                           "The *MultiViewport* <<Capability, capability>> allows for a *ViewportIndex* "
                                                           "output by a *Geometry* <<Execution_Model, Execution Model>>, input to a "
                                                           "*Fragment* <<Execution_Model, Execution Model>>. "
                                                           GAP
                                                           "The *ShaderViewportIndex* <<Capability, capability>> allows for a "
                                                           "*ViewportIndex* output by a *Vertex* or *Tessellation* "
                                                           "<<Execution_Model, Execution Model>>.";
    BuiltInParams[BuiltInTessLevelOuter].desc            = "Output patch outer levels in a *TessellationControl* <<Execution_Model,Execution Model>>. "
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInTessLevelInner].desc            = "Output patch inner levels in a *TessellationControl* <<Execution_Model,Execution Model>>. "
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInTessCoord].desc                 = "Input vertex position in *TessellationEvaluation* <<Execution_Model,Execution Model>>. "
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInPatchVertices].desc             = "Input patch vertex count in a tessellation <<Execution_Model,Execution Model>>. "
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInFragCoord].desc                 = "Coordinates _(x, y, z, 1/w)_ of the current fragment, input to the *Fragment* <<Execution_Model,Execution Model>>. "
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInPointCoord].desc                = "Coordinates within a _point_, input to the *Fragment* <<Execution_Model,Execution Model>>. "
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInFrontFacing].desc               = "Face direction, input to the *Fragment* <<Execution_Model,Execution Model>>. "
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInSampleId].desc                  = "Input sample number to the *Fragment* <<Execution_Model,Execution Model>>. "
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInSamplePosition].desc            = "Input sample position to the *Fragment* <<Execution_Model,Execution Model>>. "
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInSampleMask].desc                = "Input or output sample mask to the *Fragment* <<Execution_Model,Execution Model>>. "
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInFragDepth].desc                 = "Output fragment depth from the *Fragment* <<Execution_Model,Execution Model>>. "
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInHelperInvocation].desc          = "Input whether a helper invocation, to the *Fragment* <<Execution_Model,Execution Model>>. "
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInNumWorkgroups].desc             = "Number of workgroups in *GLCompute* or *Kernel*  <<Execution_Model,Execution Models>>. "
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInWorkgroupSize].desc             = "Workgroup size in *GLCompute* or *Kernel*  <<Execution_Model,Execution Models>>. "
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInWorkgroupId].desc               = "Workgroup ID in *GLCompute* or *Kernel*  <<Execution_Model,Execution Models>>. "
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInLocalInvocationId].desc         = "Local invocation ID in *GLCompute* or *Kernel*  <<Execution_Model,Execution Models>>. "
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInGlobalInvocationId].desc        = "Global invocation ID in *GLCompute* or *Kernel*  <<Execution_Model,Execution Models>>. "
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInLocalInvocationIndex].desc      = "Local invocation index in *GLCompute* <<Execution_Model,Execution Models>>. "
                                                           SEE_CLIENT_API
                                                           GAP
                                                           "Workgroup Linear ID in *Kernel* <<Execution_Model,Execution Models>>. "
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInWorkDim].desc                   = "Work dimensions in *Kernel* <<Execution_Model,Execution Models>>. "
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInGlobalSize].desc                = "Global size in *Kernel* <<Execution_Model,Execution Models>>. "
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInEnqueuedWorkgroupSize].desc     = "Enqueued workgroup size in *Kernel* <<Execution_Model,Execution Models>>. "
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInGlobalOffset].desc              = "Global offset in *Kernel* <<Execution_Model,Execution Models>>. "
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInGlobalLinearId].desc            = "Global linear ID in *Kernel* <<Execution_Model,Execution Models>>. "
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInSubgroupSize].desc              = "Subgroup size. "
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInSubgroupMaxSize].desc           = "Subgroup maximum size in *Kernel* <<Execution_Model,Execution Models>>. "
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInNumSubgroups].desc              = "Number of subgroups in *GLCompute* or *Kernel* <<Execution_Model,Execution Models>>. "
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInNumEnqueuedSubgroups].desc      = "Number of enqueued subgroups in *Kernel* <<Execution_Model,Execution Models>>. "
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInSubgroupId].desc                = "Subgroup ID in *GLCompute* or *Kernel* <<Execution_Model,Execution Models>>. "
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInSubgroupLocalInvocationId].desc = "Subgroup local invocation ID. "
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInVertexIndex].desc               = "Vertex index. "
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInInstanceIndex].desc             = "Instance index. "
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInSubgroupEqMask].desc            = "Subgroup invocations bitmask where bit index = *SubgroupLocalInvocationId*. "
                                                           LINE_BREAK
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInSubgroupGeMask].desc            = "Subgroup invocations bitmask where bit index {ge} *SubgroupLocalInvocationId*. "
                                                           LINE_BREAK
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInSubgroupGtMask].desc            = "Subgroup invocations bitmask where bit index > *SubgroupLocalInvocationId*. "
                                                           LINE_BREAK
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInSubgroupLeMask].desc            = "Subgroup invocations bitmask where bit index {le} *SubgroupLocalInvocationId*. "
                                                           LINE_BREAK
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInSubgroupLtMask].desc            = "Subgroup invocations bitmask where bit index < *SubgroupLocalInvocationId*. "
                                                           LINE_BREAK
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInViewIndex].desc                 = "Input view index of the view currently being rendered to."
                                                           LINE_BREAK
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInBaseVertex].desc                = "Base vertex component of vertex ID."
                                                           LINE_BREAK
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInBaseInstance].desc              = "Base instance component of instance ID."
                                                           LINE_BREAK
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInDrawIndex].desc                 = "Contains the index of the draw currently being processed."
                                                           LINE_BREAK
                                                           SEE_CLIENT_API;
    BuiltInParams[BuiltInDeviceIndex].desc               = "Input device index of the logical device."
                                                           LINE_BREAK
                                                           SEE_CLIENT_API;

#define LITERAL_UNSIGNED_OPERAND(what) what " is specified in a subsequent unsigned 32-bit integer literal operand. "
#define LITERAL_UNSIGNED(what)         what " is an unsigned 32-bit integer. "

    SelectionControlParams[SelectionControlFlattenShift].desc = "Performance hint. Strong request to optimize away the control flow for this selection.";
    SelectionControlParams[SelectionControlDontFlattenShift].desc = "Performance hint. Strong request to keep this selection as control flow.";

    LoopControlParams[LoopControlUnrollShift].desc             = "Performance hint. Strong request to unroll or unwind this loop."
                                                                 LINE_BREAK
                                                                 "This must not be used with the *DontUnroll* bit.";
    LoopControlParams[LoopControlDontUnrollShift].desc         = "Performance hint. Strong request to keep this loop as a loop, without unrolling."
                                                                 LINE_BREAK
                                                                 "This must not be used with the *Unroll*, *PeelCount*, or *PartialCount* bits.";
    LoopControlParams[LoopControlDependencyInfiniteShift].desc = "Guarantees that there are no dependencies between loop iterations.";
    LoopControlParams[LoopControlDependencyLengthShift].desc   =
        "Guarantees that there are no dependencies between a number of loop iterations. "
        LITERAL_UNSIGNED_OPERAND("The dependency length");
    LoopControlParams[LoopControlMinIterationsShift].desc      =
        "Unchecked assertion that the loop executes at least a given number of iterations. "
        LITERAL_UNSIGNED_OPERAND("The iteration count");
    LoopControlParams[LoopControlMaxIterationsShift].desc      =
        "Unchecked assertion that the loop executes at most a given number of iterations. "
        LITERAL_UNSIGNED_OPERAND("The iteration count");
    LoopControlParams[LoopControlIterationMultipleShift].desc  =
        "Unchecked assertion that the loop executes a multiple of a given number of iterations. "
        LITERAL_UNSIGNED_OPERAND("The number")
        "It must be greater than 0.";
    LoopControlParams[LoopControlPeelCountShift].desc          =
        "Performance hint. Request that the loop be peeled by a given number of loop iterations. "
        LITERAL_UNSIGNED_OPERAND("The peel count")
        LINE_BREAK
        "This must not be used with the *DontUnroll* bit.";
    LoopControlParams[LoopControlPartialCountShift].desc       =
        "Performance hint. Request that the loop be partially unrolled by a given number of loop iterations. "
        LITERAL_UNSIGNED_OPERAND("The unroll count")
        LINE_BREAK
        "This must not be used with the *DontUnroll* bit.";

    FunctionControlParams[FunctionControlInlineShift].desc = "Performance hint. Strong request to inline the function.";
    FunctionControlParams[FunctionControlDontInlineShift].desc = "Performance hint. Strong request to not inline the function.";
    FunctionControlParams[FunctionControlPureShift].desc =
        "Compiler can assume this function has no side effect, but might read global memory or read through dereferenced function parameters. "
        "Always computes the same result when called with the same argument values and the same global state.";
    FunctionControlParams[FunctionControlConstShift].desc =
        "Compiler assumes this function has no side effects, and does not access global memory or dereference function parameters. "
        "Always computes the same result for the same argument values.";

    MemorySemanticsParams[MemorySemanticsSequentiallyConsistentShift].desc =
        "All observers see this memory access in the same order with respect to other sequentially-consistent "
        "memory accesses from this <<Invocation,invocation>>."
        LINE_BREAK
        "If the declared <<Memory_Model, memory model>> is *Vulkan*, *SequentiallyConsistent* must not be used.";
    MemorySemanticsParams[MemorySemanticsAcquireShift].desc =
        "On an atomic instruction, orders memory operations provided in program order "
        "after this atomic instruction against this atomic instruction. "
        "On a barrier, orders memory operations provided in program order after this barrier against atomic "
        "instructions before this barrier. "
        SEE_CLIENT_API;
    MemorySemanticsParams[MemorySemanticsReleaseShift].desc =
        "On an atomic instruction, orders memory operations provided in program order "
        "before this atomic instruction against this atomic instruction. "
        "On a barrier, orders memory operations provided in program order before this barrier against atomic "
        "instructions after this barrier. "
        SEE_CLIENT_API;
    MemorySemanticsParams[MemorySemanticsAcquireReleaseShift].desc = "Has the properties of both <<Memory_Semantics_-id-,*Acquire*>> and <<Memory_Semantics_-id-,*Release*>> semantics. It is used for read-modify-write operations.";

    MemorySemanticsParams[MemorySemanticsUniformMemoryShift].desc = "Apply the memory-ordering constraints to "
        "*StorageBuffer*, *PhysicalStorageBuffer*, or *Uniform* <<Storage_Class,Storage Class>> memory.";

    MemorySemanticsParams[MemorySemanticsSubgroupMemoryShift].desc = "Apply the memory-ordering constraints to subgroup memory.";
    MemorySemanticsParams[MemorySemanticsWorkgroupMemoryShift].desc = "Apply the memory-ordering constraints to *Workgroup* <<Storage_Class,Storage Class>> memory.";
    MemorySemanticsParams[MemorySemanticsCrossWorkgroupMemoryShift].desc = "Apply the memory-ordering constraints to *CrossWorkgroup* <<Storage_Class,Storage Class>> memory.";

    MemorySemanticsParams[MemorySemanticsAtomicCounterMemoryShift].desc = "Apply the memory-ordering constraints to *AtomicCounter* <<Storage_Class,Storage Class>> memory.";

    MemorySemanticsParams[MemorySemanticsImageMemoryShift].desc = "Apply the memory-ordering constraints to image contents (types declared by <<OpTypeImage,*OpTypeImage*>>), "
                                                                  "or to accesses done through pointers to the *Image* <<Storage_Class,Storage Class>>.";

    MemorySemanticsParams[MemorySemanticsOutputMemoryShift].desc =
        "Apply the memory-ordering constraints to *Output* <<Storage_Class, storage class>> memory.";

    MemorySemanticsParams[MemorySemanticsMakeAvailableShift].desc =
        "Perform an availability operation on all references in the selected <<Storage_Class, storage classes>>.";

    MemorySemanticsParams[MemorySemanticsMakeVisibleShift].desc =
        "Perform a visibility operation on all references in the selected <<Storage_Class, storage classes>>.";

    MemorySemanticsParams[MemorySemanticsVolatileShift].desc =
        "This access cannot be eliminated, duplicated, or combined with other accesses.";

    MemoryAccessParams[MemoryAccessVolatileShift].desc = "This access cannot be eliminated, duplicated, or combined with other accesses.";
    MemoryAccessParams[MemoryAccessAlignedShift].desc =
        "This access has a known alignment. "
        LITERAL_UNSIGNED_OPERAND("The alignment")
        "The value must be a power of two. "
        "Valid values are defined by the execution environment.";
    MemoryAccessParams[MemoryAccessNontemporalShift].desc = "Hints that the accessed address is not likely to be accessed again in the near future.";
    MemoryAccessParams[MemoryAccessMakePointerAvailableShift].desc =
        "Perform an availability operation on the locations pointed to by the pointer operand, after a store. "
        "A following operand is the memory <<Scope_-id-, scope>> for the availability operation. "
        "Requires *NonPrivatePointer* to also be set. "
        "Only valid with instructions writing memory.";
    MemoryAccessParams[MemoryAccessMakePointerVisibleShift].desc =
        "Perform a visibility operation on the locations pointed to by the pointer operand, before a load. "
        "A following operand is the memory <<Scope_-id-, scope>> for the visibility operation. "
        "Requires *NonPrivatePointer* to also be set. "
        "Only valid with instructions reading memory.";
    MemoryAccessParams[MemoryAccessNonPrivatePointerShift].desc =
        "The memory access obeys inter-thread ordering, as specified by the client API.";

    ScopeParams[ScopeCrossDevice].desc = "Scope crosses multiple devices.";
    ScopeParams[ScopeDevice].desc =      "Scope is the current device.";
    ScopeParams[ScopeWorkgroup].desc =   "Scope is the current <<Workgroup,workgroup>>.";
    ScopeParams[ScopeSubgroup].desc =    "Scope is the current <<Subgroup,subgroup>>.";
    ScopeParams[ScopeInvocation].desc =  "Scope is the current <<Invocation,Invocation>>.";
    ScopeParams[ScopeQueueFamily].desc = "Scope is the current queue family.";

    // Group Operations
    GroupOperationParams[GroupOperationReduce].desc = "A reduction operation for all values of a specific value X specified by <<Invocation,invocations>> within a workgroup.";
    GroupOperationParams[GroupOperationInclusiveScan].desc = "A binary operation "
                                    "with an identity _I_ and _n_ (where _n_ is the size of the workgroup) elements[_a~0~_, _a~1~_, ... _a~n-1~_] "
                                    "resulting in [_a~0~_, (_a~0~_ op _a~1~_), ...(_a~0~_ op _a~1~_ op ... op _a~n-1~_)]";
    GroupOperationParams[GroupOperationExclusiveScan].desc = "A binary operation "
        "with an identity _I_ and _n_ (where _n_ is the size of the workgroup) elements[_a~0~_, _a~1~_, ... _a~n-1~_] "
        "resulting in [_I_, _a~0~_, (_a~0~_ op _a~1~_), ... (_a~0~_ op _a~1~_ op ... op _a~n-2~_)].";

    // Enqueue flags
    KernelEnqueueFlagsParams[KernelEnqueueFlagsNoWait].desc = "Indicates that the enqueued kernels do not need to wait for the "
        "parent kernel to finish execution before they begin execution.";
    KernelEnqueueFlagsParams[KernelEnqueueFlagsWaitKernel].desc = "Indicates that all invocations of the parent kernel finish executing "
        "and all immediate side effects are committed before the enqueued child kernel begins execution. "
        NOTE "Immediate meaning not side effects resulting from child kernels. "
        "The side effects would include stores to global memory and pipe reads and writes.";
    KernelEnqueueFlagsParams[KernelEnqueueFlagsWaitWorkGroup].desc = "Indicates that the enqueued kernels wait only for the workgroup "
        "that enqueued the kernels to finish before they begin execution. "
        NOTE "This acts as a memory synchronization point between invocations in a workgroup "
        "and child kernels enqueued by invocations in the workgroup.";

    // Profiling info
    KernelProfilingInfoParams[KernelProfilingInfoCmdExecTimeShift].desc = "Indicates that the profiling info queried is the execution time.";

    // Capability
    CapabilityParams[CapabilityMatrix].desc = "Uses <<OpTypeMatrix,*OpTypeMatrix*>>.";
    CapabilityParams[CapabilityShader].desc = "Uses *Vertex*, *Fragment*, or *GLCompute* <<Execution_Model, Execution Models>>.";
    CapabilityParams[CapabilityGeometry].desc = "Uses the *Geometry* <<Execution_Model, Execution Model>>.";
    CapabilityParams[CapabilityTessellation].desc = "Uses the *TessellationControl* or *TessellationEvaluation* <<Execution_Model, Execution Models>>.";
    CapabilityParams[CapabilityAddresses].desc = "Uses physical addressing, non-logical addressing modes.";
    CapabilityParams[CapabilityLinkage].desc = "Uses partially linked modules and libraries.";
    CapabilityParams[CapabilityKernel].desc = "Uses the *Kernel* <<Execution_Model, Execution Model>>.";
    CapabilityParams[CapabilityVector16].desc = "Uses <<OpTypeVector,*OpTypeVector*>> to declare 8 component or 16 component vectors.";
    CapabilityParams[CapabilityFloat16Buffer].desc = "Allows a 16-bit <<OpTypeFloat, *OpTypeFloat*>> instruction using the IEEE 754 encoding for creating an "
        "<<OpTypePointer, *OpTypePointer*>> to a 16-bit float. "
        "Pointers to a 16-bit float must not be dereferenced, unless specifically allowed by a specific instruction. "
        "All other uses of 16-bit *OpTypeFloat* are disallowed.";
    CapabilityParams[CapabilityFloat16].desc = "Uses <<OpTypeFloat,*OpTypeFloat*>> to declare the 16-bit floating-point type using the IEEE 754 encoding.";
    CapabilityParams[CapabilityFloat64].desc = "Uses <<OpTypeFloat,*OpTypeFloat*>> to declare the 64-bit floating-point type using the IEEE 754 encoding.";
    CapabilityParams[CapabilityInt16].desc = "Uses <<OpTypeInt,*OpTypeInt*>> to declare 16-bit integer types.";
    CapabilityParams[CapabilityInt64].desc = "Uses <<OpTypeInt,*OpTypeInt*>> to declare 64-bit integer types.";
    CapabilityParams[CapabilityInt64Atomics].desc = "Uses atomic instructions on 64-bit integer types.";
    CapabilityParams[CapabilityImageBasic].desc = "Uses <<OpTypeImage,*OpTypeImage*>> or <<OpTypeSampler,*OpTypeSampler*>> in a *Kernel*.";
    CapabilityParams[CapabilityImageReadWrite].desc = "Uses <<OpTypeImage,*OpTypeImage*>> with the *ReadWrite* <<Access_Qualifier, _access qualifier_>> in a kernel.";
    CapabilityParams[CapabilityImageMipmap].desc = "Uses non-zero *Lod* <<Image_Operands, Image Operands>> in a kernel.";
    CapabilityParams[CapabilityPipes].desc = "Uses <<OpTypePipe,*OpTypePipe*>>, <<OpTypeReserveId,*OpTypeReserveId*>> or <<Pipe, _pipe_>> instructions.";
    CapabilityParams[CapabilityGroups].desc = "Uses common group instructions.";
    CapabilityParams[CapabilityDeviceEnqueue].desc = "Uses <<OpTypeQueue,*OpTypeQueue*>>, <<OpTypeDeviceEvent,*OpTypeDeviceEvent*>>, and "
                                                     "<<Device-Side_Enqueue, _device side enqueue_>> instructions.";
    CapabilityParams[CapabilityLiteralSampler].desc = "<<Sampler,Samplers>> are made from literals within the module. See <<OpConstantSampler,*OpConstantSampler*>>.";
    CapabilityParams[CapabilityAtomicStorage].desc = "Uses the *AtomicCounter* <<Storage_Class, Storage Class>>, "
                                                     "allowing use of only the <<OpAtomicLoad,*OpAtomicLoad*>>, <<OpAtomicIIncrement,*OpAtomicIIncrement*>>, "
                                                     "and <<OpAtomicIDecrement,*OpAtomicIDecrement*>> instructions.";
    CapabilityParams[CapabilityTessellationPointSize].desc = "Tessellation stage exports point size.";
    CapabilityParams[CapabilityGeometryPointSize].desc = "Geometry stage exports point size";
    CapabilityParams[CapabilityImageGatherExtended].desc = "Uses texture gather with non-constant or independent offsets";
    CapabilityParams[CapabilityStorageImageExtendedFormats].desc = "Uses additional set of formats for storage images.";
    CapabilityParams[CapabilityStorageImageMultisample].desc = "An _MS_ operand in <<OpTypeImage, OpTypeImage>> indicates multisampled, "
                                                               "used with an <<OpTypeImage, *OpTypeImage*>> having _Sampled_ == 2.";
    CapabilityParams[CapabilityUniformBufferArrayDynamicIndexing].desc = "*Block*-decorated arrays in uniform storage classes use <<DynamicallyUniform,dynamically uniform>> indexing.";
    CapabilityParams[CapabilitySampledImageArrayDynamicIndexing].desc = "Arrays of sampled images, samplers, or images with _Sampled_ = 0 or 1 use <<DynamicallyUniform,dynamically uniform>> indexing.";
    CapabilityParams[CapabilityStorageBufferArrayDynamicIndexing].desc = "Arrays in the *StorageBuffer* <<Storage_Class, Storage Class>>, or "
                                                                         "*BufferBlock*-decorated arrays, use <<DynamicallyUniform, dynamically uniform>> indexing.";
    CapabilityParams[CapabilityStorageImageArrayDynamicIndexing].desc = "Arrays of images with _Sampled_ = 2 are accessed with <<DynamicallyUniform,dynamically uniform>> indexing.";
    CapabilityParams[CapabilityClipDistance].desc = "Uses the *ClipDistance* <<BuiltIn, BuiltIn>>.";
    CapabilityParams[CapabilityCullDistance].desc = "Uses the *CullDistance* <<BuiltIn, BuiltIn>>.";
    CapabilityParams[CapabilityImageCubeArray].desc = "Uses the *Cube* <<Dim,Dim>> with the _Arrayed_ operand in <<OpTypeImage, *OpTypeImage*>>, "
                                                      "with an <<OpTypeImage, *OpTypeImage*>> having _Sampled_ == 2.";
    CapabilityParams[CapabilitySampleRateShading].desc = "Uses per-sample rate shading.";
    CapabilityParams[CapabilityGenericPointer].desc = "Uses the *Generic* <<Storage_Class, Storage Class>>.";
    CapabilityParams[CapabilityInt8].desc = "Uses <<OpTypeInt,*OpTypeInt*>> to declare 8-bit integer types.";
    CapabilityParams[CapabilityImageRect].desc = "Uses the *Rect* <<Dim,Dim>> with an <<OpTypeImage, *OpTypeImage*>> having _Sampled_ == 2.";
    CapabilityParams[CapabilitySampledRect].desc = "Uses the *Rect* <<Dim,Dim>> with an <<OpTypeImage, *OpTypeImage*>> having _Sampled_ == 0 or 1.";
    CapabilityParams[CapabilityInputAttachment].desc = "Uses the *SubpassData* <<Dim,Dim>>.";
    CapabilityParams[CapabilitySparseResidency].desc = "Uses *OpImageSparse...* instructions.";
    CapabilityParams[CapabilityMinLod].desc = "Uses the *MinLod* <<Image_Operands, Image Operand>>.";
    CapabilityParams[CapabilitySampled1D].desc = "Uses the *1D* <<Dim,Dim>> with an <<OpTypeImage, *OpTypeImage*>> having _Sampled_ == 0 or 1.";
    CapabilityParams[CapabilityImage1D].desc = "Uses the *1D* <<Dim,Dim>> with an <<OpTypeImage, *OpTypeImage*>> having _Sampled_ == 2.";
    CapabilityParams[CapabilitySampledCubeArray].desc = "Uses the *Cube* <<Dim,Dim>> with the _Arrayed_ operand in <<OpTypeImage, OpTypeImage>>, "
                                                        "with an <<OpTypeImage, *OpTypeImage*>> having _Sampled_ == 0 or 1.";
    CapabilityParams[CapabilitySampledBuffer].desc = "Uses the *Buffer* <<Dim,Dim>> with an <<OpTypeImage, *OpTypeImage*>> having _Sampled_ == 0 or 1.";
    CapabilityParams[CapabilityImageBuffer].desc = "Uses the *Buffer* <<Dim,Dim>> with an <<OpTypeImage, *OpTypeImage*>> having _Sampled_ == 2.";
    CapabilityParams[CapabilityImageMSArray].desc = "An _MS_ operand in <<OpTypeImage, OpTypeImage>> indicates multisampled, "
                                                    "used with an <<OpTypeImage, *OpTypeImage*>> having _Sampled_ == 2 and _Arrayed_ == 1.";
    CapabilityParams[CapabilityStorageImageExtendedFormats].desc = "One of a large set of more advanced image formats are used, namely one of those in the <<Image_Format, Image Format>> table listed as requiring this capability.";
    CapabilityParams[CapabilityImageQuery].desc = "The sizes, number of samples, or lod, etc. are queried.";
    CapabilityParams[CapabilityDerivativeControl].desc = "Uses fine or coarse-grained derivatives, e.g., <<OpDPdxFine,*OpDPdxFine*>>.";
    CapabilityParams[CapabilityInterpolationFunction].desc = "Uses one of the *InterpolateAtCentroid*, *InterpolateAtSample*, or *InterpolateAtOffset* GLSL.std.450 extended instructions.";
    CapabilityParams[CapabilityTransformFeedback].desc = "Uses the *Xfb* <<Execution_Mode, Execution Mode>>.";
    CapabilityParams[CapabilityGeometryStreams].desc = "Uses multiple numbered streams for geometry-stage output.";
    CapabilityParams[CapabilityStorageImageReadWithoutFormat].desc = "<<OpImageRead,*OpImageRead*>> can use the *Unknown* <<Image_Format, Image Format>>.";
    CapabilityParams[CapabilityStorageImageWriteWithoutFormat].desc = "<<OpImageWrite,*OpImageWrite*>> can use the *Unknown* <<Image_Format, Image Format>>.";
    CapabilityParams[CapabilityMultiViewport].desc = "Multiple viewports are used.";
    CapabilityParams[CapabilitySubgroupDispatch].desc = "Uses subgroup dispatch instructions.";
    CapabilityParams[CapabilityNamedBarrier].desc = "Uses <<OpTypeNamedBarrier, *OpTypeNamedBarrier*>>.";
    CapabilityParams[CapabilityPipeStorage].desc = "Uses <<OpTypePipeStorage, *OpTypePipeStorage*>>.";

    CapabilityParams[CapabilityStorageBuffer8BitAccess].desc =
        "Uses 8-bit <<OpTypeInt, *OpTypeInt*>> instructions for creating "
        "scalar, vector, and composite types that become members of a block residing in the "
        "*StorageBuffer* <<Storage_Class, storage class>> or the *PhysicalStorageBuffer* storage class.";
    CapabilityParams[CapabilityUniformAndStorageBuffer8BitAccess].desc =
        "Uses 8-bit <<OpTypeInt, *OpTypeInt*>> instructions for creating "
        "scalar, vector, and composite types that become members of a block residing in the "
        "*StorageBuffer* <<Storage_Class, storage class>>, the *PhysicalStorageBuffer* storage class, or the "
        "*Uniform* storage class.";
    CapabilityParams[CapabilityStoragePushConstant8].desc =
        "Uses 8-bit <<OpTypeInt, *OpTypeInt*>> instructions for creating "
        "scalar, vector, and composite types that become members of a block residing in the "
        "*PushConstant* <<Storage_Class, storage class>>.";
    CapabilityParams[CapabilityStorageUniformBufferBlock16].desc =
        "Uses 16-bit <<OpTypeFloat, *OpTypeFloat*>> and <<OpTypeInt, *OpTypeInt*>> instructions for creating "
        "scalar, vector, and composite types that become members of a block residing in the "
        "*StorageBuffer* <<Storage_Class, storage class>>, the *PhysicalStorageBuffer* storage class, or the "
        "*Uniform* storage class with the *BufferBlock* <<Decoration, decoration>>.";
    CapabilityParams[CapabilityStorageUniform16].desc =
        "Uses 16-bit <<OpTypeFloat, *OpTypeFloat*>> and <<OpTypeInt, *OpTypeInt*>> instructions for creating "
        "scalar, vector, and composite types that become members of a block residing in the "
        "*StorageBuffer* <<Storage_Class, storage class>>, the *PhysicalStorageBuffer* storage class, or the "
        "*Uniform* storage class.";
    CapabilityParams[CapabilityStoragePushConstant16].desc =
        "Uses 16-bit <<OpTypeFloat, *OpTypeFloat*>> and <<OpTypeInt, *OpTypeInt*>> instructions for creating "
        "scalar, vector, and composite types that become members of a block residing in the "
        "*PushConstant* <<Storage_Class, storage class>>.";
    CapabilityParams[CapabilityStorageInputOutput16].desc =
        "Uses 16-bit <<OpTypeFloat, *OpTypeFloat*>> and <<OpTypeInt, *OpTypeInt*>> instructions for creating "
        "scalar, vector, and composite types that become members of a block residing in the "
        "*Output* <<Storage_Class, storage class>>.";

    CapabilityParams[CapabilityVariablePointersStorageBuffer].desc =
        "Allow <<VariablePointer, _variable pointers_>>, each confined to a single "
        "*Block*-decorated struct in the *StorageBuffer* storage class.";
    CapabilityParams[CapabilityVariablePointers].desc = "Allow <<VariablePointer, _variable pointers_>>.";

    CapabilityParams[CapabilityDenormPreserve].desc = "Uses the *DenormPreserve* <<Execution_Mode, execution mode>>.";
    CapabilityParams[CapabilityDenormFlushToZero].desc = "Uses the *DenormFlushToZero* <<Execution_Mode, execution mode>>.";
    CapabilityParams[CapabilitySignedZeroInfNanPreserve].desc = "Uses the *SignedZeroInfNanPreserve* <<Execution_Mode, execution mode>>.";
    CapabilityParams[CapabilityRoundingModeRTE].desc = "Uses the *RoundingModeRTE* <<Execution_Mode, execution mode>>.";
    CapabilityParams[CapabilityRoundingModeRTZ].desc = "Uses the *RoundingModeRTZ* <<Execution_Mode, execution mode>>.";
    CapabilityParams[CapabilityVulkanMemoryModel].desc =
        "Uses the *Vulkan* <<Memory_Model, memory model>>. "
        "This capability must be declared if and only if the *Vulkan* memory model is declared.";
    CapabilityParams[CapabilityVulkanMemoryModelDeviceScope].desc =
        "Uses *Device* <<Scope_-id-, scope>> with any instruction when the *Vulkan* <<Memory_Model, memory model>> is declared.";
    CapabilityParams[CapabilityPhysicalStorageBufferAddresses].desc =
        "Uses physical addressing on storage buffers.";

    CapabilityParams[CapabilityShaderNonUniform].desc =
        "Uses the *NonUniform* <<Decoration, decoration>> on a variable or instruction.";
    CapabilityParams[CapabilityRuntimeDescriptorArray].desc =
        "Uses arrays of resources which are sized at run-time.";
    CapabilityParams[CapabilityInputAttachmentArrayDynamicIndexing].desc =
        "Arrays of **InputAttachment**s use <<DynamicallyUniform, dynamically uniform>> indexing.";
    CapabilityParams[CapabilityUniformTexelBufferArrayDynamicIndexing].desc =
        "Arrays of **SampledBuffer**s use <<DynamicallyUniform, dynamically uniform>> indexing.";
    CapabilityParams[CapabilityStorageTexelBufferArrayDynamicIndexing].desc =
        "Arrays of **ImageBuffer**s use <<DynamicallyUniform, dynamically uniform>> indexing.";
    CapabilityParams[CapabilityUniformBufferArrayNonUniformIndexing].desc =
        "**Block**-<<Decoration, decorated>> arrays in uniform storage classes use <<Uniformity, non-uniform>> indexing.";
    CapabilityParams[CapabilitySampledImageArrayNonUniformIndexing].desc =
        "Arrays of sampled images use <<Uniformity, non-uniform>> indexing.";
    CapabilityParams[CapabilityStorageBufferArrayNonUniformIndexing].desc =
        "Arrays in the *StorageBuffer* <<Storage_Class, storage class>> or "
        "**BufferBlock**-<<Decoration, decorated>> arrays use <<Uniformity, non-uniform>> indexing.";
    CapabilityParams[CapabilityStorageImageArrayNonUniformIndexing].desc =
        "Arrays of non-sampled images use <<Uniformity, non-uniform>> indexing.";
    CapabilityParams[CapabilityInputAttachmentArrayNonUniformIndexing].desc =
        "Arrays of **InputAttachment**s use <<Uniformity, non-uniform>> indexing.";
    CapabilityParams[CapabilityUniformTexelBufferArrayNonUniformIndexing].desc =
        "Arrays of **SampledBuffer**s use <<Uniformity, non-uniform>> indexing.";
    CapabilityParams[CapabilityStorageTexelBufferArrayNonUniformIndexing].desc =
        "Arrays of **ImageBuffer**s use <<Uniformity, non-uniform>> indexing.";
    CapabilityParams[CapabilityDotProduct].desc =
        "Uses dot product instructions";
    CapabilityParams[CapabilityDotProductInputAll].desc =
        "Uses vector of any integer type as input to the dot product instructions";
    CapabilityParams[CapabilityDotProductInput4x8Bit].desc =
        "Uses vectors of four components of 8-bit integer type as inputs to the dot product instructions";
    CapabilityParams[CapabilityDotProductInput4x8BitPacked].desc =
        "Uses 32-bit integer scalars packing 4-component vectors of 8-bit integers as inputs to the dot product instructions";
    CapabilityParams[CapabilityUniformDecoration].desc =
        "Uses the *Uniform* or *UniformId* <<Decoration, decoration>>";

    // Packed Vector Format
    PackedVectorFormatParams[PackedVectorFormatPackedVectorFormat4x8Bit].desc =
        "Interpret 32-bit scalar integer operands as vectors of four 8-bit components. "
        "Vector components follow byte significance order with the lowest-numbered component "
        "stored in the least significant byte.";

    // Instructions.

    InstructionDesc[OpNop].opDesc = REMOVABLE;

    InstructionDesc[OpSource].opDesc = "Document what <<Source_Language, source language>> and text this module was translated from. "
                                       REMOVABLE
                                       GAP "_Version_ is the version of the source language. " LITERAL_UNSIGNED("It")
                                       GAP "_File_ is an <<OpString,*OpString*>> instruction and is the source-level file name."
                                       GAP "_Source_ is the text of the source-level file."
                                       GAP "Each client API specifies what form the _Version_ operand takes, per source language.";

    InstructionDesc[OpSourceContinued].opDesc = "Continue specifying the _Source_ text from the previous instruction. "
                                                REMOVABLE
                                                GAP "_Continued Source_ is a continuation of the source text in the previous _Source_."
                                                GAP "The previous instruction must be an <<OpSource, *OpSource*>> or an *OpSourceContinued* instruction. "
                                                    "As is true for all literal strings, the previous instruction's string was nul terminated. "
                                                    "That terminating nul from the previous instruction is not part of the source text; the "
                                                    "first character of _Continued Source_ logically immediately follows the last character of _Source_ before its nul.";

    InstructionDesc[OpSourceExtension].opDesc = "Document an extension to the source language. "
                                                REMOVABLE
                                                GAP "_Extension_ is a string describing a source-language extension. "
                                                "Its form is dependent on the how the source language describes extensions.";

    InstructionDesc[OpModuleProcessed].opDesc = "Document a process that was applied to a module. "
                                                REMOVABLE
                                                GAP "_Process_ is a string describing a process and/or tool (processor) that did the processing. "
                                                "Its form is dependent on the processor.";

    InstructionDesc[OpName].opDesc = "Assign a name string to another instruction's _Result <id>_. "
                                     REMOVABLE
                                     GAP "_Target_ is the _Result <id>_ to assign a name to. "
                                         "It can be the _Result <id>_ of any other instruction; a variable, function, type, intermediate result, etc."
                                     GAP "_Name_ is the string to assign.";

    InstructionDesc[OpMemberName].opDesc = "Assign a name string to a member of a structure type. "
                                           REMOVABLE
                                           GAP "_Type_ is the _<id>_ from an <<OpTypeStruct,*OpTypeStruct*>> instruction."
                                           GAP "_Member_ is the number of the member to assign in the structure. The first member is member 0, the next is member 1, ..." LITERAL_UNSIGNED("_Member_")
                                           GAP "_Name_ is the string to assign to the member.";

    InstructionDesc[OpString].opDesc = "Assign a _Result <id>_ to a string for use by other debug instructions (see <<OpLine,*OpLine*>> and <<OpSource,*OpSource*>>). "
                                       REMOVABLE "(Removal also requires removal of all instructions referencing _Result <id>_.)"
                                       GAP "_String_ is the string being assigned a _Result <id>_.";

    InstructionDesc[OpLine].opDesc = "Add source-level location information. "
                                     REMOVABLE
                                     GAP "This location information applies to the instructions physically following this instruction, "
                                         "up to the first occurrence of any of the following: "
                                         "the next end of block, the next *OpLine* instruction, or the next <<OpNoLine,*OpNoLine*>> instruction."
                                     GAP "_File_ must be an <<OpString,*OpString*>> instruction and is the source-level file name."
                                     GAP "_Line_ is the source-level line number. " LITERAL_UNSIGNED("_Line_")
                                     GAP "_Column_ is the source-level column number. " LITERAL_UNSIGNED("_Column_")
                                     GAP "*OpLine* can generally immediately precede other instructions, with the following exceptions:"
                                     GAP
                                     " - it may not be used until after the <<Annotation,annotation>> instructions,"
                                     "   (see the <<LogicalLayout, Logical Layout>> section)\n"
                                     " - must not be the last instruction in a block, which is defined to end with a <<Termination, termination instruction>>\n"
                                     " - if a branch <<Merge, merge instruction>> is used, the last *OpLine* in the block must be before its merge instruction\n";

    InstructionDesc[OpNoLine].opDesc = "Discontinue any source-level location information that might be active from a previous <<OpLine,*OpLine*>> instruction. "
                                       REMOVABLE
                                       GAP
                                       "This instruction must only appear after the <<Annotation,annotation>> instructions (see the <<LogicalLayout, Logical Layout>> section). "
                                       "It must not be the last instruction in a block, or the second-to-last instruction if the block has a <<Merge, merge instruction>>. "
                                       "There is not a requirement that there is a preceding *OpLine* instruction.";

    InstructionDesc[OpExtension].opDesc = "Declare use of an extension to SPIR-V. This allows validation of additional instructions, tokens, semantics, etc."
                                          GAP "_Name_ is the extension's name string.";

    InstructionDesc[OpExtInstImport].opDesc =
        "Import an extended set of instructions. It can be later referenced by the _Result <id>_."
        GAP "_Name_ is the extended instruction-set's name string. "
            "<<Unified, Before>> version 1.6, "
            "there must be an external specification defining the semantics for this extended instruction set. "
            "<<Unified, Starting with>> version 1.6, if _Name_ starts with \"NonSemantic.\", including the period that separates the "
            "namespace \"NonSemantic\" from the rest of the name, "
            "it is encouraged for a specification to exist on the SPIR-V Registry, "
            "but it is not required."
        GAP
            "<<Unified, Starting with>> version 1.6, an extended instruction-set name which is prefixed with \"NonSemantic.\" is guaranteed "
            "to contain only <<NonSemanticInstruction, non-semantic instructions>>, and all <<OpExtInst, *OpExtInst*>> instructions "
            "referencing this set can be ignored. "
            "All instructions within such a set must have only _<id>_ operands; no literals. "
            "When literals are needed, then the _Result <id>_ from an *OpConstant* or *OpString* instruction is referenced as appropriate. "
            "_Result <id>s_ from these non-semantic instruction-set instructions must be used only in other non-semantic instructions."
        GAP "See <<ExtInst, Extended Instruction Sets>> for more information.";

    InstructionDesc[OpCapability].opDesc = "Declare a capability used by this module."
                                           GAP "_Capability_ is the <<Capability,capability>> declared by this instruction. There are no restrictions on the order in which capabilities are declared."
                                           GAP "See the <<Capabilities,capabilities section>> for more detail.";

    InstructionDesc[OpMemoryModel].opDesc = "Set addressing model and memory model for the entire module."
                                            GAP "_Addressing Model_ selects the module's <<Addressing_Model,*Addressing Model*>>."
                                            GAP "_Memory Model_ selects the module's memory model, see <<Memory_Model,*Memory Model*>>.";

    InstructionDesc[OpEntryPoint].opDesc = "Declare an <<EntryPoint,entry point>>, its execution model, and its interface."
                            GAP "_Execution Model_ is the execution model for the entry point and its static call tree. "
                                "See <<Execution_Model,Execution Model>>."
                            GAP "_Entry Point_ must be the _Result <id>_ of an <<OpFunction,*OpFunction*>> instruction."
                            GAP "_Name_ is a name string for the entry point. "
                                "A module must not have two *OpEntryPoint* instructions with the same <<Execution_Model,Execution Model>> "
                                "and the same _Name_ string."
                            GAP "_Interface_ is a list of _<id>_ of global <<OpVariable, *OpVariable*>> instructions. "
                                "These declare the set of global variables from a module that form the interface of this entry point. "
                                "The set of _Interface <id>_ must be equal to or a superset of the global "
                                "*OpVariable* _Result <id>_ referenced by the entry point's static call tree, within the interface's storage classes. "
                                "Before *version 1.4*, the interface's storage classes are limited to the "
                                "*Input* and *Output* <<Storage_Class, storage classes>>. "
                                "Starting with *version 1.4*, the interface's storage classes are all <<Storage_Class, storage classes>> used in declaring all global variables referenced by the entry point's call tree."
                            GAP "_Interface_ _<id>_ are forward references. "
                                "Before *version 1.4*, duplication of these _<id>_ is tolerated. "
                                "Starting with *version 1.4*, an _<id>_ must not appear more than once.";

    InstructionDesc[OpExecutionMode].opDesc = "Declare an execution mode for an entry point."
                                              GAP "_Entry Point_ must be the _Entry Point <id>_ operand of an <<OpEntryPoint,*OpEntryPoint*>> instruction."
                                              GAP "_Mode_ is the execution mode. See <<Execution_Mode,Execution Mode>>."
                                              GAP "This instruction is only valid if the _Mode_ operand is an <<Execution_Mode, execution mode>> "
                                                  "that takes no *Extra Operands*, or takes *Extra Operands* that are not _<id>_ operands.";

    InstructionDesc[OpExecutionModeId].opDesc = "Declare an execution mode for an entry point, using _<id>s_ as *Extra Operands*."
                                              GAP "_Entry Point_ must be the _Entry Point <id>_ operand of an <<OpEntryPoint,*OpEntryPoint*>> instruction."
                                              GAP "_Mode_ is the execution mode. See <<Execution_Mode,Execution Mode>>."
                                              GAP "This instruction is only valid if the _Mode_ operand is an <<Execution_Mode, execution mode>> that takes *Extra Operands* that are _<id>_ operands. "
                                                  "Otherwise, use <<OpExecutionMode,*OpExecutionMode*>>.";

    InstructionDesc[OpTypeVoid].opDesc = "Declare the void type.";

    InstructionDesc[OpTypeInt].opDesc = "Declare a new <<Integer,_integer type_>>."
                                        GAP "_Width_ specifies how many bits wide the type is. "
                                            LITERAL_UNSIGNED("_Width_")
                                            "The bit pattern of a signed integer value is two's complement."
                                        GAP "_Signedness_ specifies whether there are signed semantics to preserve or validate." LINE_BREAK
                                        "0 indicates unsigned, or no signedness semantics" LINE_BREAK
                                        "1 indicates signed semantics." LINE_BREAK
                                        "In all cases, the type of operation of an instruction comes from the instruction's opcode, not the signedness of the operands.";

    InstructionDesc[OpTypeBool].opDesc = "Declare the <<Boolean,_Boolean type_>>. Values of this type can only be either *true* or *false*. "
                                         "There is no physical size or bit pattern defined for these values. "
                                         "If they are stored (in conjunction with <<OpVariable,*OpVariable*>>), they must only be used with logical addressing operations, "
                                         "not physical, and only with non-externally visible shader <<Storage_Class, storage classes>>: "
                                         "*UniformConstant*, *Workgroup*, *CrossWorkgroup*, *Private*, *Function*, *Input*, and *Output*.";
    InstructionDesc[OpTypeFloat].opDesc = "Declare a new <<Floating,_floating-point type_>>."
                                          GAP "_Width_ specifies how many bits wide the type is. "
                                                LITERAL_UNSIGNED("_Width_")
                                          GAP "_Floating Point Encoding_ specifies the bit pattern of values."
                                          GAP "Unless _Floating Point Encoding_ is present, the bit pattern of a floating-point value is the binary format described by the IEEE 754 encoding for the specified _Width_.";

    InstructionDesc[OpTypeVector].opDesc = "Declare a new <<Vector,vector type>>."
                                           GAP "_Component Type_ is the type of each component in the resulting type. It must be a <<Scalar,scalar type>>."
                                           GAP "_Component Count_ is the number of components in the resulting type. "
                                               LITERAL_UNSIGNED("_Component Count_")
                                               "It must be at least 2."
                                           GAP "Components are numbered consecutively, starting with 0.";

    InstructionDesc[OpTypeMatrix].opDesc = "Declare a new matrix type."
                                           GAP "_Column Type_ is the type of each column in the matrix. It must be vector type."
                                           GAP "_Column Count_ is the number of columns in the new matrix type. "
                                               LITERAL_UNSIGNED("_Column Count_")
                                               "It must be at least 2."
                                           GAP "Matrix columns are numbered consecutively, starting with 0. "
                                               "This is true independently of any <<Decoration,Decorations>> describing the memory layout of a matrix (e.g., *RowMajor* or *MatrixStride*).";

    InstructionDesc[OpTypeImage].opDesc = "Declare a new <<ImageTerm,image>> type. Consumed, for example, by <<OpTypeSampledImage,*OpTypeSampledImage*>>. "
                                            "This type is opaque: values of this type have no defined physical size or bit pattern."
                                            GAP "_Sampled Type_ is the type of the components that result from sampling or reading from this image type. "
                                                "Must be a scalar <<Numerical,numerical type>> or <<OpTypeVoid,*OpTypeVoid*>>."
                                            GAP "_Dim_ is the image <<Dim,dimensionality>> (Dim)."
                                            GAP "All the following literals are integers taking one operand each."
                                            GAP "_Depth_ is whether or not this image is a depth image. "
                                                "(Note that whether or not depth comparisons are actually done is a property of the sampling opcode, not of this type declaration.)"
                                                LINE_BREAK "0 indicates not a depth image"
                                                LINE_BREAK "1 indicates a depth image"
                                                LINE_BREAK "2 means no indication as to whether this is a depth or non-depth image"
                                            GAP "_Arrayed_ must be one of the following indicated values:"
                                                LINE_BREAK "0 indicates non-arrayed content"
                                                LINE_BREAK "1 indicates arrayed content"
                                            GAP "_MS_ must be one of the following indicated values:"
                                                LINE_BREAK "0 indicates single-sampled content"
                                                LINE_BREAK "1 indicates multisampled content"
                                            GAP "_Sampled_ indicates whether or not this image is accessed in combination with a <<Sampler,sampler>>, and must be one of the following values:"
                                                LINE_BREAK "0 indicates this is only known at run time, not at compile time"
                                                LINE_BREAK "1 indicates an image compatible with sampling operations"
                                                LINE_BREAK "2 indicates an image compatible with read/write operations (a storage or subpass data image)."
                                            GAP "_Image Format_ is the <<Image_Format,Image Format>>, which can be *Unknown*, as specified by the client API."
                                            GAP "If <<Dim,_Dim_>> is *SubpassData*, _Sampled_ must be 2, _Image Format_ must be *Unknown*, and "
                                                "the <<Execution_Model, Execution Model>> must be *Fragment*."
                                            GAP "_Access Qualifier_ is an image <<Access_Qualifier,*Access Qualifier*>>.";

    InstructionDesc[OpTypeSampler].opDesc = "Declare the <<Sampler,sampler>> type. Consumed by <<OpSampledImage,*OpSampledImage*>>. "
                                            "This type is opaque: values of this type have no defined physical size or bit pattern.";

    InstructionDesc[OpTypeSampledImage].opDesc = "Declare a <<SampledImage,sampled image>> type, the _Result Type_ of <<OpSampledImage,*OpSampledImage*>>, "
                                                 "or an externally combined sampler and image. "
                                                 "This type is opaque: values of this type have no defined physical size or bit pattern."
                                                 GAP "_Image Type_ must be an <<OpTypeImage,*OpTypeImage*>>. It is the type of the image in the combined sampler and image type. "
                                                 "It must not have a <<Dim,_Dim_>> of *SubpassData*. Additionally, <<Unified,starting with>> *version 1.6*, it must not have a <<Dim,_Dim_>> of *Buffer*.";

    InstructionDesc[OpTypeArray].opDesc = "Declare a new <<Array, array>> type."
                                          GAP "_Element Type_ is the type of each element in the array."
                                          GAP "_Length_ is the number of elements in the array. It must be at least 1. "
                                              "_Length_ must come from a <<ConstantInstruction,_constant instruction_>> of an <<Integer,_integer-type_>> scalar "
                                              "whose value is at least 1."
                                          GAP "Array elements are numbered consecutively, starting with 0.";

    InstructionDesc[OpTypeRuntimeArray].opDesc = "Declare a new run-time array type. Its length is not known at compile time."
                                                 GAP "If in a <<OpTypeStruct,*OpTypeStruct*>>, it must have the largest *Offset* decoration of all members in the structure."
                                                 GAP "_Element Type_ is the type of each element in the array."
                                                 GAP "See <<OpArrayLength,*OpArrayLength*>> for getting the _Length_ of an array of this type.";

    InstructionDesc[OpTypeStruct].opDesc = "Declare a new <<Structure, structure>> type."
                                           GAP "_Member N type_ is the type of member _N_ of the structure. The first member is member 0, the next is member 1, ... "
                                               "It is valid for the structure to have no members."
                                           GAP "If an operand is not yet defined, it must be defined by an <<OpTypePointer,*OpTypePointer*>>, "
                                                "where the type pointed to is an *OpTypeStruct*.";

    InstructionDesc[OpTypeOpaque].opDesc = "Declare a structure type with no body specified.";

    InstructionDesc[OpTypePointer].opDesc = "Declare a new pointer type."
                                            GAP "_Storage Class_ is the <<Storage_Class,Storage Class>> of the memory holding the object pointed to. "
                                                "If there was a forward reference to this type from an <<OpTypeForwardPointer, *OpTypeForwardPointer*>>, "
                                                "the _Storage Class_ of that instruction must equal the _Storage Class_ of this instruction."
                                            GAP "_Type_ is the type of the object pointed to.";

    InstructionDesc[OpTypeForwardPointer].opDesc =
        "Declare the <<Storage_Class, storage class>> for a forward reference to a pointer."
        GAP "_Pointer Type_ is a forward reference to the result of an <<OpTypePointer, *OpTypePointer*>>. "
            "That *OpTypePointer* instruction must declare _Pointer Type_ to be a pointer to an "
            "<<OpTypeStruct, *OpTypeStruct*>>. "
            "Any consumption of _Pointer Type_ before its *OpTypePointer* declaration must be "
            "a <<Type-Declaration, type-declaration instruction>>."
        GAP "_Storage Class_ is the <<Storage_Class,Storage Class>> of the memory holding the object pointed to.";

    InstructionDesc[OpTypeEvent].opDesc = "Declare an OpenCL event type.";

    InstructionDesc[OpTypeDeviceEvent].opDesc = "Declare an OpenCL device-side event type.";

    InstructionDesc[OpTypeReserveId].opDesc = "Declare an OpenCL reservation id type.";

    InstructionDesc[OpTypeQueue].opDesc = "Declare an OpenCL queue type.";

    InstructionDesc[OpTypePipe].opDesc = "Declare an OpenCL pipe type."
                                         GAP "_Qualifier_ is the pipe access qualifier.";

    InstructionDesc[OpTypePipeStorage].opDesc = "Declare the OpenCL pipe-storage type.";

    InstructionDesc[OpTypeNamedBarrier].opDesc = "Declare the named-barrier type.";

    InstructionDesc[OpTypeFunction].opDesc = "Declare a new function type."
                                             GAP "<<OpFunction,*OpFunction*>> uses this to declare the return type and parameter types of a function."
                                             GAP "_Return Type_ is the type of the return value of functions of this type. "
                                                 "It must be a <<Concrete, concrete>> or <<Abstract, abstract>> type, or a pointer to such a type. "
                                                 "If the function has no return value, _Return Type_ must be <<OpTypeVoid,*OpTypeVoid*>>."
                                             GAP "_Parameter N Type_ is the type _<id>_ of the type of parameter _N_. "
                                                 "It must not be <<OpTypeVoid, *OpTypeVoid*>>";

    InstructionDesc[OpConstantTrue].opDesc =  "Declare a *true* <<Boolean,_Boolean-type_>> scalar constant."
                                              GAP "_Result Type_ must be the scalar <<Boolean,_Boolean type_>>.";
    InstructionDesc[OpConstantFalse].opDesc = "Declare a *false* <<Boolean,_Boolean-type_>> scalar constant."
                                              GAP "_Result Type_ must be the scalar <<Boolean,_Boolean type_>>.";

    InstructionDesc[OpConstant].opDesc = "Declare a new <<Integer,_integer-type_>> or <<Floating,_floating-point-type_>> scalar constant."
                                         GAP "_Result Type_ must be a scalar <<Integer,_integer type_>> or <<Floating,_floating-point type_>>."
                                         GAP "_Value_ is the bit pattern for the constant. Types 32 bits wide or smaller take one word. "
                                             "Larger types take multiple words, with low-order words appearing first.";

    InstructionDesc[OpConstantComposite].opDesc = "Declare a new <<CompositeType,_composite_>> constant."
                                                  GAP "_Result Type_ must be a <<CompositeType,_composite_>> type, whose top-level members/elements/components/columns have the same type as the types of the _Constituents_. "
                                                      "The ordering must be the same between the top-level types in _Result Type_ and the _Constituents_."
                                                  GAP "_Constituents_ become members of a structure, or elements of an array, or components of a vector, or columns of a matrix. "
                                                      "There must be exactly one _Constituent_ for each top-level member/element/component/column of the result. "
                                                      "The _Constituents_ must appear in the order needed by the definition of the _Result Type_. "
                                                      "The _Constituents_ must all be _<id>s_ of non-specialization constant-instruction declarations or an <<OpUndef, *OpUndef*>>.";

    InstructionDesc[OpConstantNull].opDesc = "Declare a new _null_ constant value."
        GAP "The _null_ value is type dependent, defined as follows:\n"
        GAP
        " -  Scalar Boolean: *false*\n"
        " -  Scalar integer: 0\n"
        " -  Scalar floating point: +0.0 (all bits 0)\n"
        " -  All other scalars: Abstract\n"
        " -  Composites: Members are set recursively to the null constant according to the null value of their constituent types.\n"
        GAP "_Result Type_ must be one of the following types:"
        GAP
        " -  Scalar or vector <<Boolean,_Boolean type_>>\n"
        " -  Scalar or vector <<Integer,_integer type_>>\n"
        " -  Scalar or vector <<Floating,_floating-point type_>>\n"
        " -  Pointer type\n"
        " -  <<OpTypeEvent,_Event type_>>\n"
        " -  <<OpTypeDeviceEvent,_Device side event type_>>\n"
        " -  <<OpTypeReserveId,_Reservation id type_>>\n"
        " -  <<OpTypeQueue,_Queue type_>>\n"
        " -  <<CompositeType,_Composite type_>>\n";

    InstructionDesc[OpConstantSampler].opDesc = "Declare a new sampler constant."
                                                GAP "_Result Type_ must be <<OpTypeSampler,*OpTypeSampler*>>."
                                                GAP "_Sampler Addressing Mode_ is the addressing mode; a literal from <<Sampler_Addressing_Mode, Sampler Addressing Mode>>."
                                                GAP "_Param_ is a 32-bit integer and is one of:"
                                                LINE_BREAK "0: Non Normalized"
                                                LINE_BREAK "1: Normalized"
                                                GAP "_Sampler Filter Mode_ is the filter mode; a literal from <<Sampler_Filter_Mode, Sampler Filter Mode>>.";

    InstructionDesc[OpSpecConstantTrue].opDesc = "Declare a <<Boolean,_Boolean-type_>> scalar specialization constant with a default value of *true*."
                                                 GAP "This instruction can be specialized to become either an "
                                                     "<<OpConstantTrue,*OpConstantTrue*>> or <<OpConstantFalse,*OpConstantFalse*>> instruction."
                                                 GAP "_Result Type_ must be the scalar <<Boolean,_Boolean type_>>."
                                                 GAP "See <<SpecializationSection,Specialization>>.";

    InstructionDesc[OpSpecConstantFalse].opDesc = "Declare a <<Boolean,_Boolean-type_>> scalar specialization constant with a default value of *false*."
                                                  GAP "This instruction can be specialized to become either an "
                                                      "<<OpConstantTrue,*OpConstantTrue*>> or <<OpConstantFalse,*OpConstantFalse*>> instruction."
                                                  GAP "_Result Type_ must be the scalar <<Boolean,_Boolean type_>>."
                                                  GAP "See <<SpecializationSection,Specialization>>.";

    InstructionDesc[OpSpecConstant].opDesc = "Declare a new <<Integer,_integer-type_>> or <<Floating,_floating-point-type_>> scalar specialization constant."
                                             GAP "_Result Type_ must be a scalar <<Integer,_integer type_>> or <<Floating,_floating-point type_>>."
                                             GAP "_Value_ is the bit pattern for the default value of the constant. Types 32 bits wide or smaller take one word. "
                                                 "Larger types take multiple words, with low-order words appearing first."
                                             GAP "This instruction can be specialized to become an <<OpConstant,*OpConstant*>> instruction."
                                             GAP "See <<SpecializationSection,Specialization>>.";

    InstructionDesc[OpSpecConstantComposite].opDesc =
        "Declare a new <<CompositeType,_composite_>> specialization constant."
        GAP "_Result Type_ must be a <<CompositeType,_composite_>> type, "
            "whose top-level members/elements/components/columns have the same type as the types of the _Constituents_. "
            "The ordering must be the same between the top-level types in _Result Type_ and the _Constituents_."
        GAP "_Constituents_ become members of a structure, or elements of an array, or components of a vector, or columns of a matrix. "
            "There must be exactly one _Constituent_ for each top-level member/element/component/column of the result. "
            "The _Constituents_ must appear in the order needed by the definition of the type of the result. "
            "The _Constituents_ must be the _<id>_ of other specialization constants, constant declarations, or an <<OpUndef, **OpUndef**>>."
        GAP "This instruction will be specialized to an <<OpConstantComposite,*OpConstantComposite*>> instruction."
        GAP "See <<SpecializationSection,Specialization>>.";

    InstructionDesc[OpSpecConstantOp].opDesc = "Declare a new specialization constant that results from doing an operation."
                                               GAP "_Result Type_ must be the type required by the _Result Type_ of _Opcode_."
                                               GAP LITERAL_UNSIGNED("_Opcode_") "It must equal one of the following opcodes."
                                               LINE_BREAK "*OpSConvert*, *OpUConvert* (<<Unified, missing before>> *version 1.4*), *OpFConvert*"
                                               LINE_BREAK "*OpSNegate*, *OpNot*, *OpIAdd*, *OpISub*"
                                               LINE_BREAK "*OpIMul*, *OpUDiv*, *OpSDiv*, *OpUMod*, *OpSRem*, *OpSMod*"
                                               LINE_BREAK "*OpShiftRightLogical*, *OpShiftRightArithmetic*, *OpShiftLeftLogical*"
                                               LINE_BREAK "*OpBitwiseOr*, *OpBitwiseXor*, *OpBitwiseAnd*"
                                               LINE_BREAK "*OpVectorShuffle*, *OpCompositeExtract*, *OpCompositeInsert*"
                                               LINE_BREAK "*OpLogicalOr*, *OpLogicalAnd*, *OpLogicalNot*,"
                                               LINE_BREAK "*OpLogicalEqual*, *OpLogicalNotEqual*"
                                               LINE_BREAK "*OpSelect*"
                                               LINE_BREAK "*OpIEqual*, *OpINotEqual*"
                                               LINE_BREAK "*OpULessThan*, *OpSLessThan*"
                                               LINE_BREAK "*OpUGreaterThan*, *OpSGreaterThan*"
                                               LINE_BREAK "*OpULessThanEqual*, *OpSLessThanEqual*"
                                               LINE_BREAK "*OpUGreaterThanEqual*, *OpSGreaterThanEqual*"

                                               GAP "If the *Shader* capability was declared, *OpQuantizeToF16* is also valid."

                                               GAP "If the *Kernel* capability was declared, the following opcodes are also valid:"
                                               LINE_BREAK "*OpConvertFToS*, *OpConvertSToF*"
                                               LINE_BREAK "*OpConvertFToU*, *OpConvertUToF*"
                                               LINE_BREAK "*OpUConvert*, *OpConvertPtrToU*, *OpConvertUToPtr*"
                                               LINE_BREAK "*OpGenericCastToPtr*, *OpPtrCastToGeneric*, *OpBitcast*"
                                               LINE_BREAK "*OpFNegate*, *OpFAdd*, *OpFSub*, *OpFMul*, *OpFDiv*, *OpFRem*, *OpFMod*"
                                               LINE_BREAK "*OpAccessChain*, *OpInBoundsAccessChain*"
                                               LINE_BREAK "*OpPtrAccessChain*, *OpInBoundsPtrAccessChain*"

                                               GAP "_Operands_ are the operands required by _opcode_, and satisfy the semantics of _opcode_. "
                                                   "In addition, all _Operands_ that are _<id>s_ must be either:"
                                               GAP
                                               " - the _<id>s_ of other <<ConstantInstruction, constant instructions>>, or\n"
                                               " - *OpUndef*, when allowed by _opcode_, or\n"
                                               " - for the *AccessChain* named opcodes, their _Base_ is allowed to be a global (module scope)"
                                               "   <<OpVariable, *OpVariable*>> instruction.\n"
                                               GAP "See <<SpecializationSection,Specialization>>.";

    InstructionDesc[OpVariable].opDesc = "Allocate an object in memory, resulting in a pointer to it, which can be used with <<OpLoad,*OpLoad*>> and <<OpStore,*OpStore*>>."
                                         GAP RESULT_PTR "Its _Type_ operand is the type of object in memory."
                                         GAP "_Storage Class_ is the <<Storage_Class,Storage Class>> of the memory holding the object. It must not be *Generic*. It must be the same as the _Storage Class_ operand of the _Result Type_. "
                                             "If _Storage Class_ is *Function*, the memory is allocated on execution of the instruction for the current invocation for each dynamic instance of the function. "
                                             "The current invocation's memory is deallocated when it executes any <<FunctionTermination, function termination instruction>> of the dynamic instance of the function it was allocated by. "
                                         GAP "_Initializer_ is optional. If _Initializer_ is present, it will be the initial value of the variable's memory content. "
                                             "_Initializer_ must be an _<id>_ from a <<ConstantInstruction,_constant instruction_>> or a global (module scope) <<OpVariable, *OpVariable*>> instruction. "
                                             "_Initializer_ must have the same type as the type pointed to by _Result Type_. "
                                         GAP "If _Initializer_ is not provided, and the variable does not otherwise take a defined value (e.g. via linkage to the client API), its value is <<Poison,poison>>.";

    InstructionDesc[OpFunction].opDesc = "Add a function. This instruction must be immediately followed by one <<OpFunctionParameter,*OpFunctionParameter*>> "
                                         "instruction per each formal parameter of this function. "
                                         "This function's body or declaration terminates with the next <<OpFunctionEnd,*OpFunctionEnd*>> instruction."
                                         GAP "_Result Type_ must be the same as the _Return Type_ declared in _Function Type_."
                                         GAP "_Function Type_ is the result of an <<OpTypeFunction,*OpTypeFunction*>>, which declares the types of the return value and parameters of the function.";

    InstructionDesc[OpFunctionParameter].opDesc = "Declare a formal parameter of the current function."
                                                  GAP "_Result Type_ is the type of the parameter."
                                                  GAP "This instruction must immediately follow an <<OpFunction,*OpFunction*>> or <<OpFunctionParameter,*OpFunctionParameter*>> instruction. "
                                                      "The order of contiguous *OpFunctionParameter* instructions is the same order arguments are listed in an "
                                                      "<<OpFunctionCall,*OpFunctionCall*>> instruction to this function. "
                                                      "It is also the same order in which _Parameter Type_ operands are listed in the <<OpTypeFunction,*OpTypeFunction*>> "
                                                      "of the _Function Type_ operand for this function's <<OpFunction,*OpFunction*>> instruction."
                                                  GAP "_Result Type_ must be the same as the corresponding _Parameter Type_ operands in the <<OpTypeFunction,*OpTypeFunction*>> "
                                                      "of the _Function Type_ operand for this function's <<OpFunction,*OpFunction*>> instruction.";

    InstructionDesc[OpFunctionEnd].opDesc = "Last instruction of a function.";

    InstructionDesc[OpFunctionCall].opDesc = "Call a function."
                                             GAP "_Result Type_ is the type of the return value of the function. It must be the same as the _Return Type_ operand of the "
                                                 "_Function Type_ operand of the _Function_ operand."
                                             GAP "_Function_ is an <<OpFunction,*OpFunction*>> instruction. This could be a forward reference."
                                             GAP "_Argument N_ is the object to copy to parameter _N_ of _Function_."
                                             NOTE "A forward call is possible because there is no missing type information: "
                                                  "_Result Type_ must match the _Return Type_ of the function, and "
                                                  "the calling argument types must match the formal parameter types.";

    InstructionDesc[OpExtInst].opDesc = "Execute an instruction in an imported set of extended instructions."
        GAP "_Result Type_ is defined, per _Instruction_, in the external specification for _Set_."
        GAP "_Set_ is the result of an <<OpExtInstImport,*OpExtInstImport*>> instruction."
        GAP "_Instruction_ is the enumerant of the instruction to execute within _Set_. "
                LITERAL_UNSIGNED("It")
            "The semantics of the instruction are defined in the external specification for _Set_."
        GAP "_Operand 1, ..._ are the operands to the extended instruction.";

    InstructionDesc[OpUndef].opDesc = "Make an <<Intermediate, intermediate>> object with an <<UndefinedValue,undefined value>>."
                                      GAP "_Result Type_ is the type of object to make.  _Result Type_ can be any type except <<OpTypeVoid, *OpTypeVoid*>>.";

    #define OPT_MEMORY_OPERANDS "If present, any _Memory Operands_ must begin with a <<Memory_Operands, memory operand>> literal. " \
                                "If not present, it is the same as specifying the <<Memory_Operands, memory operand>> *None*. "

    #define TWO_MEMORY_OPERAND_GROUPS  "Before *version 1.4*, at most one <<Memory_Operands, memory operands>> mask can be provided. " \
                                       "Starting with *version 1.4* two masks can be provided, "                                       \
                                       "as described in <<Memory_Operands, *Memory Operands*>>. "                                      \
                                       "If no masks or only one mask is present, it applies to both _Source_ and _Target_. "           \
                                       "If two masks are present, the first applies to _Target_ and "                                  \
                                       "must not include *MakePointerVisible*, and the second applies to _Source_ and "                \
                                       "must not include *MakePointerAvailable*. "

    InstructionDesc[OpLoad].opDesc = "Load through a pointer."
                                     GAP "_Result Type_ is the type of the loaded object. "
                                         "It must be a type with fixed size; i.e., it must not be, nor include, "
                                         "any <<OpTypeRuntimeArray, *OpTypeRuntimeArray*>> types."
                                     GAP "_Pointer_ is the pointer to load through. Its type must be an <<OpTypePointer,*OpTypePointer*>> "
                                         "whose _Type_ operand is the same as _Result Type_."
                                     GAP OPT_MEMORY_OPERANDS;

    InstructionDesc[OpStore].opDesc = "Store through a pointer."
                                      GAP "_Pointer_ is the pointer to store through. Its type must be an <<OpTypePointer,*OpTypePointer*>> "
                                          "whose _Type_ operand is the same as the type of _Object_."
                                      GAP "_Object_ is the object to store."
                                      GAP "If the value stored in _Pointer_ is <<Poison,_Poison_>>, and _Object_ is not poison, the value stored to _Pointer_ will still be equal to _Object_."
                                      GAP OPT_MEMORY_OPERANDS;

    InstructionDesc[OpPhi].opDesc = "The SSA phi function."
                                    GAP "The result is selected based on control flow: "
                                        "If control reached the current block from _Parent i_, _Result Id_ gets the value that _Variable i_ had at the end of _Parent i_."
                                    GAP "_Result Type_ can be any type except <<OpTypeVoid, *OpTypeVoid*>>."
                                    GAP "Operands are a sequence of pairs: (_Variable 1_, _Parent 1_ block), (_Variable 2_, _Parent 2_ block), ... "
                                        "Each _Parent i_ block is the label of an immediate predecessor in the CFG of the current block. "
                                        "There must be exactly one _Parent i_ for each parent block of the current block in the CFG. "
                                        "If _Parent i_ is reachable in the CFG and _Variable i_ is defined in a block, "
                                        "that defining block must dominate _Parent i_. "
                                        "All _Variables_ must have a type matching _Result Type_."
                                    GAP "If a _Variable i_ operand is a <<Poison,poison value>>, it is only propagated to the result if control reached the current block from its corresponding _Parent_."
                                    GAP "Within a block, this instruction must appear before all non-*OpPhi* instructions (except for <<OpLine,*OpLine*>> and <<OpNoLine,*OpNoLine*>>, which can be mixed with *OpPhi*).";

    InstructionDesc[OpDecorationGroup].opDesc =
        DEPRECATED("directly use non-group decoration instructions instead")
        GAP "A collector for <<Decoration, Decorations>> from <<OpDecorate,*OpDecorate*>> "
            "instructions. "
        "All such decoration instructions targeting this *OpDecorationGroup* instruction must precede it. "
        "Subsequent <<OpGroupDecorate,*OpGroupDecorate*>> and <<OpGroupMemberDecorate,*OpGroupMemberDecorate*>> "
        "instructions that consume this instruction's _Result <id>_ will apply these decorations to their targets.";

    InstructionDesc[OpDecorate].opDesc = "Add a <<Decoration,Decoration>> to another _<id>_."
                                         GAP "_Target_ is the _<id>_ to decorate. It can potentially be any _<id>_ that is a forward reference. "
                                             "A set of decorations can be grouped together by having multiple decoration instructions targeting the same <<OpDecorationGroup,*OpDecorationGroup*>> instruction."
                                         GAP "This instruction is only valid if the _Decoration_ operand is a <<Decoration, decoration>> "
                                             "that takes no *Extra Operands*, or takes *Extra Operands* that are not _<id>_ operands.";

    InstructionDesc[OpDecorateId].opDesc = "Add a <<Decoration,Decoration>> to another _<id>_, using _<id>s_ as *Extra Operands*."
                                         GAP "_Target_ is the _<id>_ to decorate. It can potentially be any _<id>_ that is a forward reference. "
                                             "_Target_ must not be an <<OpDecorationGroup,*OpDecorationGroup*>> instruction."
                                         GAP "This instruction is only valid if the _Decoration_ operand is a <<Decoration, decoration>> "
                                             "that takes *Extra Operands* that are _<id>_ operands. "
                                             "All such _<id>_ *Extra Operands* must be <<ConstantInstruction, constant instructions>> "
                                             "or <<OpVariable, *OpVariable*>> instructions. "
                                             "All _<id>_ *Extra Operands* must appear before _Target_.";

    InstructionDesc[OpDecorateStringGOOGLE].opDesc = "Add a string <<Decoration,Decoration>> to another _<id>_."
                                         GAP "_Target_ is the _<id>_ to decorate. It can potentially be any _<id>_ that is a forward reference, "
                                             "except it must not be the _<id>_ of an <<OpDecorationGroup, *OpDecorationGroup*>>."
                                         GAP "_Decoration_ is a <<Decoration, decoration>> that takes at least one _Literal_ operand, "
                                             "and has only _Literal_ string operands.";

    InstructionDesc[OpMemberDecorateStringGOOGLE].opDesc = "Add a string <<Decoration,Decoration>> to a member of a structure type."
                                         GAP "_Structure Type_ is the _<id>_ of an <<OpTypeStruct, *OpTypeStruct*>>."
                                         GAP "_Member_ is the number of the member to decorate in the type. "
                                             LITERAL_UNSIGNED("_Member_")
                                             "The first member is member 0, the next is member 1, ..."
                                         GAP "_Decoration_ is a <<Decoration, decoration>> that takes at least one _Literal_ operand, "
                                             "and has only _Literal_ string operands.";

    InstructionDesc[OpMemberDecorate].opDesc = "Add a <<Decoration,Decoration>> to a member of a structure type."
                                               GAP "_Structure type_ is the _<id>_ of a type from <<OpTypeStruct,*OpTypeStruct*>>."
                                               GAP "_Member_ is the number of the member to decorate in the type. The first member is member 0, the next is member 1, ..."
                                               GAP "Note: See *OpDecorate* for creating groups of decorations for consumption by *OpGroupMemberDecorate*";

    InstructionDesc[OpGroupDecorate].opDesc =
        DEPRECATED("directly use non-group decoration instructions instead")
        GAP "Add a group of <<Decoration,Decorations>> to another _<id>_."
        GAP "_Decoration Group_ is the _<id>_ of an <<OpDecorationGroup,*OpDecorationGroup*>> instruction."
        GAP "_Targets_ is a list of _<id>s_ to decorate with the groups of decorations. "
            "The _Targets_ list must not include the _<id>_ of any <<OpDecorationGroup,*OpDecorationGroup*>> instruction.";

    InstructionDesc[OpGroupMemberDecorate].opDesc =
        DEPRECATED("directly use non-group decoration instructions instead")
        GAP "Add a group of <<Decoration,Decorations>> to members of structure types."
        GAP "_Decoration Group_ is the _<id>_ of an <<OpDecorationGroup,*OpDecorationGroup*>> instruction."
        GAP "_Targets_ is a list of (_<id>_, _Member_) pairs to decorate with the groups of decorations. "
            "Each _<id>_ in the pair must be a target structure type, "
            "and the associated _Member_ is the number of the member to decorate in the type. "
            "The first member is member 0, the next is member 1, ...";

    InstructionDesc[OpVectorExtractDynamic].opDesc = "Extract a single, dynamically selected, component of a vector."
        GAP "_Result Type_ must be a <<Scalar,scalar>> type."
        GAP "_Vector_ must have a type <<OpTypeVector,*OpTypeVector*>> whose _Component Type_ is _Result Type_."
        GAP "_Index_ must be a scalar <<Integer,integer>>. "
            "It is interpreted as a 0-based index of which component of _Vector_ to extract."
        GAP "<<UndefinedBehavior,Behavior is undefined>> if _Index's_ value is less than zero or greater than or equal to the number of components in _Vector_.";

    InstructionDesc[OpVectorInsertDynamic].opDesc = "Make a copy of a vector, with a single, variably selected, component modified."
        GAP "_Result Type_ must be an <<OpTypeVector,*OpTypeVector*>>."
        GAP "_Vector_ must have the same type as _Result Type_ and is the vector that the non-written components are copied from."
        GAP "_Component_ is the value supplied for the component selected by _Index_. "
            "It must have the same type as the type of components in _Result Type_."
        GAP "_Index_ must be a scalar <<Integer,integer>>. "
            "It is interpreted as a 0-based index of which component to modify."
        GAP "<<UndefinedBehavior,Behavior is undefined>> if _Index's_ value is less than zero or greater than or equal to the number of components in _Vector_.";

    InstructionDesc[OpVectorShuffle].opDesc =
        "Select arbitrary components from two vectors to make a new vector."
        GAP "_Result Type_ must be an <<OpTypeVector,*OpTypeVector*>>. "
            "The number of components in _Result Type_ must be the same as the number of _Component_ operands."
        GAP "_Vector 1_ and _Vector 2_ must both have vector types, with the same _Component Type_ as _Result Type_. "
            "They do not have to have the same number of components as _Result Type_ or with each other. "
            "They are logically concatenated, forming a single vector with _Vector 1's_ components appearing before _Vector 2's_. "
            "The components of this logical vector are logically numbered with a single consecutive set of numbers from 0 to _N_ - 1, where _N_ is the total number of components."
        GAP "_Components_ are these logical numbers (see above), selecting which of the logically numbered components form the result. "
            LITERAL_UNSIGNED("Each component")
            "They can select the components in any order and can repeat components. "
            "The first component of the result is selected by the first _Component_ operand, "
            "the second component of the result is selected by the second _Component_ operand, etc. "
            "A _Component literal_ may also be FFFFFFFF, which means the corresponding result component has no source and is undefined. "
            "All _Component literals_ must either be FFFFFFFF or in [0, _N_ - 1] (<<Inclusive, inclusive>>)."
        NOTE "A vector \"`swizzle`\" can be done by using the vector for both _Vector_ operands, "
            "or using an <<OpUndef,*OpUndef*>> for one of the _Vector_ operands.";

    InstructionDesc[OpCompositeConstruct].opDesc = "Construct a new <<CompositeType,_composite_>> object from a set of constituent objects."
                                                   GAP "_Result Type_ must be a <<CompositeType,_composite_>> type, whose top-level members/elements/components/columns have the same type as the types of the operands, with one exception. "
                                                       "The exception is that for constructing a vector, the operands may also be vectors with the same component type as the _Result Type_ component type. "
                                                       "If constructing a vector, the total number of components in all the operands must equal the number of components in _Result Type_."
                                                   GAP "_Constituents_ become members of a structure, or elements of an array, or components of a vector, or columns of a matrix. "
                                                       "There must be exactly one _Constituent_ for each top-level member/element/component/column of the result, with one exception. "
                                                       "The exception is that for constructing a vector, a contiguous subset of the scalars consumed can be represented by a vector operand instead. "
                                                       "The _Constituents_ must appear in the order needed by the definition of the type of the result. "
                                                       "If constructing a vector, there must be at least two _Constituent_ operands.";

    InstructionDesc[OpCompositeExtract].opDesc =
        "Extract a part of a <<CompositeType,_composite_>> object. "
        GAP "_Result Type_ must be the type of object selected by the last provided index. The instruction result is the extracted object."
        GAP "_Composite_ is the composite to extract from."
        GAP "_Indexes_ walk the type hierarchy, potentially down to component granularity, to select the part to extract. "
            "All indexes must be in bounds. "
            "All composite constituents use zero-based numbering, as described by their *OpType...* instruction. "
            LITERAL_UNSIGNED("Each index");

    InstructionDesc[OpCompositeInsert].opDesc =
        "Make a copy of a <<CompositeType,_composite_>> object, while modifying one part of it."
        GAP "_Result Type_ must be the same type as _Composite_."
        GAP "_Object_ is the object to use as the modified part."
        GAP "_Composite_ is the composite to copy all but the modified part from."
        GAP "_Indexes_ walk the type hierarchy of _Composite_ to the desired depth, potentially down to component granularity, to select the part to modify. "
            "All indexes must be in bounds. "
            "All composite constituents use zero-based numbering, as described by their *OpType...* instruction. "
            "The type of the part selected to modify must match the type of _Object_. "
            LITERAL_UNSIGNED("Each index");

    InstructionDesc[OpCopyObject].opDesc = "Make a copy of _Operand_. There are no pointer dereferences involved."
                                           GAP "_Result Type_ must equal _Operand_ type. _Result Type_ can be any type except <<OpTypeVoid, *OpTypeVoid*>>.";

    InstructionDesc[OpCopyLogical].opDesc = "Make a logical copy of _Operand_. There are no pointer dereferences involved."
                                           GAP "_Result Type_ must not equal the type of _Operand_ (see <<OpCopyObject, *OpCopyObject*>>), "
                                               "but _Result Type_ must _logically match_ the _Operand_ type."
                                           GAP "_Logically match_ is recursively defined by these three rules:"
                                           GAP
                                           "1. They must be either both be <<OpTypeArray, *OpTypeArray*>> or both be <<OpTypeStruct, *OpTypeStruct*>>\n"
                                           "2. If they are <<OpTypeArray, *OpTypeArray*>>:\n"
                                           "   - they must have the same _Length_ operand, and\n"
                                           "   - their _Element Type_ operands must be either the same or must _logically match_.\n"
                                           "3. If they are <<OpTypeStruct, *OpTypeStruct*>>:\n"
                                           "   - they must have the same number of _Member type_, and\n"
                                           "   - _Member N type_ for the same _N_ in the two types must be either the same or must _logically match_.\n";

    InstructionDesc[OpCopyMemory].opDesc = "Copy from the memory pointed to by _Source_ to the memory pointed to by _Target_. "
                                           "Both operands must be non-void pointers and having the same _<id> Type_ operand in their *OpTypePointer* "
                                           "type declaration. Matching Storage Class is not required. "
                                           "The amount of memory copied is the size of the type pointed to. "
                                           "The copied type must have a fixed size; i.e., it must not be, nor include, "
                                           "any <<OpTypeRuntimeArray, *OpTypeRuntimeArray*>> types."
                                           GAP
                                           OPT_MEMORY_OPERANDS
                                           TWO_MEMORY_OPERAND_GROUPS;

    InstructionDesc[OpCopyMemorySized].opDesc = "Copy from the memory pointed to by _Source_ to the memory pointed to by _Target_. "
                                                GAP "_Size_ is the number of bytes to copy. It must have a scalar <<Integer,integer type>>. "
                                                    "If it is a <<ConstantInstruction, constant instruction>>, the constant value must not be 0. "
                                                    "It is invalid for both the constant's type to have _Signedness_ of 1 and to have the sign bit set. "
                                                    "Otherwise, as a run-time value, _Size_ is treated as unsigned, and if its value is 0, no memory access is made."
                                                GAP
                                                OPT_MEMORY_OPERANDS
                                                TWO_MEMORY_OPERAND_GROUPS;

    #define IMAGE_RESULT_FLOAT_COMP  "It must be the same as _Sampled Type_ of the underlying <<OpTypeImage,*OpTypeImage*>>. "
    #define IMAGE_RESULT_FLOAT       GAP RESULT_S_FP_I IMAGE_RESULT_FLOAT_COMP
    #define IMAGE_RESULT_VEC4_COMPS  "Its components must be the same as _Sampled Type_ of the underlying <<OpTypeImage,*OpTypeImage*>> (unless that underlying _Sampled Type_ is *OpTypeVoid*). "
    #define IMAGE_RESULT_VEC4        GAP RESULT_V4_FP_I IMAGE_RESULT_VEC4_COMPS
    #define IMAGE_LOAD_STORE         "It must be a scalar or vector with component type the same as _Sampled Type_ of the <<OpTypeImage,*OpTypeImage*>> (unless that _Sampled Type_ is *OpTypeVoid*). "
    #define IMAGE_RESULT_GATHER      "It has one component per gathered texel. "

    #define SPARSE_IMAGE_STRUCT      "_Result Type_ must be an <<OpTypeStruct,*OpTypeStruct*>> with two members. " \
                                     "The first member's type " S_I " It holds a _Residency Code_ that can be passed to <<OpImageSparseTexelsResident,*OpImageSparseTexelsResident*>>. "

    #define SPARSE_IMAGE_RESULT_VEC4 GAP SPARSE_IMAGE_STRUCT "The second member " V4_FP_I IMAGE_RESULT_VEC4_COMPS

    #define SPARSE_IMAGE_RESULT_FLOAT GAP SPARSE_IMAGE_STRUCT "The second member " S_FP_I IMAGE_RESULT_FLOAT_COMP

    #define SPARSE_IMAGE_RESULT_READ GAP SPARSE_IMAGE_STRUCT "The second member " SV_FP_I IMAGE_LOAD_STORE

    #define SAMPLED_IMAGE_OPERAND    GAP "_Sampled Image_ must be an object whose type is <<OpTypeSampledImage,*OpTypeSampledImage*>>. "
    #define SAMPLED_IMAGE_DIM        "Its <<OpTypeImage,*OpTypeImage*>> must not have a <<Dim,_Dim_>> of *Buffer*. "
    #define SAMPLED_IMAGE_MS_0       "The _MS_ operand of the underlying <<OpTypeImage, *OpTypeImage*>> must be 0. "
    #define PROJ_IMAGE_OPERAND       SAMPLED_IMAGE_OPERAND "The <<Dim,_Dim_>> operand of the underlying <<OpTypeImage,*OpTypeImage*>> must be *1D*, *2D*, *3D*, or *Rect*, " \
                                         "and the _Arrayed_ and _MS_ operands must be 0."
    #define IMAGE_OPERAND            GAP "_Image_ must be an object whose type is <<OpTypeImage,*OpTypeImage*>> with a _Sampled_ operand of 0 or 2. " \
                                         "If the _Arrayed_ operand is 1, then additional capabilities may be required; " \
                                         "e.g., *ImageCubeArray*, or *ImageMSArray*. "

    #define SPARSE_IMAGE_OPERAND     GAP "_Image_ must be an object whose type is <<OpTypeImage,*OpTypeImage*>> with a _Sampled_ operand of 2. "

    #define GENERAL_IMAGE_OPERAND    GAP "_Image_ must be an object whose type is <<OpTypeImage,*OpTypeImage*>>. "

    #define COORD_CONTAINS_ARRAY     "It contains (_u_[, _v_] ... [, _array layer_]) as needed by the definition of _Sampled Image_. "
    #define COORD_CONTAINS_NO_ARRAY  "It contains (_u_[, _v_] ... ) as needed by the definition of _Sampled Image_, not including any array layer index. "
    #define COORD_CONTAINS_PROJ      "It contains (_u_[, _v_] [, _w_], _q_), as needed by the definition of _Sampled Image_, " \
                                     "with the _q_ component consumed for the projective division. "

    #define COORD_FP_OR_KERNEL       "Unless the *Kernel* <<Capability,capability>> is declared, it must be floating point. "

    #define IMAGE_READWRITE_COORD    GAP "_Coordinate_ " SV_FP_I "It contains non-normalized texel coordinates (_u_[, _v_] ... [, _array layer_]) as needed by the definition of _Image_. " \
            "See the client API specification for handling of coordinates outside the image. "
    #define IMAGE_READWRITE_COORD_32 GAP "_Coordinate_ " SV_FP_I_32 "It contains non-normalized texel coordinates (_u_[, _v_] ... [, _array layer_]) as needed by the definition of _Image_. " \
            "See the client API specification for handling of coordinates outside the image. "
    #define IMAGE_COORD_OPERAND_AI   GAP "_Coordinate_ " SV_FP_I COORD_CONTAINS_ARRAY COORD_FP_OR_KERNEL
    #define IMAGE_COORD_OPERAND_AI_32 GAP "_Coordinate_ " SV_FP_I_32 COORD_CONTAINS_ARRAY COORD_FP_OR_KERNEL
    #define IMAGE_COORD_OPERAND_FLOAT GAP "_Coordinate_ " SV_FP COORD_CONTAINS_ARRAY
    #define IMAGE_COORD_OPERAND_FLOAT_32 GAP "_Coordinate_ " SV_FP_32 COORD_CONTAINS_ARRAY
    #define IMAGE_COORD_OPERAND_INT  GAP "_Coordinate_ " SV_I COORD_CONTAINS_ARRAY
    #define IMAGE_COORD_OPERAND_INT_32 GAP "_Coordinate_ " SV_I_32 COORD_CONTAINS_ARRAY
    #define IMAGE_COORD_OPERAND_LODQ GAP "_Coordinate_ " SV_FP COORD_CONTAINS_NO_ARRAY
    #define IMAGE_COORD_OPERAND_LODQ_32 GAP "_Coordinate_ " SV_FP_32 COORD_CONTAINS_NO_ARRAY
    #define IMAGE_COORD_OPERAND_PROJ GAP "_Coordinate_ " V_FP COORD_CONTAINS_PROJ \
                                         "That is, the actual sample coordinate is (_u/q_ [, _v/q_] [, _w/q_]), as needed by the definition of _Sampled Image_. "
    #define IMAGE_COORD_OPERAND_PROJ_32 GAP "_Coordinate_ " V_FP_32 COORD_CONTAINS_PROJ \
                                         "That is, the actual sample coordinate is (_u/q_ [, _v/q_] [, _w/q_]), as needed by the definition of _Sampled Image_. "

    #define IMAGE_COORD_COMPS            "It may be a vector larger than needed, but all unused components appear after all used components. "

    #define IMAGE_OPERANDS           GAP "_Image Operands_ encodes what operands follow, as per <<Image_Operands, Image Operands>>. "
    #define IMAGE_LOD                IMAGE_OPERANDS "Either *Lod* or *Grad* image operands must be present. "
    #define IMAGE_DREF               GAP "_D~ref~_ is the depth-comparison reference value. It " S_FP_32
    #define IMAGE_PROJ_DREF          GAP "_D~ref~_ /_q_ is the depth-comparison reference value. _D~ref~_ " S_FP_32

    InstructionDesc[OpSampledImage].opDesc = "Create a <<SampledImage,sampled image>>, containing both a <<Sampler,sampler>> and an <<ImageTerm,image>>."
                                        GAP "_Result Type_ must be <<OpTypeSampledImage,*OpTypeSampledImage*>>."
                                        GAP "_Image_ is an object whose type is an <<OpTypeImage,*OpTypeImage*>>, whose _Sampled_ operand is 0 or 1, "
                                            "and whose <<Dim,_Dim_>> operand is not *SubpassData*. Additionally, <<Unified,starting with>> *version 1.6*, the <<Dim,_Dim_>> operand must not be *Buffer*."
                                        GAP "_Sampler_ must be an object whose type is <<OpTypeSampler,*OpTypeSampler*>>."
                                        GAP "If the client API does not ignore _Depth_, the _Image Type_ operand of the _Result Type_ must be the same as the type of _Image_. "
                                            "Otherwise, the type of _Image_ and the _Image Type_ operand of the _Result Type_ must be two <<OpTypeImage,*OpTypeImage*>> with all operands matching each other except for _Depth_ which can be different.";

    InstructionDesc[OpImage].opDesc = "Extract the image from a sampled image."
                                      GAP "_Result Type_ must be <<OpTypeImage,*OpTypeImage*>>."
                                      GAP "_Sampled Image_ must have type <<OpTypeSampledImage,*OpTypeSampledImage*>> whose _Image Type_ is the same as _Result Type_.";

    InstructionDesc[OpImageRead].opDesc = "Read a texel from an <<ImageTerm,image>> without a <<Sampler,sampler>>."
                                              GAP RESULT_SV_FP_I IMAGE_LOAD_STORE
                                              IMAGE_OPERAND
                                              IMAGE_READWRITE_COORD_32
                                              GAP "If the _Image_ <<Dim,_Dim_>> operand is *SubpassData*, _Coordinate_ is relative to the current fragment location. "
                                              SEE_CLIENT_API_ABOUT("on how these coordinates are applied")
                                              GAP "If the _Image_ <<Dim,_Dim_>> operand is not *SubpassData*, the <<Image_Format,_Image Format_>> must not be *Unknown*, "
                                                  "unless the *StorageImageReadWithoutFormat* or *Kernel* <<Capability, Capabilities>> were declared. "
                                              IMAGE_OPERANDS;

    InstructionDesc[OpImageWrite].opDesc = "Write a texel to an <<ImageTerm,image>> without a <<Sampler,sampler>>. "
                                              IMAGE_OPERAND "Its <<Dim,_Dim_>> operand must not be *SubpassData*. "
                                              IMAGE_READWRITE_COORD_32
                                              GAP "_Texel_ is the data to write. " IMAGE_LOAD_STORE
                                              GAP "The <<Image_Format,_Image Format_>> must not be *Unknown*, "
                                                  "unless the *StorageImageWriteWithoutFormat* or *Kernel* <<Capability, Capabilities>> were declared. "
                                              IMAGE_OPERANDS;

    InstructionDesc[OpImageSampleImplicitLod].opDesc = "Sample an image with an implicit level of detail. "
                                              GAP DERIVATIVE_TANGLED_EXEC_WAIT
                                              IMAGE_RESULT_VEC4
                                              SAMPLED_IMAGE_OPERAND SAMPLED_IMAGE_DIM SAMPLED_IMAGE_MS_0
                                              IMAGE_COORD_OPERAND_FLOAT_32 IMAGE_COORD_COMPS
                                              IMAGE_OPERANDS
                                              IMPLICIT_DERIVATIVE;

    InstructionDesc[OpImageSampleExplicitLod].opDesc = "Sample an image using an explicit level of detail. "
                                              IMAGE_RESULT_VEC4
                                              SAMPLED_IMAGE_OPERAND SAMPLED_IMAGE_DIM SAMPLED_IMAGE_MS_0
                                              IMAGE_COORD_OPERAND_AI_32 IMAGE_COORD_COMPS
                                              IMAGE_LOD;

    InstructionDesc[OpImageSampleDrefImplicitLod].opDesc = "Sample an image doing depth-comparison with an implicit level of detail. "
                                              GAP DERIVATIVE_TANGLED_EXEC_WAIT
                                              IMAGE_RESULT_FLOAT
                                              SAMPLED_IMAGE_OPERAND SAMPLED_IMAGE_DIM SAMPLED_IMAGE_MS_0
                                              IMAGE_COORD_OPERAND_FLOAT_32 IMAGE_COORD_COMPS
                                              IMAGE_DREF
                                              IMAGE_OPERANDS
                                              IMPLICIT_DERIVATIVE;

    InstructionDesc[OpImageSampleDrefExplicitLod].opDesc = "Sample an image doing depth-comparison using an explicit level of detail. "
                                              IMAGE_RESULT_FLOAT
                                              SAMPLED_IMAGE_OPERAND SAMPLED_IMAGE_DIM SAMPLED_IMAGE_MS_0
                                              IMAGE_COORD_OPERAND_FLOAT_32 IMAGE_COORD_COMPS
                                              IMAGE_DREF
                                              IMAGE_LOD;

    InstructionDesc[OpImageSampleProjImplicitLod].opDesc = "Sample an image with with a project coordinate and an implicit level of detail. "
                                              GAP DERIVATIVE_TANGLED_EXEC_WAIT
                                              IMAGE_RESULT_VEC4
                                              PROJ_IMAGE_OPERAND
                                              IMAGE_COORD_OPERAND_PROJ_32 IMAGE_COORD_COMPS
                                              IMAGE_OPERANDS
                                              IMPLICIT_DERIVATIVE;

    InstructionDesc[OpImageSampleProjExplicitLod].opDesc = "Sample an image with a project coordinate using an explicit level of detail. "
                                              IMAGE_RESULT_VEC4
                                              PROJ_IMAGE_OPERAND
                                              IMAGE_COORD_OPERAND_PROJ_32 IMAGE_COORD_COMPS
                                              IMAGE_LOD;

    InstructionDesc[OpImageSampleProjDrefImplicitLod].opDesc = "Sample an image with a project coordinate, doing depth-comparison, with an implicit level of detail. "
                                              GAP DERIVATIVE_TANGLED_EXEC_WAIT
                                              IMAGE_RESULT_FLOAT
                                              PROJ_IMAGE_OPERAND
                                              IMAGE_COORD_OPERAND_PROJ_32 IMAGE_COORD_COMPS
                                              IMAGE_PROJ_DREF
                                              IMAGE_OPERANDS
                                              IMPLICIT_DERIVATIVE;

    InstructionDesc[OpImageSampleProjDrefExplicitLod].opDesc = "Sample an image with a project coordinate, doing depth-comparison, using an explicit level of detail. "
                                              IMAGE_RESULT_FLOAT
                                              PROJ_IMAGE_OPERAND
                                              IMAGE_COORD_OPERAND_PROJ_32 IMAGE_COORD_COMPS
                                              IMAGE_PROJ_DREF
                                              IMAGE_LOD;

    InstructionDesc[OpImageFetch].opDesc =  "Fetch a single texel from an image whose _Sampled_ operand is 1. "
                                              IMAGE_RESULT_VEC4
                                              GENERAL_IMAGE_OPERAND "Its <<Dim,_Dim_>> operand must not be *Cube*, and its _Sampled_ operand must be 1. "
                                              IMAGE_COORD_OPERAND_INT_32
                                              IMAGE_OPERANDS;

    InstructionDesc[OpImageGather].opDesc =
        "Gathers the requested component from four texels. "
        IMAGE_RESULT_VEC4 IMAGE_RESULT_GATHER
        SAMPLED_IMAGE_OPERAND "Its <<OpTypeImage,*OpTypeImage*>> must have a <<Dim,_Dim_>> of *2D*, *Cube*, or *Rect*. "
        SAMPLED_IMAGE_MS_0
        IMAGE_COORD_OPERAND_FLOAT_32
        GAP "_Component_ is the component number gathered from all four texels. "
            "It " S_I_32
            "<<UndefinedBehavior,Behavior is undefined>> if its value is not 0, 1, 2 or 3. "
        IMAGE_OPERANDS;

    InstructionDesc[OpImageDrefGather].opDesc =
        "Gathers the requested depth-comparison from four texels. "
        IMAGE_RESULT_VEC4 IMAGE_RESULT_GATHER
        SAMPLED_IMAGE_OPERAND "Its <<OpTypeImage,*OpTypeImage*>> must have a <<Dim,_Dim_>> of *2D*, *Cube*, or *Rect*. "
        SAMPLED_IMAGE_MS_0
        IMAGE_COORD_OPERAND_FLOAT_32
        IMAGE_DREF
        IMAGE_OPERANDS;

    InstructionDesc[OpImageSparseSampleImplicitLod].opDesc = "Sample a sparse image with an implicit level of detail. "
                                              GAP DERIVATIVE_TANGLED_EXEC_WAIT
                                              SPARSE_IMAGE_RESULT_VEC4
                                              SAMPLED_IMAGE_OPERAND SAMPLED_IMAGE_DIM SAMPLED_IMAGE_MS_0
                                              IMAGE_COORD_OPERAND_FLOAT_32 IMAGE_COORD_COMPS
                                              IMAGE_OPERANDS
                                              IMPLICIT_DERIVATIVE;

    InstructionDesc[OpImageSparseSampleExplicitLod].opDesc = "Sample a sparse image using an explicit level of detail. "
                                              SPARSE_IMAGE_RESULT_VEC4
                                              SAMPLED_IMAGE_OPERAND SAMPLED_IMAGE_DIM SAMPLED_IMAGE_MS_0
                                              IMAGE_COORD_OPERAND_AI_32 IMAGE_COORD_COMPS
                                              IMAGE_LOD;

    InstructionDesc[OpImageSparseSampleDrefImplicitLod].opDesc = "Sample a sparse image doing depth-comparison with an implicit level of detail. "
                                              GAP DERIVATIVE_TANGLED_EXEC_WAIT
                                              SPARSE_IMAGE_RESULT_FLOAT
                                              SAMPLED_IMAGE_OPERAND SAMPLED_IMAGE_DIM SAMPLED_IMAGE_MS_0
                                              IMAGE_COORD_OPERAND_FLOAT_32 IMAGE_COORD_COMPS
                                              IMAGE_DREF
                                              IMAGE_OPERANDS
                                              IMPLICIT_DERIVATIVE;

    InstructionDesc[OpImageSparseSampleDrefExplicitLod].opDesc = "Sample a sparse image doing depth-comparison using an explicit level of detail. "
                                              SPARSE_IMAGE_RESULT_FLOAT
                                              SAMPLED_IMAGE_OPERAND SAMPLED_IMAGE_DIM SAMPLED_IMAGE_MS_0
                                              IMAGE_COORD_OPERAND_FLOAT_32 IMAGE_COORD_COMPS
                                              IMAGE_DREF
                                              IMAGE_LOD;

    InstructionDesc[OpImageSparseFetch].opDesc =  "Fetch a single texel from a sampled sparse image whose _Sampled_ operand is 1. "
                                              SPARSE_IMAGE_RESULT_VEC4
                                              GENERAL_IMAGE_OPERAND "Its <<Dim,_Dim_>> operand must not be *Cube*. "
                                              IMAGE_COORD_OPERAND_INT_32
                                              IMAGE_OPERANDS;

    InstructionDesc[OpImageSparseGather].opDesc = "Gathers the requested component from four texels of a sparse image. "
        SPARSE_IMAGE_RESULT_VEC4 IMAGE_RESULT_GATHER
        SAMPLED_IMAGE_OPERAND "Its <<OpTypeImage,*OpTypeImage*>> must have a <<Dim,_Dim_>> of *2D*, *Cube*, or *Rect*. "
        IMAGE_COORD_OPERAND_FLOAT_32
        GAP "_Component_ is the component number gathered from all four texels. "
            "It " S_I_32
            "<<UndefinedBehavior,Behavior is undefined>> if its value is not 0, 1, 2 or 3. "
        IMAGE_OPERANDS;

    InstructionDesc[OpImageSparseDrefGather].opDesc = "Gathers the requested depth-comparison from four texels of a sparse image. "
                                              SPARSE_IMAGE_RESULT_VEC4 IMAGE_RESULT_GATHER
                                              SAMPLED_IMAGE_OPERAND "Its <<OpTypeImage,*OpTypeImage*>> must have a <<Dim,_Dim_>> of *2D*, *Cube*, or *Rect*. "
                                              IMAGE_COORD_OPERAND_FLOAT_32
                                              IMAGE_DREF
                                              IMAGE_OPERANDS;

    InstructionDesc[OpImageSparseRead].opDesc = "Read a texel from a sparse <<ImageTerm,image>> without a <<Sampler,sampler>>. "
                                              SPARSE_IMAGE_RESULT_READ
                                              SPARSE_IMAGE_OPERAND
                                              IMAGE_READWRITE_COORD_32
                                              GAP "The _Image_ <<Dim,_Dim_>> operand must not be *SubpassData*. "
                                                  "The <<Image_Format,_Image Format_>> must not be *Unknown* "
                                                  "unless the *StorageImageReadWithoutFormat* or *Kernel* <<Capability, Capabilities>> were declared. "
                                              IMAGE_OPERANDS;

    InstructionDesc[OpImageSparseTexelsResident].opDesc = "Translates a _Resident Code_ into a Boolean. "
                                                          "Result is *false* if any of the texels were in uncommitted texture memory, and *true* otherwise."
                                                          GAP RESULT_S_B
                                                          GAP "_Resident Code_ is a value from an *OpImageSparse...* instruction that results in a resident code.";

    InstructionDesc[OpImageQuerySizeLod].opDesc = "Query the dimensions of _Image_ for mipmap level for _Level of Detail_."
                                                    GAP "_Result Type_ must be an <<Integer,integer type>> scalar or vector. The number of components must be"
                                                    LINE_BREAK "1 for the *1D* <<Dim, dimensionality>>,"
                                                    LINE_BREAK "2 for the *2D* and *Cube* <<Dim, dimensionalities>>,"
                                                    LINE_BREAK "3 for the *3D* <<Dim, dimensionality>>,"
                                                    LINE_BREAK "plus 1 more if the image type is arrayed. This vector is filled in with "
                                                        "(_width_ [, _height_] [, _depth_] [, _elements_]) "
                                                        "where _elements_ is the number of layers in an image array, or the number of cubes in a cube-map array."
                                                    GENERAL_IMAGE_OPERAND "Its <<Dim,_Dim_>> operand must be one of *1D*, *2D*, *3D*, or *Cube*, "
                                                        "and its _MS_ must be 0. "
                                                        "See <<OpImageQuerySize,*OpImageQuerySize*>> for querying image types without level of detail. "
                                                        "See the client API specification for additional image type restrictions."
                                                    GAP "_Level of Detail_ is used to compute which mipmap level to query and must be a 32-bit <<Integer,integer type>> scalar.";

    InstructionDesc[OpImageQuerySize].opDesc =    "Query the dimensions of _Image_, with no level of detail."
                                                    GAP "_Result Type_ must be an <<Integer,integer type>> scalar or vector. The number of components must be:"
                                                    LINE_BREAK "1 for the *1D* and *Buffer* <<Dim, dimensionalities>>,"
                                                    LINE_BREAK "2 for the *2D*, *Cube*, and *Rect* <<Dim, dimensionalities>>,"
                                                    LINE_BREAK "3 for the *3D* <<Dim, dimensionality>>,"
                                                    LINE_BREAK "plus 1 more if the image type is arrayed. This vector is filled in with (_width_ [, _height_] [, _elements_]) "
                                                               "where _elements_ is the number of layers in an image array or the number of cubes in a cube-map array."
                                                    GENERAL_IMAGE_OPERAND "Its <<Dim,_Dim_>> operand must be one of those listed under _Result Type_, above. "
                                                        "Additionally, if its _Dim_ is *1D*, *2D*, *3D*, or *Cube*, "
                                                        "it must also have either an _MS_ of 1 or a _Sampled_ of 0 or 2. "
                                                        "There is no implicit level-of-detail consumed by this instruction. "
                                                        "See <<OpImageQuerySizeLod,*OpImageQuerySizeLod*>> for querying images having level of detail. "
                                                        "See the client API specification for additional image type restrictions.";

    InstructionDesc[OpImageQueryLod].opDesc =     "Query the mipmap level and the level of detail for a hypothetical sampling of _Image_ at _Coordinate_ using an implicit level of detail."
                                                    GAP DERIVATIVE_TANGLED_EXEC_WAIT
                                                    GAP "_Result Type_ must be a two-component <<Floating,floating-point type>> vector."
                                                    LINE_BREAK "The first component of the result contains the mipmap array layer."
                                                    LINE_BREAK "The second component of the result contains the implicit level of detail relative to the base level. "
                                                    SAMPLED_IMAGE_OPERAND "Its <<OpTypeImage,*OpTypeImage*>> <<Dim,_Dim_>> operand must be one of *1D*, *2D*, *3D*, or *Cube*, "
                                                        "and its _MS_ must be 0. "
                                                    IMAGE_COORD_OPERAND_LODQ_32
                                                    IMPLICIT_DERIVATIVE;

    InstructionDesc[OpImageQueryLevels].opDesc =  "Query the number of mipmap levels accessible through _Image_."
                                                    GAP "_Result Type_ must be a scalar <<Integer,integer type>>. The result is the number of mipmap levels, "
                                                        "as specified by the client API."
                                                    GENERAL_IMAGE_OPERAND "Its <<Dim,_Dim_>> operand must be one of *1D*, *2D*, *3D*, or *Cube*, "
                                                        "and its _MS_ must be 0. "
                                                        "See the client API specification for additional image type restrictions.";

    InstructionDesc[OpImageQuerySamples].opDesc = "Query the number of samples available per texel fetch in a multisample image."
                                                    GAP "_Result Type_ must be a scalar <<Integer,integer type>>. The result is the number of samples. "
                                                    GENERAL_IMAGE_OPERAND "Its <<Dim,_Dim_>> operand must be one of *2D* and _MS_ of 1.";

    InstructionDesc[OpImageQueryFormat].opDesc = "Query the image format of an image created with an *Unknown* <<Image_Format,Image Format>>."
                                                    GAP "_Result Type_ must be a scalar <<Integer,integer type>>. "
                                                        "The resulting value is an enumerant from <<Image_Channel_Data_Type, Image Channel Data Type>>. "
                                                    GENERAL_IMAGE_OPERAND;

    InstructionDesc[OpImageQueryOrder].opDesc = "Query the channel order of an image created with an *Unknown* <<Image_Format,Image Format>>."
                                                    GAP "_Result Type_ must be a scalar <<Integer,integer type>>. "
                                                        "The resulting value is an enumerant from <<Image_Channel_Order, Image Channel Order>>. "
                                                    GENERAL_IMAGE_OPERAND;

    InstructionDesc[OpAccessChain].opDesc =
        "Create a pointer into an object, or to the object itself when _Indexes_ is empty."
        GAP RESULT_PTR "Its _Type_ operand must be the type reached by walking the "
            "_Base's_ type hierarchy down to the last provided index in _Indexes_ if _Indexes_ is not empty. "
            "Otherwise, its _Type_ operand must be the same as the _Type_ operand of the type of _Base_. "
            "Its _Storage Class_ operand must be the same as the Storage Class of _Base_."
        GAP "If _Result Type_ is an array-element pointer that is <<Decoration,decorated>> with *ArrayStride*, "
            "its _Array Stride_ must match the _Array Stride_ of the array's type. "
            "If the array's type is not decorated with *ArrayStride*, _Result Type_ also must not be decorated with *ArrayStride*."
        GAP "_Base_ must be a pointer. "
            "If _Indexes_ is not empty, _Base_ must point to the base of a <<CompositeType,_composite_>> object."
        GAP "_Indexes_ walk the type hierarchy to the desired depth, potentially down to scalar granularity. "
            "The first index in _Indexes_ selects the top-level member/element/component/column of the base composite. "
            "All composite constituents use zero-based numbering, as described by their *OpType...* instruction. "
            "The second index applies similarly to that result, and so on. "
            "Once any non-composite type is reached, there must be no remaining (unused) indexes."
        GAP "_Indexes_ can be empty, in which case no type hierarchy is walked and _Result_ points to the same object as _Base_."
        GAP "Each index in _Indexes_"
        GAP
        " - must have a scalar <<Integer, integer type>>\n"
        " - is treated as signed\n"
        " - if indexing into a structure, must be an <<OpConstant,*OpConstant*>> whose value is in bounds for selecting a member\n"
        " - if indexing into a vector, array, or matrix,"
        "   with the result type being a <<LogicalPointerType, logical pointer type>>, <<UndefinedBehavior, behavior is undefined>> if not in bounds.";

    InstructionDesc[OpInBoundsAccessChain].opDesc = "Has the same semantics as <<OpAccessChain,*OpAccessChain*>>, with the addition that the resulting pointer is known to point within the base object.";

    InstructionDesc[OpPtrAccessChain].opDesc = "Has the same semantics as <<OpAccessChain,*OpAccessChain*>>, with the addition of the _Element_ operand."
                                            GAP "_Base_ is treated as the address of an element in an array, "
                                                "and a new element address is computed from _Base_ and _Element_ to become the "
                                                "*OpAccessChain* _Base_ to walk the type hierarchy as per *OpAccessChain*. "
                                                "This computed _Base_ has the same type as the originating _Base_. "
                                            GAP "To compute the new element address, _Element_ is treated as a signed count of elements _E_, "
                                                "relative to the original _Base_ element _B_, and the address of element _B + E_ is computed "
                                                "using enough precision to avoid overflow and underflow. "
                                                "For objects in <<Storage_Class, storage classes>> requiring <<ExplicitLayout,explicit layout>>, "
                                                "the element's address or location is calculated using a stride, "
                                                "which will be the __Base__-type's _Array Stride_ if the _Base_ type is decorated with *ArrayStride*. "
                                                "For all other objects, the implementation calculates the element's address or location."
                                            GAP "With one exception, <<UndefinedBehavior, behavior is undefined>> when _B + E_ is not an element in the same array "
                                                "(same innermost array, if array types are nested) as _B_. "
                                                "The exception being when _B + E = L_, "
                                                "where _L_ is the length of the array: the address computation for element _L_ is done with the same "
                                                "stride as any other _B + E_ computation that stays within the array."
                                            GAP "If the <<Storage_Class, storage class>> of _Base_ requires an <<ExplicitLayout,explicit layout>> then its type must be "
                                                "<<Decoration,decorated>> with *ArrayStride*."
                                            GAP "If _Base_ points to a structure decorated with *Block* or *BufferBlock* and the value of _Element_ is not zero then "
                                                "_Result_ is <<Poison,poison>>."
                                            GAP "Note: If _Base_ is typed to be a pointer to an array "
                                                "and the desired operation is to select an element of that array, "
                                                "<<OpAccessChain,*OpAccessChain*>> should be directly used, "
                                                "as its first _Index_ selects the array element.";

    InstructionDesc[OpInBoundsPtrAccessChain].opDesc = "Has the same semantics as <<OpPtrAccessChain,*OpPtrAccessChain*>>, with the addition that the resulting pointer is known to point within the base object.";

    InstructionDesc[OpPtrEqual].opDesc = "Result is *true* if _Operand 1_ and _Operand 2_ have the same value. "
                                         "Result is *false* if _Operand 1_ and _Operand 2_ have different values. "
                                         GAP RESULT_S_B
                                         GAP SAME_POINTERS;

    InstructionDesc[OpPtrNotEqual].opDesc = "Result is *true* if _Operand 1_ and _Operand 2_ have different values. "
                                          "Result is *false* if _Operand 1_ and _Operand 2_ have the same value. "
                                         GAP RESULT_S_B
                                         GAP SAME_POINTERS;

    InstructionDesc[OpPtrDiff].opDesc = "Element-number subtraction: The number of elements to add to _Operand 2_ to get to _Operand 1_."
                            GAP RESULT_S_I
                                "It is computed as a signed value, as negative differences are allowed, "
                                "independently of the signed bit in the type. "
                                "The result equals the low-order _N_ bits of the correct result _R_, "
                                "where _R_ is computed with enough precision to avoid overflow and underflow "
                                "and _Result Type_ has a bitwidth of _N_ bits."
                            GAP "The units of _Result Type_ are a count of elements. "
                                "I.e., the same value you would use as the _Element_ operand to <<OpPtrAccessChain, *OpPtrAccessChain*>>."
                            GAP "The types of _Operand 1_ and _Operand 2_ must be <<OpTypePointer, *OpTypePointer*>> of exactly the same type, "
                                "and point to a type that can be aggregated into an array. "
                                "For an array of length _L_, _Operand 1_ and _Operand 2_ can point to any element in the "
                                "<<Inclusive, range>> _[0, L]_, where element _L_ is outside the array but has a representative address computed "
                                "with the same stride as elements in the array. "
                                "Additionally, _Operand 1_ must be a valid _Base_ operand of <<OpPtrAccessChain, *OpPtrAccessChain*>>. "
                                "<<UndefinedBehavior, Behavior is undefined>> if _Operand 1_ and _Operand 2_ are not pointers to element "
                                "numbers in _[0, L]_ in the same array.";

    InstructionDesc[OpSNegate].opDesc = "Signed-integer subtract of _Operand_ from zero. "
                                        GAP RESULT_SV_I
                                        GAP OPERAND_UNARY_INTEGER SAME_COMP_RESULT SAME_WIDTHS_RESULT
                                        GAP PER_COMPONENT;

    InstructionDesc[OpFNegate].opDesc = "Inverts the sign bit of _Operand_. (Note, however, that *OpFNegate* is still considered a floating-point "
                                        "instruction, and so is subject to the general floating-point rules regarding, for example, subnormals "
                                        "and NaN propagation)."
                                        GAP RESULT_SV_FP
                                        GAP MATCHING("_Operand_")
                                        GAP PER_COMPONENT;

    InstructionDesc[OpAny].opDesc = "Result is *true* if any component of _Vector_ is *true*, otherwise result is *false*. "
                                    GAP SCALAR_BOOL_RESULT
                                    GAP "_Vector_ must be a vector of <<Boolean,_Boolean type_>>.";

    InstructionDesc[OpAll].opDesc = "Result is *true* if all components of _Vector_ are *true*, otherwise result is *false*. "
                                    GAP SCALAR_BOOL_RESULT
                                    GAP "_Vector_ must be a vector of <<Boolean,_Boolean type_>>.";

    InstructionDesc[OpConvertFToU].opDesc =
        "Convert value numerically from floating point to unsigned integer, with round toward 0.0."
        GAP RESULT_SV_U "<<UndefinedBehavior, Behavior is undefined>> if _Result Type_ is not wide enough to hold the converted value."
        GAP "_Float Value_ " SV_FP SAME_COMP_RESULT
        GAP PER_COMPONENT;

    InstructionDesc[OpConvertFToS].opDesc =
        "Convert value numerically from floating point to signed integer, with round toward 0.0."
        GAP RESULT_SV_I "<<UndefinedBehavior, Behavior is undefined>> if _Result Type_ is not wide enough to hold the converted value."
        GAP "_Float Value_ " SV_FP SAME_COMP_RESULT
        GAP PER_COMPONENT;

    InstructionDesc[OpConvertSToF].opDesc = "Convert value numerically from signed integer to floating point."
                                            GAP RESULT_SV_FP
                                            GAP "_Signed Value_ " SV_I SAME_COMP_RESULT
                                            GAP PER_COMPONENT;

    InstructionDesc[OpConvertUToF].opDesc = "Convert value numerically from unsigned integer to floating point."
                                            GAP RESULT_SV_FP
                                            GAP "_Unsigned Value_ " SV_I SAME_COMP_RESULT
                                            GAP PER_COMPONENT;

    InstructionDesc[OpUConvert].opDesc = "Convert unsigned width. This is either a truncate or a zero extend."
                                         GAP RESULT_SV_U
                                         GAP "_Unsigned Value_ " SV_I SAME_COMP_RESULT DIFF_WIDTHS_RESULT
                                         GAP PER_COMPONENT;

    InstructionDesc[OpSConvert].opDesc = "Convert signed width. This is either a truncate or a sign extend."
                                         GAP RESULT_SV_I
                                         GAP "_Signed Value_ " SV_I SAME_COMP_RESULT DIFF_WIDTHS_RESULT
                                         GAP PER_COMPONENT;

    InstructionDesc[OpFConvert].opDesc = "Convert value numerically from one floating-point width to another width."
                                         GAP RESULT_SV_FP
                                         GAP "_Float Value_ " SV_FP SAME_COMP_RESULT DIFF_TYPES_RESULT
                                         GAP PER_COMPONENT;

    #define SATURATED_VALUE "Converted values outside the representable range of _Result Type_ are clamped to the nearest representable value of _Result Type_. "

    InstructionDesc[OpSatConvertSToU].opDesc = "Convert a signed integer to unsigned integer. " SATURATED_VALUE
                                               GAP RESULT_SV_I
                                               GAP "_Signed Value_ " SV_I SAME_COMP_RESULT
                                               GAP PER_COMPONENT;

    InstructionDesc[OpSatConvertUToS].opDesc = "Convert an unsigned integer to signed integer. " SATURATED_VALUE
                                               GAP RESULT_SV_I
                                               GAP "_Unsigned Value_ " SV_I SAME_COMP_RESULT
                                               GAP PER_COMPONENT;

    InstructionDesc[OpConvertPtrToU].opDesc =
        "Bit pattern-preserving conversion of a pointer to an unsigned scalar integer of possibly different bit width."
        GAP RESULT_S_U
        GAP "_Pointer_ must be a <<PhysicalPointerType, physical pointer type>>. "
            "If the bit width of _Pointer_ is smaller than that of _Result Type_, the conversion zero extends _Pointer_. "
            "If the bit width of _Pointer_ is larger than that of _Result Type_, the conversion truncates _Pointer_. "
            "For same bit width _Pointer_ and _Result Type_, this is the same as <<OpBitcast, *OpBitcast*>>.";

    InstructionDesc[OpConvertUToPtr].opDesc =
        "Bit pattern-preserving conversion of an unsigned scalar integer to a pointer. "
        GAP RESULT_PHYS_PTR
        GAP "_Integer Value_ " S_U
            "If the bit width of _Integer Value_ is smaller than that of _Result Type_, the conversion zero extends _Integer Value_. "
            "If the bit width of _Integer Value_ is larger than that of _Result Type_, the conversion truncates _Integer Value_. "
            "For same-width _Integer Value_ and _Result Type_, this is the same as <<OpBitcast,*OpBitcast*>>. "
        GAP "<<UndefinedBehavior, Behavior is undefined>> if the <<Storage_Class,storage class>> of _Result Type_ does not match the one used by the operation that produced the value of _Integer Value_.";

    InstructionDesc[OpPtrCastToGeneric].opDesc = "Convert a pointer's Storage Class to *Generic*."
                                                 GAP RESULT_PTR "Its <<Storage_Class,Storage Class>> must be *Generic*."
                                                 GAP "_Pointer_ must point to the *Workgroup*, *CrossWorkgroup*, or *Function* <<Storage_Class,Storage Class>>."
                                                 GAP "_Result Type_ and _Pointer_ must point to the same type.";

    InstructionDesc[OpGenericCastToPtr].opDesc = "Convert a pointer's Storage Class to a non-*Generic* class."
                                                 GAP RESULT_PTR "Its <<Storage_Class,Storage Class>> must be *Workgroup*, *CrossWorkgroup*, or *Function*."
                                                 GAP "_Pointer_ must point to the *Generic* <<Storage_Class,Storage Class>>."
                                                 GAP "_Result Type_ and _Pointer_ must point to the same type.";

    InstructionDesc[OpGenericCastToPtrExplicit].opDesc = "Attempts to explicitly convert _Pointer_ to _Storage_ storage-class pointer value. "
                                                         GAP RESULT_PTR "Its <<Storage_Class,Storage Class>> must be _Storage_."
                                                         GAP "_Pointer_ must have a type of <<OpTypePointer,*OpTypePointer*>> whose _Type_ is the same as the _Type_ of _Result Type_. "
                                                         "_Pointer_ must point to the *Generic* <<Storage_Class,Storage Class>>. "
                                                         "If the cast fails, the instruction result is an <<OpConstantNull, *OpConstantNull*>> pointer in the _Storage_ <<Storage_Class,Storage Class>>."
                                                         GAP "_Storage_ must be one of the following literal values from <<Storage_Class,Storage Class>>: *Workgroup*, *CrossWorkgroup*, or *Function*.";

    InstructionDesc[OpGenericPtrMemSemantics].opDesc = "Result is a valid <<Memory_Semantics_-id-, *Memory Semantics*>> which includes mask bits set for the Storage Class for the specific (non-Generic) Storage Class of _Pointer_. "
                                                       GAP "_Pointer_ must point to *Generic* <<Storage_Class,Storage Class>>."
                                                       GAP "_Result Type_ must be an <<OpTypeInt,OpTypeInt>> with 32-bit _Width_ and 0 _Signedness_.";

    InstructionDesc[OpBitcast].opDesc = "Bit pattern-preserving type conversion."
                                        GAP "_Result Type_ must be an <<OpTypePointer,*OpTypePointer*>>, or a scalar or vector of <<Numerical,_numerical-type_>>."
                                        GAP "_Operand_ must have a type of <<OpTypePointer,*OpTypePointer*>>, or a scalar or vector of <<Numerical,_numerical-type_>>. "
                                            "It must be a different type than _Result Type_."
                                        GAP        "Before *version 1.5*: If either _Result Type_ or _Operand_ is a pointer, "
                                                   "the other must be a pointer or an integer scalar."
                                        LINE_BREAK "Starting with *version 1.5*: If either _Result Type_ or _Operand_ is a pointer, "
                                                   "the other must be a pointer, an integer scalar, or an integer vector."
                                        GAP "If both _Result Type_ and the type of _Operand_ are pointers, they both must point into same <<Storage_Class,storage class>>."
                                        GAP "<<UndefinedBehavior, Behavior is undefined>> if the <<Storage_Class,storage class>> of _Result Type_ does not match the one used by the operation that produced the value of _Operand_."
                                        GAP "If _Result Type_ has the same number of components as _Operand_, they must also have the same component width, "
                                            "and results are computed per component."
                                        GAP "If _Result Type_ has a different number of components than _Operand_, "
                                            "the total number of bits in _Result Type_ must equal the total number of bits in _Operand_. "
                                            "Let _L_ be the type, either _Result Type_ or _Operand's_ type, that has the larger number of components. "
                                            "Let _S_ be the other type, with the smaller number of components. "
                                            "The number of components in _L_ must be an integer multiple of the number of components in _S_. "
                                            "The first component (that is, the only or lowest-numbered component) of _S_ maps to the first components of _L_, and so on, "
                                            "up to the last component of _S_ mapping to the last components of _L_. "
                                            "Within this mapping, any single component of _S_ (mapping to multiple components of _L_) maps its lower-ordered bits to the lower-numbered components of _L_.";

    InstructionDesc[OpQuantizeToF16].capabilities.push_back("Shader");
    InstructionDesc[OpQuantizeToF16].opDesc = "Quantize a floating-point value to what is expressible by a 16-bit floating-point value."
        GAP RESULT_SV_FP "The component width must be 32 bits and must not have a _Floating Point Encoding_ operand."
        GAP "_Value_ is the value to quantize. " MATCHING("_Value_")
        GAP "If _Value_ is an infinity, the result is the same infinity. "
            "If _Value_ is a NaN, the result is a NaN, but not necessarily the same NaN. "
            "If _Value_ is positive with a magnitude too large to represent as a 16-bit floating-point value, the result is positive infinity. "
            "If _Value_ is negative with a magnitude too large to represent as a 16-bit floating-point value, the result is negative infinity. "
            "If the magnitude of _Value_ is too small to represent as a normalized 16-bit floating-point value, the result must be either +0 or -0. "
        GAP "The *RelaxedPrecision* <<Decoration, Decoration>> has no effect on this instruction."
        GAP PER_COMPONENT;

    InstructionDesc[OpTranspose].opDesc = "Transpose a matrix."
                                          GAP "_Result Type_ must be an <<OpTypeMatrix,*OpTypeMatrix*>>."
                                          GAP "_Matrix_ must be an object of type <<OpTypeMatrix,*OpTypeMatrix*>>. "
                                              "The number of columns and the column size of _Matrix_ must be the reverse of those in _Result Type_. "
                                              "The types of the scalar components in _Matrix_ and _Result Type_ must be the same."
                                          GAP "_Matrix_ must have of type of <<OpTypeMatrix,*OpTypeMatrix*>>.";

    InstructionDesc[OpIsNan].opDesc = "Result is *true* if _x_ is a NaN for the floating-point encoding used by the type of _x_, otherwise result is *false*."
                                      GAP RESULT_SV_B
                                      GAP "_x_ " SV_FP MATCHING_COMP_COUNT
                                      GAP PER_COMPONENT;

    InstructionDesc[OpIsInf].opDesc = "Result is *true* if _x_ is an Inf for the floating-point encoding used by the type of _x_, otherwise result is *false*"
                                      GAP RESULT_SV_B
                                      GAP "_x_ " SV_FP MATCHING_COMP_COUNT
                                      GAP PER_COMPONENT;

    InstructionDesc[OpIsFinite].opDesc = "Result is *true* if _x_ is a finite number for the floating-point encoding used by the type of _x_, otherwise result is *false*."
                                         GAP RESULT_SV_B
                                         GAP "_x_ " SV_FP MATCHING_COMP_COUNT
                                         GAP PER_COMPONENT;

    InstructionDesc[OpIsNormal].opDesc = "Result is *true* if _x_ is a normal number for the floating-point encoding used by the type of _x_, otherwise result is *false*."
                                         GAP RESULT_SV_B
                                         GAP "_x_ " SV_FP MATCHING_COMP_COUNT
                                         GAP PER_COMPONENT;

    InstructionDesc[OpSignBitSet].opDesc = "Result is *true* if _x_ has its sign bit set, otherwise result is *false*."
                                           GAP RESULT_SV_B
                                           GAP "_x_ " SV_FP MATCHING_COMP_COUNT
                                           GAP PER_COMPONENT;

    InstructionDesc[OpLessOrGreater].opDesc = DEPRECATED("use <<OpFOrdNotEqual, *OpFOrdNotEqual*>>")
                                              GAP "Has the same semantics as <<OpFOrdNotEqual, *OpFOrdNotEqual*>>."
                                              GAP RESULT_SV_B
                                              GAP "_x_ " SV_FP MATCHING_COMP_COUNT
                                              GAP "_y_ must have the same type as _x_."
                                              GAP PER_COMPONENT;

    InstructionDesc[OpOrdered].opDesc = "Result is *true* if both _x_ == _x_ and _y_ == _y_ are *true*, where <<OpFOrdEqual, *OpFOrdEqual*>> is used as comparison, otherwise result is *false*."
                                        GAP RESULT_SV_B
                                        GAP "_x_ " SV_FP MATCHING_COMP_COUNT
                                        GAP "_y_ must have the same type as _x_."
                                        GAP PER_COMPONENT;

    InstructionDesc[OpUnordered].opDesc = "Result is *true* if either _x_ or _y_ is an NaN for the floating-point encoding used by the type of _x_ and _y_, otherwise result is *false*."
                                          GAP RESULT_SV_B
                                          GAP "_x_ " SV_FP MATCHING_COMP_COUNT
                                          GAP "_y_ must have the same type as _x_."
                                          GAP PER_COMPONENT;

    InstructionDesc[OpArrayLength].opDesc = "Length of a run-time array. The contents of the array are not accessed."
        GAP "_Result Type_ must be an <<OpTypeInt,OpTypeInt>> with 32- or 64-bit _Width_ and 0 _Signedness_."
        GAP "_Structure_ must be a <<LogicalPointerType, logical pointer>> to an <<OpTypeStruct,*OpTypeStruct*>> "
            "whose last member is a run-time array."
        GAP "_Array member_ is an unsigned 32-bit integer index of the last member of the structure that _Structure_ points to. "
            "That member's type must be from <<OpTypeRuntimeArray,*OpTypeRuntimeArray*>>.";

#define INT_ADD_SUB_MUL_OVERFLOW "The resulting value equals the low-order _N_ bits of the correct result _R_, " \
                                 "where _N_ is the component width and _R_ is computed with enough precision to avoid overflow and underflow."

    InstructionDesc[OpIAdd].opDesc = "Integer addition of _Operand 1_ and _Operand 2_."
                                     GAP RESULT_SV_I
                                     GAP MATCHING_BINARY_INTEGERS
                                     GAP INT_ADD_SUB_MUL_OVERFLOW
                                     GAP PER_COMPONENT;

    InstructionDesc[OpFAdd].opDesc = "Floating-point addition of _Operand 1_ and _Operand 2_."
                                     GAP RESULT_SV_FP
                                     GAP MATCHING_BINARY_OPERANDS
                                     GAP PER_COMPONENT;

    InstructionDesc[OpISub].opDesc = "Integer subtraction of _Operand 2_ from _Operand 1_."
                                     GAP RESULT_SV_I
                                     GAP MATCHING_BINARY_INTEGERS
                                     GAP INT_ADD_SUB_MUL_OVERFLOW
                                     GAP PER_COMPONENT;

    InstructionDesc[OpFSub].opDesc = "Floating-point subtraction of _Operand 2_ from _Operand 1_."
                                     GAP RESULT_SV_FP
                                     GAP MATCHING_BINARY_OPERANDS
                                     GAP PER_COMPONENT;

    InstructionDesc[OpIMul].opDesc = "Integer multiplication of _Operand 1_ and _Operand 2_."
                                     GAP RESULT_SV_I
                                     GAP MATCHING_BINARY_INTEGERS
                                     GAP INT_ADD_SUB_MUL_OVERFLOW
                                     GAP PER_COMPONENT;

    InstructionDesc[OpFMul].opDesc = "Floating-point multiplication of _Operand 1_ and _Operand 2_."
                                     GAP RESULT_SV_FP
                                     GAP MATCHING_BINARY_OPERANDS
                                     GAP PER_COMPONENT;

    InstructionDesc[OpUDiv].opDesc = "Unsigned-integer division of _Operand 1_ divided by _Operand 2_."
                                     GAP RESULT_SV_U
                                     GAP MATCHING_BINARY_OPERANDS
                                     GAP PER_COMPONENT OPERAND2_UNDEFINED_BEHAVIOR;

    InstructionDesc[OpSDiv].opDesc = "Signed-integer division of _Operand 1_ divided by _Operand 2_."
                                     GAP RESULT_SV_I
                                     GAP MATCHING_BINARY_INTEGERS
                                     GAP PER_COMPONENT OPERAND2_UNDEFINED_BEHAVIOR OPERAND2_UNDEFINED_SDIV;

    InstructionDesc[OpFDiv].opDesc = "Floating-point division of _Operand 1_ divided by _Operand 2_."
                                     GAP RESULT_SV_FP
                                     GAP MATCHING_BINARY_OPERANDS
                                     GAP PER_COMPONENT;

    InstructionDesc[OpUMod].opDesc = "Unsigned modulo operation of _Operand 1_ modulo _Operand 2_."
                                     GAP RESULT_SV_U
                                     GAP MATCHING_BINARY_OPERANDS
                                     GAP PER_COMPONENT OPERAND2_UNDEFINED_BEHAVIOR;

    InstructionDesc[OpSRem].opDesc = "Signed remainder operation for the remainder whose sign matches the sign of _Operand 1_."
                                     GAP RESULT_SV_I
                                     GAP MATCHING_BINARY_INTEGERS
                                     GAP PER_COMPONENT OPERAND2_UNDEFINED_BEHAVIOR OPERAND2_UNDEFINED_SDIV MODREM_RESULT("_Operand 1_");

    InstructionDesc[OpSMod].opDesc = "Signed remainder operation for the remainder whose sign matches the sign of _Operand 2_."
                                     GAP RESULT_SV_I
                                     GAP MATCHING_BINARY_INTEGERS
                                     GAP PER_COMPONENT OPERAND2_UNDEFINED_BEHAVIOR OPERAND2_UNDEFINED_SDIV MODREM_RESULT("_Operand 2_");

    InstructionDesc[OpFRem].opDesc = "The floating-point <<Remainder, _remainder_>> whose sign matches the sign of _Operand 1_."
                                     GAP RESULT_SV_FP
                                     GAP MATCHING_BINARY_OPERANDS
                                     GAP PER_COMPONENT OPERAND2_ZERO_UNDEFINED MODREM_RESULT("_Operand 1_");

    InstructionDesc[OpFMod].opDesc = "The floating-point <<Remainder, _remainder_>> whose sign matches the sign of _Operand 2_."
                                     GAP RESULT_SV_FP
                                     GAP MATCHING_BINARY_OPERANDS
                                     GAP PER_COMPONENT OPERAND2_ZERO_UNDEFINED MODREM_RESULT("_Operand 2_");

    InstructionDesc[OpVectorTimesScalar].opDesc = "Scale a floating-point vector."
                                                  GAP RESULT_V_FP
                                                  GAP MATCHING("_Vector_") "Each component of _Vector_ is multiplied by _Scalar_."
                                                  GAP "_Scalar_ must have the same type as the _Component Type_ in _Result Type_.";

    InstructionDesc[OpMatrixTimesScalar].opDesc = "Scale a floating-point matrix."
                                                  GAP RESULT_M_FP
                                                  GAP MATCHING("_Matrix_") "Each component in each column in _Matrix_ is multiplied by _Scalar_."
                                                  GAP "_Scalar_ must have the same type as the _Component Type_ in _Result Type_.";

    InstructionDesc[OpVectorTimesMatrix].opDesc = "Linear-algebraic _Vector X Matrix_."
                                                  GAP RESULT_V_FP
                                                  GAP "_Vector_ " MATCHING_VECTOR_COMPS "Its number of components must equal the number of components in each column in _Matrix_."
                                                  GAP "_Matrix_ " MATCHING_MATRIX_COMPS "Its number of columns must equal the number of components in _Result Type_.";

    InstructionDesc[OpMatrixTimesVector].opDesc = "Linear-algebraic _Matrix X Vector_."
                                                  GAP RESULT_V_FP
                                                  GAP "_Matrix_ " MATCHING_COLUMN
                                                  GAP "_Vector_ " MATCHING_VECTOR_COMPS "Its number of components must equal the number of columns in _Matrix_.";

    InstructionDesc[OpMatrixTimesMatrix].opDesc = "Linear-algebraic multiply of _LeftMatrix_ X _RightMatrix_."
                                                  GAP RESULT_M_FP
                                                  GAP "_LeftMatrix_ must be a matrix whose _Column Type_ is the same as the _Column Type_ in _Result Type_."
                                                  GAP "_RightMatrix_ " MATCHING_MATRIX_COMPS "Its number of columns must equal the number of columns in _Result Type_. "
                                                      "Its columns must have the same number of components as the number of columns in _LeftMatrix_.";

    InstructionDesc[OpOuterProduct].opDesc = "Linear-algebraic outer product of _Vector 1_ and _Vector 2_."
                                             GAP RESULT_M_FP
                                             GAP "_Vector 1_ must have the same type as the _Column Type_ in _Result Type_."
                                             GAP "_Vector 2_ " MATCHING_VECTOR_COMPS "Its number of components must equal the number of columns in _Result Type_.";

    InstructionDesc[OpDot].opDesc = "Dot product of _Vector 1_ and _Vector 2_."
                                    GAP RESULT_S_FP
                                    GAP "_Vector 1_ and _Vector 2_ must be vectors of the same type, and their component type must be _Result Type_.";

    InstructionDesc[OpIAddCarry].opDesc = "Result is the unsigned integer addition of _Operand 1_ and _Operand 2_, including its carry."
                                          GAP RESULT_STRUCT_SV_U
                                          GAP MATCHING_STRUCT_BINARY_INTEGERS "These are consumed as unsigned integers."
                                          GAP PER_COMPONENT
                                          GAP "Member 0 of the result gets the low-order bits (full component width) of the addition."
                                          GAP "Member 1 of the result gets the high-order (carry) bit of the result of the addition. "
                                              "That is, it gets the value 1 if the addition overflowed the component width, and 0 otherwise.";

    InstructionDesc[OpISubBorrow].opDesc = "Result is the unsigned integer subtraction of _Operand 2_ from _Operand 1_, and what it needed to borrow."
                                           GAP RESULT_STRUCT_SV_U
                                           GAP MATCHING_STRUCT_BINARY_INTEGERS "These are consumed as unsigned integers."
                                           GAP PER_COMPONENT
                                           GAP "Member 0 of the result gets the low-order bits (full component width) of the subtraction. "
                                               "That is, if _Operand 1_ is larger than _Operand 2_, member 0 gets the full value of the subtraction; "
                                               "if _Operand 2_ is larger than _Operand 1_, member 0 gets _2^w^_ + _Operand 1_ - _Operand 2_, where _w_ is the component width."
                                           GAP "Member 1 of the result gets 0 if _Operand 1_ {ge} _Operand 2_, and gets 1 otherwise.";

    InstructionDesc[OpUMulExtended].opDesc = "Result is the full value of the unsigned integer multiplication of _Operand 1_ and _Operand 2_."
                                             GAP RESULT_STRUCT_SV_U
                                             GAP MATCHING_STRUCT_BINARY_INTEGERS "These are consumed as unsigned integers."
                                             GAP PER_COMPONENT
                                             GAP "Member 0 of the result gets the low-order bits of the multiplication."
                                             GAP "Member 1 of the result gets the high-order bits of the multiplication.";

    InstructionDesc[OpSMulExtended].opDesc = "Result is the full value of the signed integer multiplication of _Operand 1_ and _Operand 2_."
                                             GAP RESULT_STRUCT_SV_I
                                             GAP MATCHING_STRUCT_BINARY_INTEGERS "These are consumed as signed integers."
                                             GAP PER_COMPONENT
                                             GAP "Member 0 of the result gets the low-order bits of the multiplication."
                                             GAP "Member 1 of the result gets the high-order bits of the multiplication.";

    InstructionDesc[OpShiftRightLogical].opDesc =
        "Shift the bits in _Base_ right by the number of bits specified in _Shift_. The most-significant bits are zero filled. "
        GAP RESULT_SV_I
        GAP OPERANDS_INTEGERS("_Base_ and _Shift_") MATCHING_WIDTH_TYPE("_Base_")
        GAP "_Shift_ is consumed as an unsigned integer. "
            "The resulting value is <<Poison,poison>> if _Shift_ is greater than or equal to the bit width of the components of _Base_."
        GAP PER_COMPONENT;

    InstructionDesc[OpShiftRightArithmetic].opDesc =
        "Shift the bits in _Base_ right by the number of bits specified in _Shift_. "
        "The most-significant bits are filled with the most-significant bit from _Base_. "
        GAP RESULT_SV_I
        GAP OPERANDS_INTEGERS("_Base_ and _Shift_") MATCHING_WIDTH_TYPE("_Base_")
        GAP "_Shift_ is treated as unsigned. "
            "The resulting value is <<Poison,poison>> if _Shift_ is greater than or equal to the bit width of the components of _Base_. "
        GAP PER_COMPONENT;

    InstructionDesc[OpShiftLeftLogical].opDesc =
        "Shift the bits in _Base_ left by the number of bits specified in _Shift_. "
        "The least-significant bits are zero filled. "
        GAP RESULT_SV_I
        GAP OPERANDS_INTEGERS("_Base_ and _Shift_") MATCHING_WIDTH_TYPE("_Base_")
        GAP "_Shift_ is treated as unsigned. "
            "The resulting value is <<Poison,poison>> if _Shift_ is greater than or equal to the bit width of the components of _Base_. "
        GAP "The number of components and bit width of _Result Type_ must match those _Base_ type. All types must be integer types."
        GAP PER_COMPONENT;

    InstructionDesc[OpLogicalOr].opDesc = "Result is *true* if either _Operand 1_ or _Operand 2_ is *true*. Result is *false* if both _Operand 1_ and _Operand 2_ are *false*."
                                          GAP RESULT_SV_B
                                          GAP MATCHING("_Operand 1_")
                                          GAP MATCHING("_Operand 2_")
                                          GAP PER_COMPONENT;

    InstructionDesc[OpLogicalAnd].opDesc = "Result is *true* if both _Operand 1_ and _Operand 2_ are *true*. "
                                           "Result is *false* if either _Operand 1_ or _Operand 2_ are *false*."
                                           GAP RESULT_SV_B
                                           GAP MATCHING("_Operand 1_")
                                           GAP MATCHING("_Operand 2_")
                                           GAP PER_COMPONENT;

    InstructionDesc[OpLogicalEqual].opDesc =    "Result is *true* if _Operand 1_ and _Operand 2_ have the same value. "
                                                "Result is *false* if _Operand 1_ and _Operand 2_ have different values."
                                                GAP RESULT_SV_B
                                                GAP MATCHING("_Operand 1_")
                                                GAP MATCHING("_Operand 2_")
                                                GAP PER_COMPONENT;

    InstructionDesc[OpLogicalNotEqual].opDesc = "Result is *true* if _Operand 1_ and _Operand 2_ have different values. "
                                                "Result is *false* if _Operand 1_ and _Operand 2_ have the same value."
                                                GAP RESULT_SV_B
                                                GAP MATCHING("_Operand 1_")
                                                GAP MATCHING("_Operand 2_")
                                                GAP PER_COMPONENT;

    InstructionDesc[OpLogicalNot].opDesc = "Result is *true* if _Operand_ is *false*. Result is *false* if _Operand_ is *true*."
                                                GAP RESULT_SV_B
                                                GAP MATCHING("_Operand_")
                                                GAP PER_COMPONENT;

    InstructionDesc[OpBitwiseOr].opDesc = "Result is 1 if either _Operand 1_ or _Operand 2_ is 1. Result is 0 if both _Operand 1_ and _Operand 2_ are 0."
                                          GAP PER_BIT
                                          GAP RESULT_SV_I
                                          MATCHING_BINARY_INTEGERS;

    InstructionDesc[OpBitwiseXor].opDesc = "Result is 1 if exactly one of _Operand 1_ or _Operand 2_ is 1. "
                                           "Result is 0 if _Operand 1_ and _Operand 2_ have the same value."
                                           GAP PER_BIT
                                           GAP RESULT_SV_I
                                           MATCHING_BINARY_INTEGERS;

    InstructionDesc[OpBitwiseAnd].opDesc = "Result is 1 if both _Operand 1_ and _Operand 2_ are 1. "
                                           "Result is 0 if either _Operand 1_ or _Operand 2_ are 0."
                                           GAP PER_BIT
                                           GAP RESULT_SV_I
                                           MATCHING_BINARY_INTEGERS;

    InstructionDesc[OpNot].opDesc = "Complement the bits of _Operand_."
                                    GAP PER_BIT
                                    GAP RESULT_SV_I
                                    GAP OPERAND_UNARY_INTEGER SAME_COMP_RESULT SAME_WIDTHS_RESULT;

    InstructionDesc[OpBitFieldInsert].opDesc =
        "Make a copy of an object, with a modified bit field that comes from another object."
        GAP PER_COMPONENT
        GAP RESULT_SV_I
        GAP MATCHING("_Base_ and _Insert_")
        GAP "Any result bits numbered outside [_Offset_, _Offset_ + _Count_ -  1] (<<Inclusive, inclusive>>) come from the corresponding bits in _Base_."
        GAP "Any result bits numbered in [_Offset_, _Offset_ + _Count_ -  1] come, in order, from the bits numbered [0, _Count_ - 1] of _Insert_."
        GAP "_Count_ " S_I "_Count_ is the number of bits taken from _Insert_. It is consumed as an unsigned value. "
            "_Count_ can be 0, in which case the result is _Base_."
        GAP "_Offset_ " S_I "_Offset_ is the lowest-order bit of the bit field.  It is consumed as an unsigned value."
        GAP "The resulting value is <<Poison,poison>> if _Count_ or _Offset_ or their sum is greater than the number of bits in the result.";

    InstructionDesc[OpBitFieldSExtract].opDesc =
        "Extract a bit field from an object, with sign extension."
        GAP PER_COMPONENT
        GAP RESULT_SV_I
        GAP MATCHING("_Base_")
        GAP "If _Count_ is greater than 0: The bits of _Base_ numbered in [_Offset_, _Offset_ + _Count_ -  1] (<<Inclusive, inclusive>>) become the bits numbered [0, _Count_ - 1] of the result. "
            "The remaining bits of the result will all be the same as bit _Offset + Count -  1_ of _Base_."
        GAP "_Count_ " S_I "_Count_ is the number of bits extracted from _Base_. It is consumed as an unsigned value. "
            "_Count_ can be 0, in which case the result is 0."
        GAP "_Offset_ " S_I "_Offset_ is the lowest-order bit of the bit field to extract from _Base_. "
            "It is consumed as an unsigned value."
        GAP "The resulting value is <<Poison,poison>> if _Count_ or _Offset_ or their sum is greater than the number of bits in the result.";

    InstructionDesc[OpBitFieldUExtract].opDesc = "Extract a bit field from an object, without sign extension."
                                               GAP "The semantics are the same as with <<OpBitFieldSExtract,*OpBitFieldSExtract*>> with the exception that there is no sign extension. "
                                                   "The remaining bits of the result will all be 0.";

    InstructionDesc[OpBitReverse].opDesc = "Reverse the bits in an object."
        GAP PER_COMPONENT
        GAP RESULT_SV_I
        GAP MATCHING("_Base_")
        GAP "The bit-number _n_ of the result is taken from bit-number _Width - 1 - n_ of _Base_, "
            "where _Width_ is the <<OpTypeInt,*OpTypeInt*>> operand of the _Result Type_.";

    InstructionDesc[OpBitCount].opDesc = "Count the number of set bits in an object."
                                               GAP PER_COMPONENT
                                               GAP RESULT_SV_I "The components must be wide enough to hold the unsigned _Width_ of _Base_ as an unsigned value. "
                                                               "That is, no sign bit is needed or counted when checking for a wide enough result width."
                                               GAP "_Base_ " SV_I "It must have the same number of components as _Result Type_."
                                               GAP "The result is the unsigned value that is the number of bits in _Base_ that are 1.";

    InstructionDesc[OpSelect].opDesc = "Select between two objects. "
                                "Before *version 1.4*, results are only computed per component."
                            GAP "Before *version 1.4*, _Result Type_ must be a pointer, scalar, or vector. "
                                "Starting with *version 1.4*, _Result Type_ can additionally be a <<CompositeType, composite>> type other than a vector."
                            GAP MATCHINGS("_Object 1_ and _Object 2_")
                            GAP "_Condition_ " SV_B
                            GAP "If _Condition_ is a scalar and *true*, the result is _Object 1_. If _Condition_ is a scalar and *false*, "
                                "the result is _Object 2_."
                            GAP "If _Condition_ is a vector, _Result Type_ must be a vector with the same number of components as _Condition_ "
                                "and the result is a mix of _Object 1_ and _Object 2_: If a component of _Condition_ is *true*, "
                                "the corresponding component in the result is taken from _Object 1_, otherwise it is taken from _Object 2_."
                            GAP "Components of _Object 1_ and _Object 2_ that are <<Poison,poison>> will not propagate to the result if they are not selected as part of the result.";

    InstructionDesc[OpIEqual].opDesc = "Integer comparison for equality."
                                       GAP RESULT_SV_B
                                       GAP MATCHING_REL_BINARY_SV_I
                                       GAP PER_COMPONENT;

    InstructionDesc[OpFOrdEqual].opDesc = "Floating-point comparison for being ordered and equal."
                                          GAP RESULT_SV_B
                                          GAP MATCHING_REL_BINARY_SV_FP
                                          GAP PER_COMPONENT;

    InstructionDesc[OpFUnordEqual].opDesc = "Floating-point comparison for being unordered or equal."
                                            GAP RESULT_SV_B
                                          GAP MATCHING_REL_BINARY_SV_FP
                                          GAP PER_COMPONENT;

    InstructionDesc[OpINotEqual].opDesc = "Integer comparison for inequality."
                                          GAP RESULT_SV_B
                                       GAP MATCHING_REL_BINARY_SV_I
                                       GAP PER_COMPONENT;

    InstructionDesc[OpFOrdNotEqual].opDesc = "Floating-point comparison for being ordered and not equal."
                                             GAP RESULT_SV_B
                                          GAP MATCHING_REL_BINARY_SV_FP
                                          GAP PER_COMPONENT;

    InstructionDesc[OpFUnordNotEqual].opDesc = "Floating-point comparison for being unordered or not equal."
                                               GAP RESULT_SV_B
                                          GAP MATCHING_REL_BINARY_SV_FP
                                          GAP PER_COMPONENT;

    InstructionDesc[OpULessThan].opDesc = "Unsigned-integer comparison if _Operand 1_ is less than _Operand 2_."
                                          GAP RESULT_SV_B
                                       GAP MATCHING_REL_BINARY_SV_I
                                       GAP PER_COMPONENT;

    InstructionDesc[OpSLessThan].opDesc = "Signed-integer comparison if _Operand 1_ is less than _Operand 2_."
                                          GAP RESULT_SV_B
                                       GAP MATCHING_REL_BINARY_SV_I
                                       GAP PER_COMPONENT;

    InstructionDesc[OpFOrdLessThan].opDesc = "Floating-point comparison if operands are ordered and _Operand 1_ is less than _Operand 2_."
                                             GAP RESULT_SV_B
                                          GAP MATCHING_REL_BINARY_SV_FP
                                          GAP PER_COMPONENT;

    InstructionDesc[OpFUnordLessThan].opDesc = "Floating-point comparison if operands are unordered or _Operand 1_ is less than _Operand 2_."
                                               GAP RESULT_SV_B
                                          GAP MATCHING_REL_BINARY_SV_FP
                                          GAP PER_COMPONENT;

    InstructionDesc[OpUGreaterThan].opDesc = "Unsigned-integer comparison if _Operand 1_ is greater than  _Operand 2_."
                                             GAP RESULT_SV_B
                                       GAP MATCHING_REL_BINARY_SV_I
                                       GAP PER_COMPONENT;

    InstructionDesc[OpSGreaterThan].opDesc = "Signed-integer comparison if _Operand 1_ is greater than  _Operand 2_."
                                             GAP RESULT_SV_B
                                       GAP MATCHING_REL_BINARY_SV_I
                                       GAP PER_COMPONENT;

    InstructionDesc[OpFOrdGreaterThan].opDesc = "Floating-point comparison if operands are ordered and _Operand 1_ is greater than  _Operand 2_."
                                                GAP RESULT_SV_B
                                          GAP MATCHING_REL_BINARY_SV_FP
                                          GAP PER_COMPONENT;

    InstructionDesc[OpFUnordGreaterThan].opDesc = "Floating-point comparison if operands are unordered or _Operand 1_ is greater than  _Operand 2_."
                                                   GAP RESULT_SV_B
                                          GAP MATCHING_REL_BINARY_SV_FP
                                          GAP PER_COMPONENT;

    InstructionDesc[OpULessThanEqual].opDesc = "Unsigned-integer comparison if _Operand 1_ is less than or equal to _Operand 2_."
                                               GAP RESULT_SV_B
                                       GAP MATCHING_REL_BINARY_SV_I
                                       GAP PER_COMPONENT;

    InstructionDesc[OpSLessThanEqual].opDesc = "Signed-integer comparison if _Operand 1_ is less than or equal to _Operand 2_."
                                               GAP RESULT_SV_B
                                       GAP MATCHING_REL_BINARY_SV_I
                                       GAP PER_COMPONENT;

    InstructionDesc[OpFOrdLessThanEqual].opDesc = "Floating-point comparison if operands are ordered and _Operand 1_ is less than or equal to _Operand 2_."
                                                  GAP RESULT_SV_B
                                          GAP MATCHING_REL_BINARY_SV_FP
                                          GAP PER_COMPONENT;

    InstructionDesc[OpFUnordLessThanEqual].opDesc = "Floating-point comparison if operands are unordered or _Operand 1_ is less than or equal to _Operand 2_."
                                                    GAP RESULT_SV_B
                                          GAP MATCHING_REL_BINARY_SV_FP
                                          GAP PER_COMPONENT;

    InstructionDesc[OpUGreaterThanEqual].opDesc = "Unsigned-integer comparison if _Operand 1_ is greater than or equal to _Operand 2_."
                                                  GAP RESULT_SV_B
                                       GAP MATCHING_REL_BINARY_SV_I
                                       GAP PER_COMPONENT;

    InstructionDesc[OpSGreaterThanEqual].opDesc = "Signed-integer comparison if _Operand 1_ is greater than or equal to _Operand 2_."
                                                  GAP RESULT_SV_B
                                       GAP MATCHING_REL_BINARY_SV_I
                                       GAP PER_COMPONENT;

    InstructionDesc[OpFOrdGreaterThanEqual].opDesc = "Floating-point comparison if operands are ordered and _Operand 1_ is greater than or equal to _Operand 2_."
                                                     GAP RESULT_SV_B
                                          GAP MATCHING_REL_BINARY_SV_FP
                                          GAP PER_COMPONENT;

    InstructionDesc[OpFUnordGreaterThanEqual].opDesc = "Floating-point comparison if operands are unordered or _Operand 1_ is greater than or equal to _Operand 2_."
                                                       GAP RESULT_SV_B
                                          GAP MATCHING_REL_BINARY_SV_FP
                                          GAP PER_COMPONENT;

    #define DP_P_AND_TYPES GAP RESULT_SV_754_FP "The component width must be 32 bits." GAP MATCHING("_P_") "_P_ is the value to take the derivative of. "

    InstructionDesc[OpDPdx].opDesc = "Same result as either <<OpDPdxFine,*OpDPdxFine*>> or <<OpDPdxCoarse,*OpDPdxCoarse*>> on _P_. Selection of which one is based on external factors."
                                     GAP DERIVATIVE_TANGLED_EXEC_WAIT
                                     DP_P_AND_TYPES
                                     FRAGMENT_ONLY;

    InstructionDesc[OpDPdy].opDesc = "Same result as either <<OpDPdyFine,*OpDPdyFine*>> or <<OpDPdyCoarse,*OpDPdyCoarse*>> on _P_. Selection of which one is based on external factors."
                                     GAP DERIVATIVE_TANGLED_EXEC_WAIT
                                     DP_P_AND_TYPES
                                     FRAGMENT_ONLY;

    InstructionDesc[OpFwidth].opDesc = "Result is the same as computing the sum of the absolute values of <<OpDPdx,*OpDPdx*>> and <<OpDPdy,*OpDPdy*>> on _P_."
                                       GAP DERIVATIVE_TANGLED_EXEC_WAIT
                                       DP_P_AND_TYPES
                                       FRAGMENT_ONLY;

    InstructionDesc[OpDPdxFine].opDesc = "Result is the partial derivative of _P_ with respect to the window _x_ coordinate. "
                                         "Uses local differencing based on the value of _P_ for the current fragment and its immediate neighbor(s)."
                                         GAP DERIVATIVE_TANGLED_EXEC_WAIT
                                         DP_P_AND_TYPES
                                         FRAGMENT_ONLY;

    InstructionDesc[OpDPdyFine].opDesc = "Result is the partial derivative of _P_ with respect to the window _y_ coordinate. "
                                         "Uses local differencing based on the value of _P_ for the current fragment and its immediate neighbor(s)."
                                         GAP DERIVATIVE_TANGLED_EXEC_WAIT
                                         DP_P_AND_TYPES
                                         FRAGMENT_ONLY;

    InstructionDesc[OpFwidthFine].opDesc = "Result is the same as computing the sum of the absolute values of <<OpDPdxFine,*OpDPdxFine*>> and <<OpDPdyFine,*OpDPdyFine*>> on _P_."
                                           GAP DERIVATIVE_TANGLED_EXEC_WAIT
                                           DP_P_AND_TYPES
                                           FRAGMENT_ONLY;

    InstructionDesc[OpDPdxCoarse].opDesc = "Result is the partial derivative of _P_ with respect to the window _x_ coordinate. "
                                           "Uses local differencing based on the value of _P_ for the current fragment's neighbors, "
                                           "and possibly, but not necessarily, includes the value of _P_ for the current fragment. "
                                           "That is, over a given area, the implementation can compute _x_ derivatives in fewer unique locations than would be allowed for <<OpDPdxFine,*OpDPdxFine*>>."
                                           GAP DERIVATIVE_TANGLED_EXEC_WAIT
                                           DP_P_AND_TYPES
                                           FRAGMENT_ONLY;

    InstructionDesc[OpDPdyCoarse].opDesc = "Result is the partial derivative of _P_ with respect to the window _y_ coordinate. "
                                           "Uses local differencing based on the value of _P_ for the current fragment's neighbors, "
                                           "and possibly, but not necessarily, includes the value of _P_ for the current fragment. "
                                           "That is, over a given area, the implementation can compute _y_ derivatives in fewer unique locations than would be allowed for <<OpDPdyFine,*OpDPdyFine*>>."
                                           GAP DERIVATIVE_TANGLED_EXEC_WAIT
                                           DP_P_AND_TYPES
                                           FRAGMENT_ONLY;

    InstructionDesc[OpFwidthCoarse].opDesc = "Result is the same as computing the sum of the absolute values of <<OpDPdxCoarse,*OpDPdxCoarse*>> and <<OpDPdyCoarse,*OpDPdyCoarse*>> on _P_."
                                             GAP DERIVATIVE_TANGLED_EXEC_WAIT
                                             DP_P_AND_TYPES
                                             FRAGMENT_ONLY;

    #define EMIT_SEMANTICS "Emits the current values of all output variables to the current output primitive. " \
                           "After execution, the values of all output variables are <<Poison,_poison_>>. "
    #define END_SEMANTICS "Finish the current primitive and start a new one. No vertex is emitted. "
    #define STREAM_SEMANTICS "_Stream_ must be an _<id>_ of a <<ConstantInstruction,_constant instruction_>> with a scalar integer type. " \
                             "That constant is the output-primitive stream number. "
    #define SINGLE_STREAM "This instruction must only be used when only one stream is present. "
    #define MULTIPLE_STREAMS "This instruction must only be used when multiple streams are present. "
    #define WS_SCOPE "_Execution_ is the <<Scope, scope>> defining the <<ScopeRestrictedTangle, scope restricted tangle>> affected by this command. "


    InstructionDesc[OpEmitVertex].opDesc = EMIT_SEMANTICS
                                           GAP SINGLE_STREAM;

    InstructionDesc[OpEndPrimitive].opDesc = END_SEMANTICS
                                             GAP SINGLE_STREAM;

    InstructionDesc[OpEmitStreamVertex].opDesc = EMIT_SEMANTICS
                                                 GAP STREAM_SEMANTICS
                                                 GAP MULTIPLE_STREAMS;

    InstructionDesc[OpEndStreamPrimitive].opDesc = END_SEMANTICS
                                                   GAP STREAM_SEMANTICS
                                                   GAP MULTIPLE_STREAMS;

    InstructionDesc[OpControlBarrier].opDesc = "Control the order that memory accesses are observed between multiple "
            "invocations in the same <<ScopeRestrictedTangle, scope restricted tangle>>."
        GAP WS_SCOPE
            "When _Execution_ is *Workgroup* or larger, <<UndefinedBehavior,behavior is undefined>> unless all "
            "invocations within _Execution_ execute the same dynamic instance of this instruction."
        GAP "Ensures that memory accesses and barriers issued before this instruction by any invocation in the <<ScopeRestrictedTangle, "
            "scope restricted tangle>> are observed before all memory accesses and barriers issued after this instruction. "
            "This control is ensured only for memory accesses and barriers issued by invocations in the <<ScopeRestrictedTangle, "
            "scope restricted tangle>> and observed by another invocation executing within _Memory_ scope. "
            "If the *Vulkan* <<Memory_Model, memory model>> is declared, this ordering only applies to memory "
            "accesses that use the *NonPrivatePointer* <<Memory_Operands, memory operand>> or *NonPrivateTexel* "
            "<<Image_Operands, image operand>>."
        GAP "_Semantics_ declares what kind of memory is being controlled and what kind of control to apply."
        GAP "Before *version 1.3*, it is only valid to use this instruction with *TessellationControl*, "
            "*GLCompute*, or *Kernel* <<Execution_Model, execution models>>. "
            "There is no such restriction starting with *version 1.3*."
        GAP "If used with the *TessellationControl* <<Execution_Model, execution model>>, "
            "it also implicitly synchronizes the *Output* <<Storage_Class, Storage Class>>: "
            "Writes to *Output* variables performed by any invocation executed prior to a *OpControlBarrier* "
            "are visible to any other invocation proceeding beyond that *OpControlBarrier*.";

    InstructionDesc[OpMemoryBarrier].opDesc = "Control the order that memory accesses are observed."
        GAP "Ensures that memory accesses and barriers issued before this instruction are observed before memory accesses "
            "and barriers issued after this instruction. "
            "This control is ensured only for memory accesses and barriers issued by this <<Invocation, invocation>> and observed "
            "by another invocation executing within _Memory_ scope. "
            "If the *Vulkan* <<Memory_Model, memory model>> is declared, this ordering only applies to memory "
            "accesses that use the *NonPrivatePointer* <<Memory_Operands, memory operand>> or *NonPrivateTexel* "
            "<<Image_Operands, image operand>>."
        GAP "_Semantics_ declares what kind of memory is being controlled and what kind of control to apply."
        GAP "To require this ordering across multiple invocations, see <<OpControlBarrier,*OpControlBarrier*>>.";

    InstructionDesc[OpImageTexelPointer].opDesc = "Form a pointer to a texel of an image. Use of such a pointer is limited to atomic operations."
                                             GAP "_Result Type_ must be an <<OpTypePointer,*OpTypePointer*>> whose <<Storage_Class, _Storage Class_>> operand is *Image*. "
                                                 "Its _Type_ operand must be a scalar <<Numerical,numerical type>> or <<OpTypeVoid,*OpTypeVoid*>>."
                                             GAP "_Image_ must have a type of <<OpTypePointer,*OpTypePointer*>> with _Type_ <<OpTypeImage, *OpTypeImage*>>. "
                                                 "The _Sampled Type_ of the type of _Image_ must be the same as the _Type_ pointed to by _Result Type_. "
                                                 "The <<Dim,_Dim_>> operand of _Type_ must not be *SubpassData*."
                                             GAP "_Coordinate_ and _Sample_ specify which texel and sample within the image to form a pointer to."
                                             GAP "_Coordinate_ " SV_I
                                                 "It must have the number of components specified below, given the following _Arrayed_ and <<Dim,_Dim_>> operands of the type of the <<OpTypeImage, *OpTypeImage*>>."
                                                LINE_BREAK
                                                LINE_BREAK "If _Arrayed_ is 0:"
                                                LINE_BREAK "*1D*: scalar"
                                                LINE_BREAK "*2D*: 2 components"
                                                LINE_BREAK "*3D*: 3 components"
                                                LINE_BREAK "*Cube*: 3 components"
                                                LINE_BREAK "*Rect*: 2 components"
                                                LINE_BREAK "*Buffer*: scalar"
                                                LINE_BREAK
                                                LINE_BREAK "If _Arrayed_ is 1:"
                                                LINE_BREAK "*1D*: 2 components"
                                                LINE_BREAK "*2D*: 3 components"
                                                LINE_BREAK "*Cube*: 3 components; the face and layer combine into the 3rd component, _layer_face_, such that "
                                                           "face is _layer_face_ % 6 and layer is floor(_layer_face_ / 6)"
                                             GAP "_Sample_ " S_I "It specifies which sample to select at the given coordinate. "
                                                 "<<UndefinedBehavior,Behavior is undefined>> unless it is a valid _<id>_ for the value 0 when the "
                                                 "<<OpTypeImage, *OpTypeImage*>> has _MS_ of 0.";

    #define ATOMIC_POINTER "_Pointer_ must be a pointer, pointing to the object to operate on."

    #define ATOMIC_OP(op, cond) "Perform the following steps atomically with respect to any other atomic accesses within _Memory_ to the same location: "\
                          GAP \
                          "1. load through _Pointer_ to get an _Original Value_,\n" \
                          "2. get a _New Value_ " op ", and\n" \
                          "3. store the _New Value_ back through _Pointer_" cond ".\n" \
                          GAP "The instruction's result is the _Original Value_."

    #define EQUAL_MEMORY_SEMANTICS "Use _Equal_ for the memory semantics of this instruction when _Value_ and _Original Value_ compare equal. "
    #define UNEQUAL_MEMORY_SEMANTICS "Use _Unequal_ for the memory semantics of this instruction when _Value_ and _Original Value_ compare unequal. _Unequal_ must not be set to *Release* or *Acquire and Release*. In addition, _Unequal_ cannot be set to a stronger memory-order then _Equal_. "
    #define POINTER_MATCHES_VALUE    "The type of _Value_ and the type pointed to by _Pointer_ must be the same type. "
    #define POINTER_MATCHES_RESULT_AND_VALUE(result) result GAP MATCHING("_Value_") MATCHING("the value pointed to by _Pointer_")
    #define MEMORY_SCOPE "_Memory_ is a memory <<Scope_-id-, _Scope_>>. "

    InstructionDesc[OpAtomicLoad].opDesc = "Atomically load through _Pointer_ using the given _Semantics_. "
                                           "All subparts of the value that is loaded are read atomically with respect to all other atomic accesses to it within _Memory_."
                                           GAP RESULT_S_FP_I
                                           GAP "_Pointer_ is the pointer to the memory to read. " MATCHING("the value pointed to by _Pointer_")
                                           GAP MEMORY_SCOPE;

    InstructionDesc[OpAtomicStore].opDesc = "Atomically store through _Pointer_ using the given _Semantics_. "
                                            "All subparts of _Value_ are written atomically with respect to all other atomic accesses to it within _Memory_."
                                           GAP "_Pointer_ is the pointer to the memory to write. The type it points to " S_FP_I
                                           GAP "_Value_ is the value to write. " POINTER_MATCHES_VALUE
                                           GAP MEMORY_SCOPE;

    InstructionDesc[OpAtomicExchange].opDesc = ATOMIC_OP("from copying _Value_", "")
                                        GAP POINTER_MATCHES_RESULT_AND_VALUE(RESULT_S_FP_I)
                                        GAP MEMORY_SCOPE;

    InstructionDesc[OpAtomicCompareExchange].opDesc = ATOMIC_OP("from _Value_ only if _Original Value_ equals _Comparator_", " only if _Original Value_ equaled _Comparator_")
                                                GAP RESULT_S_I
                                                GAP EQUAL_MEMORY_SEMANTICS
                                                GAP UNEQUAL_MEMORY_SEMANTICS
                                                GAP MATCHING("_Value_") MATCHING("the value pointed to by _Pointer_")
                                                "This type must also match the type of _Comparator_."
                                                GAP MEMORY_SCOPE;

    InstructionDesc[OpAtomicCompareExchangeWeak].opDesc = DEPRECATED("use <<OpAtomicCompareExchange, *OpAtomicCompareExchange*>>")
                                                          GAP "Has the same semantics as <<OpAtomicCompareExchange, *OpAtomicCompareExchange*>>."
                                                          GAP MEMORY_SCOPE;

//    InstructionDesc[OpAtomicCompareExchangeWeak].opDesc = "Attempts to do the following:"
//                                                          GAP ATOMIC_OP("by selecting _Value_ if _Original Value_ equals _Comparator_ or selecting _Original Value_ otherwise")
//                                                          GAP "The weak compare-and-exchange operations may fail spuriously. That is, even when _Original Value_ equals _Comparator_ the comparison can fail and store back the _Original Value_ through _Pointer_."
//                                                          GAP RESULT_S_I
//                                                          GAP EQUAL_MEMORY_SEMANTICS
//                                                          GAP UNEQUAL_MEMORY_SEMANTICS
//                                                          GAP MATCHING("_Value_") MATCHING("the value pointed to by _Pointer_")
//                                                          "This type must also match the type of _Comparator_.";

    InstructionDesc[OpAtomicIIncrement].opDesc = ATOMIC_OP("through integer addition of _1_ to _Original Value_", "")
                                                 GAP RESULT_S_I MATCHING("the value pointed to by _Pointer_")
                                                 GAP MEMORY_SCOPE;

    InstructionDesc[OpAtomicIDecrement].opDesc = ATOMIC_OP("through integer subtraction of _1_ from _Original Value_", "")
                                                GAP RESULT_S_I MATCHING("the value pointed to by _Pointer_")
                                                GAP MEMORY_SCOPE;

    InstructionDesc[OpAtomicIAdd].opDesc = ATOMIC_OP("by integer addition of _Original Value_ and _Value_", "")
                                            GAP POINTER_MATCHES_RESULT_AND_VALUE(RESULT_S_I)
                                            GAP MEMORY_SCOPE;

    InstructionDesc[OpAtomicISub].opDesc = ATOMIC_OP("by integer subtraction of _Value_ from _Original Value_", "")
                                            GAP POINTER_MATCHES_RESULT_AND_VALUE(RESULT_S_I)
                                            GAP MEMORY_SCOPE;

    InstructionDesc[OpAtomicUMin].opDesc = ATOMIC_OP("by finding the smallest unsigned integer of _Original Value_ and _Value_", "")
                                            GAP POINTER_MATCHES_RESULT_AND_VALUE(RESULT_S_I)
                                            GAP MEMORY_SCOPE;

    InstructionDesc[OpAtomicUMax].opDesc = ATOMIC_OP("by finding the largest unsigned integer of _Original Value_ and _Value_", "")
                                            GAP POINTER_MATCHES_RESULT_AND_VALUE(RESULT_S_I)
                                            GAP MEMORY_SCOPE;

    InstructionDesc[OpAtomicSMin].opDesc = ATOMIC_OP("by finding the smallest signed integer of _Original Value_ and _Value_", "")
                                            GAP POINTER_MATCHES_RESULT_AND_VALUE(RESULT_S_I)
                                            GAP MEMORY_SCOPE;

    InstructionDesc[OpAtomicSMax].opDesc = ATOMIC_OP("by finding the largest signed integer of _Original Value_ and _Value_", "")
                                            GAP POINTER_MATCHES_RESULT_AND_VALUE(RESULT_S_I)
                                            GAP MEMORY_SCOPE;

    InstructionDesc[OpAtomicAnd].opDesc = ATOMIC_OP("by the bitwise AND of _Original Value_ and _Value_", "")
                                            GAP POINTER_MATCHES_RESULT_AND_VALUE(RESULT_S_I)
                                            GAP MEMORY_SCOPE;

    InstructionDesc[OpAtomicOr].opDesc = ATOMIC_OP("by the bitwise OR of _Original Value_ and _Value_", "")
                                            GAP POINTER_MATCHES_RESULT_AND_VALUE(RESULT_S_I)
                                            GAP MEMORY_SCOPE;

    InstructionDesc[OpAtomicXor].opDesc = ATOMIC_OP("by the bitwise exclusive OR of _Original Value_ and _Value_", "")
                                            GAP POINTER_MATCHES_RESULT_AND_VALUE(RESULT_S_I)
                                            GAP MEMORY_SCOPE;

    InstructionDesc[OpAtomicFlagTestAndSet].opDesc =
        "Atomically sets the flag value pointed to by _Pointer_ to the set state."
        GAP "_Pointer_ must be a pointer to a 32-bit integer type representing an atomic flag."
        GAP "The instruction's result is true if the flag was in the set state or false if the flag was in the clear state immediately before the operation."
        GAP "_Result Type_ must be a <<Boolean, _Boolean type_>>."
        GAP "The resulting values are <<Poison,_poison_>> if an atomic flag is modified by an instruction other than "
            "<<OpAtomicFlagTestAndSet,*OpAtomicFlagTestAndSet*>> or <<OpAtomicFlagClear,*OpAtomicFlagClear*>>."
        GAP MEMORY_SCOPE;

    InstructionDesc[OpAtomicFlagClear].opDesc =
        "Atomically sets the flag value pointed to by _Pointer_ to the clear state."
        GAP "_Pointer_ must be a pointer to a 32-bit integer type representing an atomic flag."
        GAP "Memory Semantics must not be <<Memory_Semantics_-id-, *Acquire*>> or <<Memory_Semantics_-id-, *AcquireRelease*>>"
        GAP "The resulting values are <<Poison,_poison_>> if an atomic flag is modified by an instruction other than "
            "<<OpAtomicFlagTestAndSet,*OpAtomicFlagTestAndSet*>> or <<OpAtomicFlagClear,*OpAtomicFlagClear*>>."
        GAP MEMORY_SCOPE;

    InstructionDesc[OpLoopMerge].opDesc = "Declare a structured loop."
                                          GAP "This instruction must immediately precede either an <<OpBranch,*OpBranch*>> or <<OpBranchConditional,*OpBranchConditional*>> instruction. "
                                              "That is, it must be the second-to-last instruction in its block."
                                          GAP "_Merge Block_ is the label of the merge block for this structured loop."
                                          GAP "_Continue Target_ is the label of a block targeted for processing a loop \"continue\"."
                                          GAP "_Loop Control Parameters_ appear in <<Loop_Control, Loop Control>>-table order for any _Loop Control_ setting that requires such a parameter."
                                          GAP "See <<StructuredControlFlow,Structured Control Flow>> for more detail.";

    InstructionDesc[OpSelectionMerge].opDesc = "Declare a structured selection."
                                               GAP "This instruction must immediately precede either an <<OpBranchConditional,*OpBranchConditional*>> or <<OpSwitch,*OpSwitch*>> instruction. "
                                                   "That is, it must be the second-to-last instruction in its block."
                                               GAP "_Merge Block_ is the label of the merge block for this structured selection."
                                               GAP "See <<StructuredControlFlow,Structured Control Flow>> for more detail.";

    InstructionDesc[OpLabel].opDesc =
        "The label instruction of a <<Block, block>>."
        GAP
        "References to a block are through the _Result <id>_ of its label.";

    #define LAST_INSTRUCTION GAP "This instruction must be the last instruction in a block."

    InstructionDesc[OpBranch].opDesc = "Unconditional branch to _Target Label_."
                                       GAP "_Target Label_ must be the _Result <id>_ of an <<OpLabel,*OpLabel*>> instruction in the current function."
                                       LAST_INSTRUCTION;

    InstructionDesc[OpBranchConditional].opDesc = "If _Condition_ is *true*, branch to _True Label_, otherwise branch to _False Label_."
                                                  GAP "_Condition_ must be a <<Boolean,_Boolean type_>> scalar."
                                                  GAP "_True Label_ must be an <<OpLabel,*OpLabel*>> in the current function."
                                                  GAP "_False Label_ must be an <<OpLabel,*OpLabel*>> in the current function."
                                                  GAP "Starting with *version 1.6*, _True Label_ and _False Label_ *must not* be the same _<id>_."
                                                  GAP "_Branch weights_ are unsigned 32-bit integer literals. "
                                                      "There must be either no _Branch Weights_ or exactly two branch weights. "
                                                      "If present, the first is the weight for branching to _True Label_, and the second is the weight for branching to _False Label_. "
                                                      "The implied probability that a branch is taken is its weight divided by the sum of the two _Branch weights_. "
                                                      "At least one weight must be non-zero. "
                                                      "A weight of zero does not imply a branch is dead or permit its removal; branch weights are only hints. "
                                                      "The sum of the two weights must not overflow a 32-bit unsigned integer."
                                                  GAP "If _Condition_ is an <<OpUndef, *OpUndef*>>, <<UndefinedBehavior,behavior is undefined>>."
                                                  LAST_INSTRUCTION;

    InstructionDesc[OpSwitch].opDesc = "Multi-way branch to one of the operand label _<id>_."
        GAP "_Selector_ must have a type of <<OpTypeInt,*OpTypeInt*>>. "
            "_Selector_ is compared for equality to the _Target_ literals."
        GAP "_Default_ must be the _<id>_ of a label. If _Selector_ does not equal any of the _Target_ literals, "
            "control flow branches to the _Default_ label _<id>_."
        GAP "_Target_ must be alternating scalar integer _literals_ and the _<id>_ of a label. "
            "If _Selector_ equals a _literal_, control flow branches to the following _label <id>_. "
            "It is invalid for any two _literal_ to be equal to each other. "
            "If _Selector_ does not equal any _literal_, control flow branches to the _Default_ label _<id>_. "
            "Each _literal_ is interpreted with the type of _Selector_: "
            "The bit width of _Selector's_ type is the width of each _literal's_ type. "
            "If this width is not a multiple of 32-bits and the <<OpTypeInt,*OpTypeInt*>> _Signedness_ is set to 1, "
            "the <<Literal, literal>> values are interpreted as being sign extended."
        GAP "If _Selector_ is an <<OpUndef, *OpUndef*>>, <<UndefinedBehavior,behavior is undefined>>."
        LAST_INSTRUCTION;

    InstructionDesc[OpKill].opDesc = DEPRECATED("use <<OpTerminateInvocation,*OpTerminateInvocation*>> or <<OpDemoteToHelperInvocation,*OpDemoteToHelperInvocation*>>")
                                     GAP
                                     "Fragment-shader discard."
                                     GAP "Ceases all further processing in any <<Invocation,invocation>> that executes it: "
                                         "Only instructions these invocations executed before *OpKill* have observable side effects. "
                                         "If this instruction is executed in non-<<UniformControlFlow,uniform control flow>>, all subsequent control flow is non-uniform "
                                         "(for invocations that continue to execute)."
                                     LAST_INSTRUCTION
                                     FRAGMENT_ONLY;

    InstructionDesc[OpReturn].opDesc = "Return with no value from a function with void return type."
                                       LAST_INSTRUCTION;

    InstructionDesc[OpReturnValue].opDesc = "Return a value from a function."
                                            GAP "_Value_ is the value returned, by copy, and must match the _Return Type_ operand of the <<OpTypeFunction,*OpTypeFunction*>> "
                                                "type of the <<OpFunction,*OpFunction*>> body this return instruction is in. _Value_ must not have type <<OpTypeVoid, *OpTypeVoid*>>."
                                            LAST_INSTRUCTION;

    InstructionDesc[OpUnreachable].opDesc = "<<UndefinedBehavior,Behavior is undefined>> if this instruction is executed."
                                            LAST_INSTRUCTION;

    #define LIFE_TIME(da,ss) "Declare that an object " da "this instruction." \
        GAP "_Pointer_ is a pointer to the object whose lifetime is " ss ". Its type must be an <<OpTypePointer,*OpTypePointer*>> with <<Storage_Class, Storage Class>> *Function*." \
        GAP LITERAL_UNSIGNED("_Size_")  \
            "_Size_ must be 0 if _Pointer_ is a pointer to a non-void type or " \
            "the *Addresses* <<Capability, capability>> is not declared. "   \
            "If _Size_ is non-zero, it is the number of bytes of memory whose lifetime is " ss "."

    InstructionDesc[OpLifetimeStart].opDesc = LIFE_TIME("was not defined before ", "starting");

    InstructionDesc[OpLifetimeStop].opDesc = LIFE_TIME("is dead after ", "ending");

    InstructionDesc[OpTerminateInvocation].opDesc = "Fragment-shader terminate."
                                                    GAP
                                                    "Ceases all further processing in any <<Invocation,invocation>> that executes it: Only instructions these invocations executed before *OpTerminateInvocation* " \
                                                    "will have observable side effects. If this instruction is executed in non-<<UniformControlFlow,uniform control flow>>, all subsequent control flow is non-uniform " \
                                                    "(for invocations that continue to execute)."
                                                    LAST_INSTRUCTION
                                                    FRAGMENT_ONLY;

    InstructionDesc[OpDemoteToHelperInvocation].opDesc = "Demote this fragment shader <<Invocation,invocation>> to a helper invocation. " \
                                                         "Any stores to memory after this instruction are suppressed and the fragment does not write outputs to the framebuffer."
                                                         GAP
                                                         "Unlike the <<OpTerminateInvocation,*OpTerminateInvocation*>> instruction, this does not necessarily terminate the invocation " \
                                                         "which might be needed for derivative calculations. It is not considered a flow control instruction (flow control does not " \
                                                         "become non-uniform) and does not terminate the block. The implementation may terminate helper invocations before the end of " \
                                                         "the shader as an optimization, but doing so must not affect derivative calculations and does not make control flow non-uniform."
                                                         GAP
                                                         "After an invocation executes this instruction, any subsequent load of *HelperInvocation* within that invocation will load " \
                                                         "<<Poison,poison>> unless the *HelperInvocation* <<BuiltIn,built-in variable>> is <<Decoration,decorated>> with *Volatile* or the load included " \
                                                         "*Volatile* in its <<Memory_Operands,*Memory Operands*>>"
                                                         FRAGMENT_ONLY;


    #define UNIFORM_CONTROL "<<UndefinedBehavior,Behavior is undefined>> unless all invocations within _Execution_ execute the same " \
                            "dynamic instance of this instruction."
    #define NU_SCOPE WS_SCOPE "It must be *Subgroup*. "
    InstructionDesc[OpGroupAsyncCopy].opDesc = "Perform an asynchronous group copy of _Num Elements_ elements from _Source_ to _Destination_. "
        "The asynchronous copy is performed by all invocations in the <<ScopeRestrictedTangle, scope restricted tangle>>."
        GAP "This instruction results in an event object that can be used by <<OpGroupWaitEvents, *OpGroupWaitEvents*>> to wait for the async copy to finish."
        GAP WS_SCOPE
        GAP UNIFORM_CONTROL
        GAP TANGLED_EXEC_WAIT
        GAP "_Result Type_ must be an <<OpTypeEvent, *OpTypeEvent*>> object."
        GAP "_Destination_ " PSV_FP_I
        GAP "_Destination_ pointer <<Storage_Class, Storage Class>> must be *Workgroup* or *CrossWorkgroup*."
        GAP MATCHING_OPERANDS("_Source_", "_Destination_")
        GAP "If _Destination_ pointer <<Storage_Class, Storage Class>> is *Workgroup*, the _Source_ pointer Storage Class must be *CrossWorkgroup*. "
        "In this case _Stride_ defines the stride in elements when reading from _Source_ pointer."
        GAP "If _Destination_ pointer <<Storage_Class, Storage Class>> is *CrossWorkgroup*, the _Source_ pointer Storage Class must be *Workgroup*. "
        "In this case _Stride_ defines the stride in elements when writing each element to _Destination_ pointer."
        GAP "_Stride_ and _NumElements_ must be a 32-bit <<Integer,_integer type_>> scalar if the <<Addressing_Model, addressing model>> is _Physical32_ and 64 bit <<Integer,_integer type_>> scalar if the _Addressing Model_ is _Physical64_."
        GAP "_Event_ must have a type of <<OpTypeEvent, *OpTypeEvent*>>."
        GAP "_Event_ can be used to associate the copy with a previous copy allowing an event to be shared by multiple copies. Otherwise _Event_ should be an <<OpConstantNull, *OpConstantNull*>>."
        GAP "If _Event_ is not <<OpConstantNull, *OpConstantNull*>>, "
            "the result is the event object supplied by the _Event_ operand.";

    #define OPERATION_IDENTITY(I) GAP "The identity _I_ for _Operation_ is " I ". "

    InstructionDesc[OpGroupWaitEvents].opDesc = "Wait for events generated by <<OpGroupAsyncCopy, *OpGroupAsyncCopy*>> operations to complete. "
        "_Events List_ points to _Num Events_ event objects, which is released after the wait is performed."
        GAP WS_SCOPE
        GAP UNIFORM_CONTROL
        GAP TANGLED_EXEC_WAIT
        GAP "_Num Events_ must be a 32-bit <<Integer,_integer type_>> scalar."
        GAP "_Events List_ must be a pointer to <<OpTypeEvent, *OpTypeEvent*>>.";

    InstructionDesc[OpGroupAll].opDesc = "Evaluates a predicate for all invocations in the <<ScopeRestrictedTangle, scope restricted tangle>>, "
        "resulting in *true* if predicate evaluates to *true* for all <<Invocation,invocations>> in the scope restricted tangle, otherwise the result is *false*."
        GAP WS_SCOPE
        GAP UNIFORM_CONTROL
        GAP TANGLED_EXEC_WAIT
        GAP "_Result Type_ must be a <<Boolean, _Boolean type_>>."
        GAP "_Predicate_ must be a <<Boolean, _Boolean type_>>.";

    InstructionDesc[OpGroupAny].opDesc = "Evaluates a predicate for all invocations in the <<ScopeRestrictedTangle, scope restricted tangle>>,"
        "resulting in *true* if predicate evaluates to *true* for any <<Invocation,invocation>> in the scope restricted tangle, otherwise the result is *false*."
        GAP WS_SCOPE
        GAP UNIFORM_CONTROL
        GAP TANGLED_EXEC_WAIT
        GAP "_Result Type_ must be a <<Boolean, _Boolean type_>>."
        GAP "_Predicate_ must be a <<Boolean, _Boolean type_>>.";

    InstructionDesc[OpGroupBroadcast].opDesc = "Broadcast the _Value_ of the <<Invocation,invocation>> identified by the local id _LocalId_ to the result of all invocations in the <<ScopeRestrictedTangle, scope restricted tangle>>."
        GAP WS_SCOPE
        GAP UNIFORM_CONTROL
        GAP TANGLED_EXEC_WAIT
        GAP "_Result Type_ " SV_FP_I_B
        GAP MATCHING("_Value_")
        GAP "_LocalId_ must be an integer datatype. It must be a scalar, a vector with 2 components, "
            "or a vector with 3 components. "
            "<<UndefinedBehavior,Behavior is undefined>> unless _LocalId_ is the same for all <<Invocation,invocations>> in the group, or if it is greater than or equal to the size of the group in any dimension.";

    InstructionDesc[OpGroupIAdd].opDesc = "An integer add group operation specified for all values of _X_ specified by <<Invocation,invocations>> in the <<ScopeRestrictedTangle, scope restricted tangle>>."
        GAP WS_SCOPE
        GAP UNIFORM_CONTROL
        GAP TANGLED_EXEC_WAIT
        GAP "_Result Type_ " SV_I
        OPERATION_IDENTITY("0")
        GAP MATCHING("_X_");

    InstructionDesc[OpGroupFAdd].opDesc = "A floating-point add group operation specified for all values of _X_ specified by <<Invocation,invocations>> in the <<ScopeRestrictedTangle, scope restricted tangle>>."
        GAP WS_SCOPE
        GAP UNIFORM_CONTROL
        GAP TANGLED_EXEC_WAIT
        GAP "_Result Type_ " SV_FP
        OPERATION_IDENTITY("0")
        GAP MATCHING("_X_");

    InstructionDesc[OpGroupUMin].opDesc = "An unsigned integer minimum group operation specified for all values of _X_ specified by <<Invocation,invocations>> in the <<ScopeRestrictedTangle, scope restricted tangle>>."
        GAP WS_SCOPE
        GAP UNIFORM_CONTROL
        GAP TANGLED_EXEC_WAIT
        GAP "_Result Type_ " SV_I
        OPERATION_IDENTITY("UINT_MAX when _X_ is 32 bits wide and ULONG_MAX when _X_ is 64 bits wide")
        GAP MATCHING("_X_");

    InstructionDesc[OpGroupSMin].opDesc = "A signed integer minimum group operation specified for all values of _X_ specified by <<Invocation,invocations>> in the <<ScopeRestrictedTangle, scope restricted tangle>>."
        GAP WS_SCOPE
        GAP UNIFORM_CONTROL
        GAP TANGLED_EXEC_WAIT
        GAP "_Result Type_ " SV_I
        OPERATION_IDENTITY("INT_MAX when _X_ is 32 bits wide and LONG_MAX when _X_ is 64 bits wide")
        GAP MATCHING("_X_");

    InstructionDesc[OpGroupFMin].opDesc = "A floating-point minimum group operation specified for all values of _X_ specified by <<Invocation,invocations>> in the <<ScopeRestrictedTangle, scope restricted tangle>>."
        GAP WS_SCOPE
        GAP UNIFORM_CONTROL
        GAP TANGLED_EXEC_WAIT
        GAP "_Result Type_ " SV_FP
        OPERATION_IDENTITY("+INF")
        GAP MATCHING("_X_");

    InstructionDesc[OpGroupUMax].opDesc = "An unsigned integer maximum group operation specified for all values of _X_ specified by <<Invocation,invocations>> in the <<ScopeRestrictedTangle, scope restricted tangle>>."
        GAP WS_SCOPE
        GAP UNIFORM_CONTROL
        GAP TANGLED_EXEC_WAIT
        GAP "_Result Type_ " SV_I
        OPERATION_IDENTITY("0")
        GAP MATCHING("_X_");

    InstructionDesc[OpGroupSMax].opDesc = "A signed integer maximum group operation specified for all values of _X_ specified by <<Invocation,invocations>> in the <<ScopeRestrictedTangle, scope restricted tangle>>."
        GAP WS_SCOPE
        GAP UNIFORM_CONTROL
        GAP TANGLED_EXEC_WAIT
        GAP "_Result Type_ " SV_I
        OPERATION_IDENTITY("INT_MIN when _X_ is 32 bits wide and LONG_MIN when _X_ is 64 bits wide")
        GAP MATCHING("_X_");

    InstructionDesc[OpGroupFMax].opDesc = "A floating-point maximum group operation specified for all values of _X_ specified by <<Invocation,invocations>> in the <<ScopeRestrictedTangle, scope restricted tangle>>."
        GAP WS_SCOPE
        GAP UNIFORM_CONTROL
        GAP TANGLED_EXEC_WAIT
        GAP "_Result Type_ " SV_FP
        OPERATION_IDENTITY("-INF")
        GAP MATCHING("_X_");

#define PACKET_SIZE_ID \
        GAP "_Packet Size_ must be a 32-bit <<Integer, _integer type_>> scalar that "      \
            "represents the size in bytes of each packet in the pipe."                   \
        GAP "_Packet Alignment_ must be a 32-bit <<Integer, _integer type_>> scalar that " \
            "represents the alignment in bytes of each packet in the pipe."

#define PACKET_SIZE_LITERAL \
        GAP LITERAL_UNSIGNED("_Packet Size_")                                    \
            "It represents the size in bytes of each packet in the pipe."       \
        GAP LITERAL_UNSIGNED("_Packet Alignment_")                               \
            "It represents the alignment in bytes of each packet in the pipe."

#define PIPE_PACKET_ALIGNMENT GAP "<<UndefinedBehavior,Behavior is undefined>> unless _Packet Alignment_ > 0 and evenly divides _Packet Size_."

#define PIPE_PACKET_SIZE_ALIGNMENT \
        PACKET_SIZE_ID             \
        PIPE_PACKET_ALIGNMENT

#define PIPE_PACKET_SIZE_ALIGNMENT_LITERAL \
        PACKET_SIZE_LITERAL                \
        PIPE_PACKET_ALIGNMENT

    InstructionDesc[OpReadPipe].opDesc = "Read a packet from the pipe object specified by _Pipe_ into _Pointer_. "
        "Result is 0 if the operation is successful and a negative value if the pipe is empty."
        GAP "_Result Type_ must be a 32-bit <<Integer, _integer type_>> scalar."
        GAP "_Pipe_ must have a type of <<OpTypePipe, *OpTypePipe*>> with *ReadOnly* <<Access_Qualifier, _access qualifier_>>."
        GAP "_Pointer_ must have a type of <<OpTypePointer, *OpTypePointer*>> with <<Storage_Class,Storage Class>> *Generic*."
        PIPE_PACKET_SIZE_ALIGNMENT;

    InstructionDesc[OpWritePipe].opDesc = "Write a packet from _Pointer_ to the pipe object specified by _Pipe_. "
        "Result is 0 if the operation is successful and a negative value if the pipe is full."
        GAP "_Result Type_ must be a 32-bit <<Integer, _integer type_>> scalar."
        GAP "_Pipe_ must have a type of <<OpTypePipe, *OpTypePipe*>> with *WriteOnly* <<Access_Qualifier, _access qualifier_>>."
        GAP "_Pointer_ must have a type of <<OpTypePointer, *OpTypePointer*>> with <<Storage_Class,Storage Class>> *Generic*."
        PIPE_PACKET_SIZE_ALIGNMENT;

    InstructionDesc[OpReservedReadPipe].opDesc = "Read a packet from the reserved area specified by _Reserve Id_ and _Index_ of the pipe object specified by _Pipe_ into _Pointer_. "
        "The reserved pipe entries are referred to by indices that go from 0 ... _Num Packets_ - 1. "
        "Result is 0 if the operation is successful and a negative value otherwise."
        GAP "_Result Type_ must be a 32-bit <<Integer, _integer type_>> scalar."
        GAP "_Pipe_ must have a type of <<OpTypePipe, *OpTypePipe*>> with *ReadOnly* <<Access_Qualifier, _access qualifier_>>."
        GAP "_Reserve Id_ must have a type of <<OpTypeReserveId, *OpTypeReserveId*>>."
        GAP "_Index_ must be a 32-bit <<Integer, _integer type_>> scalar, which is treated as an unsigned value."
        GAP "_Pointer_ must have a type of <<OpTypePointer, *OpTypePointer*>> with <<Storage_Class,Storage Class>> *Generic*."
        PIPE_PACKET_SIZE_ALIGNMENT;

    InstructionDesc[OpReservedWritePipe].opDesc = "Write a packet from _Pointer_ into the reserved area specified by _Reserve Id_ and _Index_ of the pipe object specified by _Pipe_. "
        "The reserved pipe entries are referred to by indices that go from 0 ... _Num Packets_ - 1. "
        "Result is 0 if the operation is successful and a negative value otherwise."
        GAP "_Result Type_ must be a 32-bit <<Integer, _integer type_>> scalar."
        GAP "_Pipe_ must have a type of <<OpTypePipe, *OpTypePipe*>> with *WriteOnly* <<Access_Qualifier, _access qualifier_>>."
        GAP "_Reserve Id_ must have a type of <<OpTypeReserveId, *OpTypeReserveId*>>."
        GAP "_Index_ must be a 32-bit <<Integer, _integer type_>> scalar, which is treated as an unsigned value."
        GAP "_Pointer_ must have a type of <<OpTypePointer, *OpTypePointer*>> with <<Storage_Class,Storage Class>> *Generic*."
        PIPE_PACKET_SIZE_ALIGNMENT;

    InstructionDesc[OpReserveReadPipePackets].opDesc = "Reserve _Num Packets_ entries for reading from the pipe object specified by _Pipe_. "
        "Result is a valid reservation ID if the reservation is successful."
        GAP "_Result Type_ must be an <<OpTypeReserveId, *OpTypeReserveId*>>."
        GAP "_Pipe_ must have a type of <<OpTypePipe, *OpTypePipe*>> with *ReadOnly* <<Access_Qualifier, _access qualifier_>>."
        GAP "_Num Packets_ must be a 32-bit <<Integer, _integer type_>> scalar, which is treated as an unsigned value. "
        PIPE_PACKET_SIZE_ALIGNMENT;

    InstructionDesc[OpReserveWritePipePackets].opDesc = "Reserve _num_packets_ entries for writing to the pipe object specified by _Pipe_. "
        "Result is a valid reservation ID if the reservation is successful."
        GAP "_Pipe_ must have a type of <<OpTypePipe, *OpTypePipe*>> with *WriteOnly* <<Access_Qualifier, _access qualifier_>>."
        GAP "_Num Packets_ must be a 32-bit <<OpTypeInt, *OpTypeInt*>> which is treated as an unsigned value."
        GAP "_Result Type_ must be an <<OpTypeReserveId, *OpTypeReserveId*>>. "
        PIPE_PACKET_SIZE_ALIGNMENT;

    InstructionDesc[OpCommitReadPipe].opDesc = "Indicates that all reads to _Num Packets_ associated with the reservation specified by "
        "_Reserve Id_ and the pipe object specified by _Pipe_ are completed."
        GAP "_Pipe_ must have a type of <<OpTypePipe, *OpTypePipe*>> with *ReadOnly* <<Access_Qualifier, _access qualifier_>>."
        GAP "_Reserve Id_ must have a type of <<OpTypeReserveId, *OpTypeReserveId*>>. "
        PIPE_PACKET_SIZE_ALIGNMENT;

    InstructionDesc[OpCommitWritePipe].opDesc = "Indicates that all writes to _Num Packets_ associated with the reservation specified by "
        "_Reserve Id_ and the pipe object specified by _Pipe_ are completed."
        GAP "_Pipe_ must have a type of <<OpTypePipe, *OpTypePipe*>> with *WriteOnly* <<Access_Qualifier, _access qualifier_>>."
        GAP "_Reserve Id_ must have a type of <<OpTypeReserveId, *OpTypeReserveId*>>. "
        PIPE_PACKET_SIZE_ALIGNMENT;

    InstructionDesc[OpIsValidReserveId].opDesc = "Result is *true* if _Reserve Id_ is a valid reservation id and *false* otherwise."
        GAP "_Result Type_ must be a <<Boolean, _Boolean type_>>."
        GAP "_Reserve Id_ must have a type of <<OpTypeReserveId, *OpTypeReserveId*>>.";

    InstructionDesc[OpGetNumPipePackets].opDesc = "Result is the number of available entries in the pipe object specified by _Pipe_. "
        "The number of available entries in a pipe is a dynamic value. The result is considered immediately stale."
        GAP "_Result Type_ must be a 32-bit <<Integer, _integer type_>> scalar, which should be treated as an unsigned value."
        GAP "_Pipe_ must have a type of <<OpTypePipe, *OpTypePipe*>> with *ReadOnly* or *WriteOnly* <<Access_Qualifier, _access qualifier_>>."
        PIPE_PACKET_SIZE_ALIGNMENT;

    InstructionDesc[OpGetMaxPipePackets].opDesc = "Result is the maximum number of packets specified by the creation of _Pipe_."
        GAP "_Result Type_ must be a 32-bit <<Integer, _integer type_>> scalar, which should be treated as an unsigned value."
        GAP "_Pipe_ must have a type of <<OpTypePipe, *OpTypePipe*>> with *ReadOnly* or *WriteOnly* <<Access_Qualifier, _access qualifier_>>. "
        PIPE_PACKET_SIZE_ALIGNMENT;

    InstructionDesc[OpGroupReserveReadPipePackets].opDesc = "Reserve _Num Packets_ entries for the <<ScopeRestrictedTangle, scope restricted tangle>> for reading from the pipe object specified by _Pipe_. "
        "Result is a valid reservation id if the reservation is successful."
        GAP "The reserved pipe entries are referred to by indices that go from 0 ... _Num Packets_ - 1."
        GAP WS_SCOPE
        GAP UNIFORM_CONTROL
        GAP TANGLED_EXEC_WAIT
        GAP "_Result Type_ must be an <<OpTypeReserveId, *OpTypeReserveId*>>."
        GAP "_Pipe_ must have a type of <<OpTypePipe, *OpTypePipe*>> with *ReadOnly* <<Access_Qualifier, _access qualifier_>>."
        GAP "_Num Packets_ must be a 32-bit <<Integer, _integer type_>> scalar, which is treated as an unsigned value. "
        PIPE_PACKET_SIZE_ALIGNMENT;

    InstructionDesc[OpGroupReserveWritePipePackets].opDesc = "Reserve _Num Packets_ entries for the <<ScopeRestrictedTangle, scope restricted tangle>> for writing to the pipe object specified by _Pipe_. "
        "Result is a valid reservation id if the reservation is successful."
        GAP "The reserved pipe entries are referred to by indices that go from 0 ... _Num Packets_ - 1."
        GAP WS_SCOPE
        GAP UNIFORM_CONTROL
        GAP TANGLED_EXEC_WAIT
        GAP "_Result Type_ must be an <<OpTypeReserveId, *OpTypeReserveId*>>."
        GAP "_Pipe_ must have a type of <<OpTypePipe, *OpTypePipe*>> with *WriteOnly* <<Access_Qualifier, _access qualifier_>>."
        GAP "_Num Packets_ must be a 32-bit <<Integer, _integer type_>> scalar, which is treated as an unsigned value. "
        PIPE_PACKET_SIZE_ALIGNMENT;

    InstructionDesc[OpGroupCommitReadPipe].opDesc = "Indicates that all reads to _Num Packets_ associated with the reservation specified by "
        "_Reserve Id_ and the pipe object specified by _Pipe_ were completed by the <<ScopeRestrictedTangle, scope restricted tangle>>."
        GAP WS_SCOPE
        GAP UNIFORM_CONTROL
        GAP TANGLED_EXEC_WAIT
        GAP "_Pipe_ must have a type of <<OpTypePipe, *OpTypePipe*>> with *ReadOnly* <<Access_Qualifier, _access qualifier_>>."
        GAP "_Reserve Id_ must have a type of <<OpTypeReserveId, *OpTypeReserveId*>>. "
        PIPE_PACKET_SIZE_ALIGNMENT;

    InstructionDesc[OpGroupCommitWritePipe].opDesc = "Indicates that all writes to _Num Packets_ associated with the reservation specified by "
        "_Reserve Id_ and the pipe object specified by _Pipe_ were completed by the <<ScopeRestrictedTangle, scope restricted tangle>>."
        GAP WS_SCOPE
        GAP UNIFORM_CONTROL
        GAP TANGLED_EXEC_WAIT
        GAP "_Pipe_ must have a type of <<OpTypePipe, *OpTypePipe*>> with *WriteOnly* <<Access_Qualifier, _access qualifier_>>."
        GAP "_Reserve Id_ must have a type of <<OpTypeReserveId, *OpTypeReserveId*>>. "
        PIPE_PACKET_SIZE_ALIGNMENT;

    InstructionDesc[OpConstantPipeStorage].opDesc = "Creates a pipe-storage object."
        GAP "_Result Type_ must be <<OpTypePipeStorage, *OpTypePipeStorage*>>. "
        PIPE_PACKET_SIZE_ALIGNMENT_LITERAL
        GAP LITERAL_UNSIGNED("_Capacity_")
            "It is the minimum number of _Packet Size_ blocks the resulting <<OpTypePipeStorage, *OpTypePipeStorage*>> can hold.";

    InstructionDesc[OpCreatePipeFromPipeStorage].opDesc = "Creates a pipe object from a pipe-storage object."
        GAP "_Result Type_ must be <<OpTypePipe, *OpTypePipe*>>."
        GAP "_Pipe Storage_ must be a pipe-storage object created from <<OpConstantPipeStorage, *OpConstantPipeStorage*>>.";

#define NDRANGE_TYPE "<<OpTypeStruct, *OpTypeStruct*>> with the following ordered list of members, starting from the first to last:" \
        GAP \
        "1. A 32-bit <<Integer, _integer type_>> scalar that specifies the number of dimensions in the global size and the workgroup size.\n" \
        "2. An <<OpTypeArray, *OpTypeArray*>> with 3 elements, where each element is a 32-bit <<Integer, _integer type_>> scalar\n" \
        "   if the <<Addressing_Model,_addressing model_>> is *Physical32* or a 64-bit <<Integer, _integer type_>> scalar if the <<Addressing_Model,_addressing model_>> is *Physical64*.\n" \
        "   This is an array of per-dimension unsigned values that specifies the global offset used to calculate the global ID for an invocation.\n" \
        "3. An <<OpTypeArray, *OpTypeArray*>> with 3 elements, where each element is a 32-bit <<Integer, _integer type_>> scalar\n" \
        "   if the <<Addressing_Model,_addressing model_>> is *Physical32* or a 64-bit <<Integer, _integer type_>> scalar if the <<Addressing_Model,_addressing model_>> is *Physical64*.\n" \
        "   This is an array of per-dimension unsigned values that specifies the number of global invocations that execute the kernel function.\n" \
        "4. An <<OpTypeArray, *OpTypeArray*>> with 3 elements, where each element is a 32-bit <<Integer, _integer type_>> scalar\n" \
        "   if the <<Addressing_Model,_addressing model_>> is *Physical32* or a 64-bit <<Integer, _integer type_>> scalar if the <<Addressing_Model,_addressing model_>> is *Physical64*.\n" \
        "   This is an array of per-dimension unsigned values that specifies the number of invocations in a workgroup.\n"

    #define ENQUEUE_FUNC_TYPE "<<OpFunction, *OpFunction*>> whose <<OpTypeFunction, *OpTypeFunction*>> operand has:" \
        GAP \
        " - _Result Type_ must be <<OpTypeVoid, *OpTypeVoid*>>.\n" \
        " - The first parameter must have a type of <<OpTypePointer, *OpTypePointer*>> to an 8-bit <<OpTypeInt, *OpTypeInt*>>.\n" \
        " - An optional list of parameters, each of which must have a type of <<OpTypePointer, *OpTypePointer*>> to the *Workgroup* <<Storage_Class,Storage Class>>.\n"

    InstructionDesc[OpBuildNDRange].opDesc = "Given the global work size specified by _GlobalWorkSize_, local work size specified by _LocalWorkSize_ "
        "and global work offset specified by _GlobalWorkOffset_, builds the result as a 1D, 2D, or 3D ND-range descriptor structure."
        GAP "_Result Type_ must be an " NDRANGE_TYPE
        GAP "_GlobalWorkSize_ must be a scalar or an array with 2 or 3 components. "
        "Where the type of each element in the array is 32-bit <<Integer, _integer type_>> scalar if the <<Addressing_Model,_addressing model_>> is *Physical32* "
        "or 64-bit <<Integer, _integer type_>> scalar if the <<Addressing_Model,_addressing model_>> is *Physical64*."
        GAP "The type of _LocalWorkSize_ must be the same as _GlobalWorkSize_."
        GAP "The type of _GlobalWorkOffset_ must be the same as _GlobalWorkSize_.";

    InstructionDesc[OpGetDefaultQueue].opDesc = "The result is the default device queue, or "
        "if a default device queue has not been created, a null queue object." GAP
        "_Result Type_ must be an <<OpTypeQueue, *OpTypeQueue*>>.";

    InstructionDesc[OpCaptureEventProfilingInfo].opDesc = "Captures the profiling information specified by _Profiling Info_ for "
        "the command associated with the event specified by _Event_ in the memory pointed to by _Value_. "
        "The profiling information is available in the memory pointed to by _Value_ after the command identified by _Event_ has completed."
        GAP "_Event_ must have a type of <<OpTypeDeviceEvent, *OpTypeDeviceEvent*>> that was produced by <<OpEnqueueKernel, *OpEnqueueKernel*>> "
        "or <<OpEnqueueMarker,*OpEnqueueMarker*>>. "
        GAP "_Profiling Info_ " S_I " The content of _Profiling Info_ is interpreted as <<Kernel_Profiling_Info, _Kernel Profiling Info_>> mask."
        GAP "_Value_ must be a pointer to a scalar 8-bit <<Integer,_integer type_>> in the *CrossWorkgroup* <<Storage_Class,Storage Class>>."
        GAP "If _Profiling Info_ is *CmdExecTime*, _Value_ <<UndefinedBehavior,behavior is undefined>> if it does not point to a 128-bit memory range."
        LINE_BREAK "The first 64 bits contain the elapsed time CL_PROFILING_COMMAND_END - CL_PROFILING_COMMAND_START for the command identified by _Event_ in nanoseconds."
        LINE_BREAK "The second 64 bits contain the elapsed time CL_PROFILING_COMMAND_COMPLETE - CL_PROFILING_COMMAND_START for the command identified by _Event_ in nanoseconds."
        NOTE "What is captured is <<Poison,_poison_>> if this instruction is called multiple times for the same event.";

    InstructionDesc[OpSetUserEventStatus].opDesc = "Sets the execution status of a user event specified by _Event_."
        "_Status_ can be either 0 (CL_COMPLETE) to indicate that this kernel and all its child kernels finished execution successfully, "
        "or a negative integer value indicating an error."
        GAP "_Event_ must have a type of <<OpTypeDeviceEvent, *OpTypeDeviceEvent*>> that was produced by <<OpCreateUserEvent, *OpCreateUserEvent*>>."
        GAP "_Status_ must have a type of 32-bit <<OpTypeInt, *OpTypeInt*>> treated as a signed integer.";

    InstructionDesc[OpIsValidEvent].opDesc =
        "Result is *true* if the event specified by _Event_ is a valid event, otherwise *false*."
        GAP "_Result Type_ must be a <<Boolean, _Boolean type_>>."
        GAP "_Event_ must have a type of <<OpTypeDeviceEvent, *OpTypeDeviceEvent*>>";

    InstructionDesc[OpCreateUserEvent].opDesc = "Create a user event. The execution status of the created event is set to a value of 2 (CL_SUBMITTED)."
        GAP "_Result Type_ must be <<OpTypeDeviceEvent, *OpTypeDeviceEvent*>>.";

    InstructionDesc[OpRetainEvent].opDesc = "Increments the reference count of the event object specified by _Event_."
        GAP "<<UndefinedBehavior,Behavior is undefined>> if _Event_ is not a valid event.";

    InstructionDesc[OpReleaseEvent].opDesc = "Decrements the reference count of the event object specified by _Event_. "
        "The event object is deleted once the event reference count is zero, "
        "the specific command identified by this event has completed (or terminated) "
        "and there are no commands in any device command queue that require a wait for this event to complete."
        GAP "<<UndefinedBehavior,Behavior is undefined>> if _Event_ is not a valid event.";

    InstructionDesc[OpGetKernelWorkGroupSize].opDesc =
        "Result is the maximum workgroup size that can be used to execute the function "
        "specified by _Invoke_ on the device."
        GAP "_Result Type_ must be a 32-bit <<Integer, _integer type_>> scalar."
        GAP "_Invoke_ must be an " ENQUEUE_FUNC_TYPE
        GAP "_Param_ is the first parameter of the function specified by _Invoke_ and must be "
        "a pointer to an 8-bit <<Integer,_integer type_>> scalar."
        GAP "_Param Size_ is the size in bytes of the memory pointed to by _Param_ and must be a 32-bit <<Integer,_integer type_>> scalar, which is treated as an unsigned integer."
        GAP "_Param Align_ is the alignment of _Param_ and must be a 32-bit <<Integer,_integer type_>> scalar, which is treated as an unsigned integer.";

    InstructionDesc[OpGetKernelPreferredWorkGroupSizeMultiple].opDesc =
        "Result is the preferred multiple of workgroup size for the function specified by _Invoke_. "
        "This is a performance hint. Specifying a workgroup size that is not a multiple of this result "
        "as the value of the local work size does not fail to enqueue _Invoke_ "
        "for execution unless the workgroup size specified is larger than the device maximum."
        GAP "_Result Type_ must be a 32-bit <<Integer, _integer type_>> scalar."
        GAP "_Invoke_ must be an " ENQUEUE_FUNC_TYPE
        GAP "_Param_ is the first parameter of the function specified by _Invoke_ and must be "
        "a pointer to an 8-bit <<Integer,_integer type_>> scalar."
        GAP "_Param Size_ is the size in bytes of the memory pointed to by _Param_ and must be a 32-bit <<Integer,_integer type_>> scalar, which is treated as an unsigned integer."
        GAP "_Param Align_ is the alignment of _Param_ and must be a 32-bit <<Integer,_integer type_>> scalar, which is treated as an unsigned integer.";

    InstructionDesc[OpGetKernelNDrangeSubGroupCount].opDesc =
        "Result is the number of subgroups in each workgroup of the dispatch "
        "(except for the last in cases where the global size does not divide cleanly into workgroups) "
        "given the combination of the passed NDRange descriptor specified by _ND Range_ and the function specified by _Invoke_."
        GAP "_Result Type_ must be a 32-bit <<Integer,_integer type_>> scalar."
        GAP "The type of _ND Range_ must be an <<OpTypeStruct, *OpTypeStruct*>> whose members are as described by the _Result Type_ of <<OpBuildNDRange, *OpBuildNDRange*>>."
        GAP "_Invoke_ must be an " ENQUEUE_FUNC_TYPE
        GAP "_Param_ is the first parameter of the function specified by _Invoke_ and must be "
        "a pointer to an 8-bit <<Integer,_integer type_>> scalar."
        GAP "_Param Size_ is the size in bytes of the memory pointed to by _Param_ and must be a 32-bit <<Integer,_integer type_>> scalar, which is treated as an unsigned integer."
        GAP "_Param Align_ is the alignment of _Param_ and must be a 32-bit <<Integer,_integer type_>> scalar, which is treated as an unsigned integer.";

    InstructionDesc[OpGetKernelNDrangeMaxSubGroupSize].opDesc = "Result is the maximum subgroup size for "
        "the function specified by _Invoke_ and the NDRange specified by _ND Range_. "
        GAP "_Result Type_ must be a 32-bit <<Integer,_integer type_>> scalar."
        GAP "The type of _ND Range_ must be an <<OpTypeStruct, *OpTypeStruct*>> whose members are as described by the _Result Type_ of <<OpBuildNDRange, *OpBuildNDRange*>>."
        GAP "_Invoke_ must be an " ENQUEUE_FUNC_TYPE
        GAP "_Param_ is the first parameter of the function specified by _Invoke_ and must be "
        "a pointer to an 8-bit <<Integer,_integer type_>> scalar."
        GAP "_Param Size_ is the size in bytes of the memory pointed to by _Param_ and must be a 32-bit <<Integer,_integer type_>> scalar, which is treated as an unsigned integer."
        GAP "_Param Align_ is the alignment of _Param_ and must be a 32-bit <<Integer,_integer type_>> scalar, which is treated as an unsigned integer.";

    InstructionDesc[OpEnqueueKernel].opDesc = "Enqueue the function specified by _Invoke_ and the NDRange specified by _ND Range_ "
        "for execution to the queue object specified by _Queue_. "
        GAP "_Result Type_ must be a 32-bit <<Integer,_integer type_>> scalar."
            " A successful enqueue results in the value 0. A failed enqueue results in a non-0 value."
        GAP "_Queue_ must be of the type <<OpTypeQueue, *OpTypeQueue*>>."
        GAP "_Flags_ " S_I " The content of _Flags_ is interpreted as <<Kernel_Enqueue_Flags, _Kernel Enqueue Flags_>> mask."
        GAP "The type of _ND Range_ must be an <<OpTypeStruct, *OpTypeStruct*>> whose members are as described by the _Result Type_ of <<OpBuildNDRange, *OpBuildNDRange*>>."
        GAP "_Num Events_ specifies the number of event objects in the wait list pointed to by _Wait Events_ and "
        "must be 32-bit <<Integer,_integer type_>> scalar, which is treated as an unsigned integer."
        GAP "_Wait Events_ specifies the list of wait event objects and "
        "must be a pointer to <<OpTypeDeviceEvent, *OpTypeDeviceEvent*>>."
        GAP "_Ret Event_ must be a pointer to <<OpTypeDeviceEvent, *OpTypeDeviceEvent*>> which gets implicitly retained by this instruction. "
        GAP "_Invoke_ must be an " ENQUEUE_FUNC_TYPE
        GAP "_Param_ is the first parameter of the function specified by _Invoke_ and must be "
        "a pointer to an 8-bit <<Integer,_integer type_>> scalar."
        GAP "_Param Size_ is the size in bytes of the memory pointed to by _Param_ and must be a 32-bit <<Integer,_integer type_>> scalar, which is treated as an unsigned integer."
        GAP "_Param Align_ is the alignment of _Param_ and must be a 32-bit <<Integer,_integer type_>> scalar, which is treated as an unsigned integer."
        GAP "Each _Local Size_ operand corresponds (in order) to one <<OpTypePointer,*OpTypePointer*>> to *Workgroup* <<Storage_Class,Storage Class>> parameter to the _Invoke_ function, "
            "and specifies the number of bytes of *Workgroup* storage used to back the pointer during the execution of the _Invoke_ function.";

    InstructionDesc[OpEnqueueMarker].opDesc = "Enqueue a marker command to the queue object specified by _Queue_. "
        "The marker command waits for a list of events to complete, or if the list is empty it waits for all previously enqueued "
        "commands in _Queue_ to complete before the marker completes."
        GAP "_Result Type_ must be a 32-bit <<Integer,_integer type_>> scalar. "
            "A successful enqueue results in the value 0. A failed enqueue results in a non-0 value."
        GAP "_Queue_ must be of the type <<OpTypeQueue, *OpTypeQueue*>>."
        GAP "_Num Events_ specifies the number of event objects in the wait list pointed to by _Wait Events_ and "
        "must be a 32-bit <<Integer,_integer type_>> scalar, which is treated as an unsigned integer."
        GAP "_Wait Events_ specifies the list of wait event objects and "
        "must be a pointer to <<OpTypeDeviceEvent, *OpTypeDeviceEvent*>>."
        GAP "_Ret Event_ is a pointer to a device event which gets implicitly retained by this instruction. "
        "It must have a type of <<OpTypePointer, *OpTypePointer*>> to <<OpTypeDeviceEvent, *OpTypeDeviceEvent*>>. If _Ret Event_ is set to null this instruction becomes a no-op.";

    InstructionDesc[OpGetKernelLocalSizeForSubgroupCount].opDesc =
        "Result is the 1D local size to enqueue _Invoke_ with _Subgroup Count_ subgroups per workgroup."
        GAP "_Result Type_ must be a 32-bit <<Integer,_integer type_>> scalar."
        GAP "_Subgroup Count_ must be a 32-bit <<Integer,_integer type_>> scalar."
        GAP "_Invoke_ must be an " ENQUEUE_FUNC_TYPE
        GAP "_Param_ is the first parameter of the function specified by _Invoke_ and must be "
        "a pointer to an 8-bit <<Integer,_integer type_>> scalar."
        GAP "_Param Size_ is the size in bytes of the memory pointed to by _Param_ and must be a 32-bit <<Integer,_integer type_>> scalar, which is treated as an unsigned integer."
        GAP "_Param Align_ is the alignment of _Param_ and must be a 32-bit <<Integer,_integer type_>> scalar, which is treated as an unsigned integer.";

    InstructionDesc[OpGetKernelMaxNumSubgroups].opDesc =
        "Result is the maximum number of subgroups that can be used to execute _Invoke_ on the device."
        GAP "_Result Type_ must be a 32-bit <<Integer,_integer type_>> scalar."
        GAP "_Invoke_ must be an " ENQUEUE_FUNC_TYPE
        GAP "_Param_ is the first parameter of the function specified by _Invoke_ and must be "
        "a pointer to an 8-bit <<Integer,_integer type_>> scalar."
        GAP "_Param Size_ is the size in bytes of the memory pointed to by _Param_ and must be a 32-bit <<Integer,_integer type_>> scalar, which is treated as an unsigned integer."
        GAP "_Param Align_ is the alignment of _Param_ and must be a 32-bit <<Integer,_integer type_>> scalar, which is treated as an unsigned integer.";

    InstructionDesc[OpSizeOf].opDesc = "Computes the run-time size of the type pointed to by _Pointer_"
        GAP "_Result Type_ must be a 32-bit <<Integer,_integer type_>> scalar."
        GAP "_Pointer_ must point to a concrete type.";

    InstructionDesc[OpNamedBarrierInitialize].opDesc = "Declare a new named-barrier object."
        GAP "_Result Type_ must be the type <<OpTypeNamedBarrier, *OpTypeNamedBarrier*>>."
        GAP "_Subgroup Count_ must be a 32-bit <<Integer,_integer type_>> scalar representing the number of subgroups that must reach the current point of execution.";

    InstructionDesc[OpMemoryNamedBarrier].opDesc = "Wait for other invocations of this module to reach the current point of execution."
        GAP "_Named Barrier_ must be the type <<OpTypeNamedBarrier, *OpTypeNamedBarrier*>>."
        GAP "If _Semantics_ is not *None*, this instruction also serves as an <<OpMemoryBarrier,*OpMemoryBarrier*>> "
            "instruction, and also performs and adheres to the description and semantics of an *OpMemoryBarrier* instruction with the "
            "same _Memory_ and _Semantics_ operands. This allows atomically specifying both a control barrier and a memory barrier (that is, without needing two instructions). "
            "If _Semantics_ *None*, _Memory_ is ignored.";

#define GROUP_UNDEFINED_ID(ID)                                                 \
  "The resulting value is <<Poison,poison>> if " ID " "                                \
  "is not part of the <<ScopeRestrictedTangle, scope restricted tangle>>, "   \
  "or is greater than or equal to the size of the "                            \
  "scope. "
#define GROUP_UNDEFINED_ID_NO_INACTIVE(ID)                                     \
  "The resulting value is <<Poison,poison>> if " ID " "                                \
  "is greater than or equal to the size of the "                               \
  "scope. "
#define GROUP_SET_OF_BITFIELDS(ID)                                             \
  ID " is a set of bitfields where the "                                       \
     "first invocation is represented in the lowest bit of the first vector "  \
     "component and the last (up to the size of the <<Scope, scope>>) is the higher bit " \
     "number of the last bitmask needed to represent all bits of the invocations "   \
     "in the scope restricted tangle. "
#define GROUP_IMPLEMENTATION_DEFINED                                           \
    "The method used to perform the group operation on the contributed "       \
    "_Value_(s) from the <<TangledInvocations, tangled invocations>> is implementation defined. "
#define GROUP_NANS                                                                  \
    "From the set of _Value_(s) provided by the <<TangledInvocations, tangled invocations>> within a subgroup, " \
    "if for any two __Value__s one of them is a NaN, the other is chosen. "        \
    "If all _Value_(s) that are used by the current invocation are NaN, "          \
    "then the result is <<Poison,poison>>. "
#define GROUP_CLUSTER_SIZE                                                     \
  GAP "_ClusterSize_ is the size of cluster to use. _ClusterSize_ "            \
  S_U "_ClusterSize_ must come from a "                                        \
  "<<ConstantInstruction,_constant instruction_>>. "                           \
  "<<UndefinedBehavior,Behavior is undefined>> unless _ClusterSize_ is at least 1 and a power of 2. " \
  "If _ClusterSize_ is greater than the size of the "                          \
  "<<Scope, scope>>, executing this "                                          \
  "instruction results in <<UndefinedBehavior, undefined behavior>>."

#define CLUSTER_OPERATION_IDENTITY(I)                                          \
  OPERATION_IDENTITY(I)                                                        \
  "If _Operation_ is *ClusteredReduce*, _ClusterSize_ must be present. "

  InstructionDesc[OpGroupNonUniformElect].opDesc =
      "Result is *true* only in the <<TangledInvocations, tangled invocation>> "
      "with the lowest id within the _Execution_ scope, otherwise result is false."
      GAP
      "_Result Type_ must be a <<Boolean, _Boolean type_>>."
      GAP
      NU_SCOPE GAP
      TANGLED_EXEC_WAIT;

  InstructionDesc[OpGroupNonUniformAll].opDesc =
      "Evaluates a predicate for all <<TangledInvocations, tangled invocations>> within the _Execution_ scope, "
      "resulting in *true* if predicate evaluates to *true* for all "
      "<<TangledInvocations, tangled invocations>> within the _Execution_ scope, otherwise the result is "
      "*false*." GAP
      "_Result Type_ must be a <<Boolean, _Boolean type_>>." GAP NU_SCOPE GAP
      "_Predicate_ must be a <<Boolean, _Boolean type_>>." GAP
      TANGLED_EXEC_WAIT;

  InstructionDesc[OpGroupNonUniformAny].opDesc =
      "Evaluates a predicate for all <<TangledInvocations, tangled invocations>> within the _Execution_ scope, "
      "resulting in *true* if predicate evaluates to *true* for any "
      "<<TangledInvocations, tangled invocations>> within the _Execution_ scope, otherwise the result is "
      "*false*." GAP
      "_Result Type_ must be a <<Boolean, _Boolean type_>>." GAP NU_SCOPE GAP
      "_Predicate_ must be a <<Boolean, _Boolean type_>>." GAP
      TANGLED_EXEC_WAIT;

  InstructionDesc[OpGroupNonUniformAllEqual].opDesc =
      "Evaluates a value for all <<TangledInvocations, tangled invocations>> within the _Execution_ scope. "
      "The result is *true* if _Value_ is equal for all <<TangledInvocations, tangled invocations>> within the _Execution_ scope. "
      "Otherwise, the result is *false*."
      GAP
      "_Result Type_ must be a <<Boolean, _Boolean type_>>." GAP NU_SCOPE
      GAP
      "_Value_ " SV_FP_I_B
      "The compare operation is based on this type, and if it is a floating-point type, "
      "an ordered-and-equal compare is used." GAP
      TANGLED_EXEC_WAIT;

  InstructionDesc[OpGroupNonUniformBroadcast].opDesc =
      "Result is the _Value_ of the <<Invocation,invocation>> identified by the "
      "id _Invocation Id_ to all <<TangledInvocations, tangled invocations>> within the _Execution_ scope."
      GAP
      "_Result Type_ " SV_FP_I_B
      GAP
      NU_SCOPE GAP MATCHING("_Value_")
      GAP
      "_Invocation Id_ " S_U
      GAP
      "Before *version 1.5*, _Invocation Id_ must come from a <<ConstantInstruction,_constant instruction_>>. "
      "Starting with *version 1.5*, this restriction is lifted. "
      "However, <<UndefinedBehavior,behavior is undefined>> when _Invocation Id_ is not <<DynamicallyUniform, dynamically uniform>>."
      GAP
      GROUP_UNDEFINED_ID("_Invocation Id_") GAP
      TANGLED_EXEC_WAIT;

  InstructionDesc[OpGroupNonUniformBroadcastFirst].opDesc =
      "Result is the _Value_ of the <<Invocation,invocation>> from the "
      "<<TangledInvocations, tangled invocations>> with the lowest id within the _Execution_ " "scope to all <<TangledInvocations, tangled invocations>> within the _Execution_ scope."
      GAP
      "_Result Type_ " SV_FP_I_B GAP NU_SCOPE GAP MATCHING("_Value_")
      GAP
      TANGLED_EXEC_WAIT;

  InstructionDesc[OpGroupNonUniformBallot].opDesc =
      "Result is a bitfield value combining the _Predicate_ value from all "
      "<<TangledInvocations, tangled invocations>> within the _Execution_ scope that execute the same dynamic instance of this "
      "instruction. "
      "The bit is set to 1 if the corresponding invocation is part of "
      "the <<TangledInvocations, tangled invocations>> within the _Execution_ scope and the "
      "_Predicate_ for that invocation evaluated to true; otherwise, it is set "
      "to 0." GAP
      "_Result Type_ " V4_U GAP GROUP_SET_OF_BITFIELDS("_Result_")
      GAP NU_SCOPE GAP "_Predicate_ must be a <<Boolean, _Boolean type_>>." GAP
      TANGLED_EXEC_WAIT;

  InstructionDesc[OpGroupNonUniformInverseBallot].opDesc =
      "Evaluates a value for all <<TangledInvocations, tangled invocations>> within the _Execution_ scope, "
      "resulting in *true* if the bit in _Value_ for the corresponding "
      "invocation is set to 1, otherwise the result is *false*." GAP
      "_Result Type_ must be a <<Boolean, _Boolean type_>>." GAP NU_SCOPE GAP
      "_Value_ " V4_U GAP
      "<<UndefinedBehavior,Behavior is undefined>> unless _Value_ is the same for all invocations that execute the same "
      "dynamic instance of this instruction." GAP GROUP_SET_OF_BITFIELDS("_Value_") GAP
      TANGLED_EXEC_WAIT;

  InstructionDesc[OpGroupNonUniformBallotBitExtract].opDesc =
      "Evaluates a value for all <<TangledInvocations, tangled invocations>> within the _Execution_ scope, "
      "resulting in *true* if the bit in _Value_ that corresponds to _Index_ "
      "is set to one, otherwise the result is *false*." GAP
      "_Result Type_ must be a <<Boolean, _Boolean type_>>." GAP NU_SCOPE GAP
      "_Value_ " V4_U GAP GROUP_SET_OF_BITFIELDS("_Value_") GAP
      "_Index_ " S_U GAP GROUP_UNDEFINED_ID_NO_INACTIVE("_Index_") GAP
      TANGLED_EXEC_WAIT;

  InstructionDesc[OpGroupNonUniformBallotBitCount].opDesc =
      "Result is the number of bits that are set to 1 in "
      "_Value_, considering only the bits in _Value_ required to represent all "
      "bits of the <<ScopeRestrictedTangle, scope restricted tangle>>."
      GAP
      "_Result Type_ " S_U GAP
      NU_SCOPE
      OPERATION_IDENTITY("0") GAP
      "_Value_ " V4_U GAP
      GROUP_SET_OF_BITFIELDS("_Value_") GAP
      TANGLED_EXEC_WAIT;

  InstructionDesc[OpGroupNonUniformBallotFindLSB].opDesc =
      "Find the least significant bit set to 1 in _Value_, considering "
      "only the bits in _Value_ required to represent all bits of the <<ScopeRestrictedTangle, scope restricted tangle>>. "
      "If none of the considered bits is set to 1, the resulting value is <<Poison,poison>>." GAP
      "_Result Type_ " S_U GAP NU_SCOPE GAP
      "_Value_ " V4_U GAP
      GROUP_SET_OF_BITFIELDS("_Value_") GAP
      TANGLED_EXEC_WAIT;

  InstructionDesc[OpGroupNonUniformBallotFindMSB].opDesc =
      "Find the most significant bit set to 1 in _Value_, considering "
      "only the bits in _Value_ required to represent all bits of the <<ScopeRestrictedTangle, scope restricted tangle>>. "
      "If none of the considered bits is set to 1, the resulting value is <<Poison,poison>>." GAP
      "_Result Type_ " S_U GAP NU_SCOPE GAP
      "_Value_ " V4_U GAP
      GROUP_SET_OF_BITFIELDS("_Value_") GAP
      TANGLED_EXEC_WAIT;

  InstructionDesc[OpGroupNonUniformShuffle].opDesc =
      "Result is the _Value_ of the <<Invocation,invocation>> identified by the "
      "id _Invocation Id_." GAP
      "_Result Type_ " SV_FP_I_B GAP NU_SCOPE GAP MATCHING("_Value_") GAP
      "_Invocation Id_ " S_U GAP GROUP_UNDEFINED_ID("_Invocation Id_") GAP
      TANGLED_EXEC_WAIT;

  InstructionDesc[OpGroupNonUniformShuffleXor].opDesc =
      "Result is the _Value_ of the <<Invocation,invocation>> identified by the "
      "current invocation's id within the <<Scope, scope>> xor'ed with _Mask_." GAP
      "_Result Type_ " SV_FP_I_B GAP NU_SCOPE GAP MATCHING("_Value_") GAP
      "_Mask_ " S_U GAP GROUP_UNDEFINED_ID(
          "current invocation's id within the scope xor'ed with "
          "_Mask_") GAP
      TANGLED_EXEC_WAIT;

  InstructionDesc[OpGroupNonUniformShuffleUp].opDesc =
      "Result is the _Value_ of the <<Invocation,invocation>> identified by the "
      "current invocation's id within the <<Scope, scope>> - _Delta_."
      GAP
      "_Result Type_ " SV_FP_I_B GAP NU_SCOPE GAP MATCHING("_Value_")
      GAP
      "_Delta_ " S_U
      GAP
      "_Delta_ is treated as unsigned. The resulting value is <<Poison,poison>> if _Delta_ is greater than the current "
      "invocation's id within the scope or if the identified invocation "
      "is not in <<ScopeRestrictedTangle, scope restricted tangle>>." GAP
      TANGLED_EXEC_WAIT;

  InstructionDesc[OpGroupNonUniformShuffleDown].opDesc =
      "Result is the _Value_ of the <<Invocation,invocation>> identified by the "
      "current invocation's id within the <<Scope, scope>> + _Delta_." GAP
      "_Result Type_ " SV_FP_I_B GAP NU_SCOPE GAP MATCHING("_Value_") GAP
      "_Delta_ " S_U GAP
      "_Delta_ is treated as unsigned. The resulting value is <<Poison,poison>> if _Delta_ is greater than or equal to the "
      "size of the scope, or if the identified invocation is not in <<ScopeRestrictedTangle, scope restricted tangle>>"
      GAP
      TANGLED_EXEC_WAIT;

  InstructionDesc[OpGroupNonUniformIAdd].opDesc =
      "An integer add <<Group_Operation,group operation>> of all _Value_ "
      "operands contributed by all <<TangledInvocations, tangled invocations>> within the _Execution_ scope."
      GAP
      "_Result Type_ " SV_I GAP NU_SCOPE
      CLUSTER_OPERATION_IDENTITY("0")
      GAP MATCHING("_Value_")
      GROUP_CLUSTER_SIZE GAP
      TANGLED_EXEC_WAIT;

  InstructionDesc[OpGroupNonUniformFAdd].opDesc =
      "A floating point add <<Group_Operation,group operation>> of all _Value_ "
      "operands contributed by all <<TangledInvocations, tangled invocations>> within the _Execution_ scope."
      GAP
      "_Result Type_ " SV_FP GAP NU_SCOPE
      CLUSTER_OPERATION_IDENTITY("0")
      GAP MATCHING("_Value_") GROUP_IMPLEMENTATION_DEFINED
      GROUP_CLUSTER_SIZE GAP
      TANGLED_EXEC_WAIT;

  InstructionDesc[OpGroupNonUniformIMul].opDesc =
      "An integer multiply <<Group_Operation,group operation>> of all _Value_ "
      "operands contributed by all <<TangledInvocations, tangled invocations>> within the _Execution_ scope."
      GAP
      "_Result Type_ " SV_I GAP NU_SCOPE
      CLUSTER_OPERATION_IDENTITY("1")
      GAP MATCHING("_Value_")
      GROUP_CLUSTER_SIZE GAP
      TANGLED_EXEC_WAIT;

  InstructionDesc[OpGroupNonUniformFMul].opDesc =
      "A floating point multiply <<Group_Operation,group operation>> of all "
      "_Value_ operands contributed by all <<TangledInvocations, tangled invocations>> "
      "within the _Execution_ scope." GAP
      "_Result Type_ " SV_FP GAP NU_SCOPE
      CLUSTER_OPERATION_IDENTITY("1")
      GAP MATCHING("_Value_") GROUP_IMPLEMENTATION_DEFINED
      GROUP_CLUSTER_SIZE GAP
      TANGLED_EXEC_WAIT;

  InstructionDesc[OpGroupNonUniformSMin].opDesc =
      "A signed integer minimum <<Group_Operation,group operation>> of all "
      "_Value_ operands contributed by all <<TangledInvocations, tangled invocations>> "
      "within the _Execution_ scope." GAP
      "_Result Type_ " SV_I GAP NU_SCOPE
      CLUSTER_OPERATION_IDENTITY("INT_MAX")
      GAP MATCHING("_Value_")
      GROUP_CLUSTER_SIZE GAP
      TANGLED_EXEC_WAIT;

  InstructionDesc[OpGroupNonUniformUMin].opDesc =
      "An unsigned integer minimum <<Group_Operation,group operation>> of all "
      "_Value_ operands contributed by all <<TangledInvocations, tangled invocations>> "
      "within the _Execution_ scope." GAP
      "_Result Type_ " SV_U GAP NU_SCOPE
      CLUSTER_OPERATION_IDENTITY("UINT_MAX")
      GAP MATCHING("_Value_")
      GROUP_CLUSTER_SIZE GAP
      TANGLED_EXEC_WAIT;

  InstructionDesc[OpGroupNonUniformFMin].opDesc =
      "A floating point minimum <<Group_Operation,group operation>> of all "
      "_Value_ operands contributed by all <<TangledInvocations, tangled invocations>> "
      "within the _Execution_ scope." GAP
      "_Result Type_ " SV_FP GAP NU_SCOPE
      CLUSTER_OPERATION_IDENTITY("+INF")
      GAP MATCHING("_Value_") GROUP_IMPLEMENTATION_DEFINED GROUP_NANS
      GROUP_CLUSTER_SIZE GAP
      TANGLED_EXEC_WAIT;

  InstructionDesc[OpGroupNonUniformSMax].opDesc =
      "A signed integer maximum <<Group_Operation,group operation>> of all "
      "_Value_ operands contributed by all <<TangledInvocations, tangled invocations>> "
      "within the _Execution_ scope." GAP
      "_Result Type_ " SV_I GAP NU_SCOPE
      CLUSTER_OPERATION_IDENTITY("INT_MIN")
      GAP MATCHING("_Value_")
      GROUP_CLUSTER_SIZE GAP
      TANGLED_EXEC_WAIT;

  InstructionDesc[OpGroupNonUniformUMax].opDesc =
      "An unsigned integer maximum <<Group_Operation,group operation>> of all "
      "_Value_ operands contributed by all <<TangledInvocations, tangled invocations>> "
      "within the _Execution_ scope." GAP
      "_Result Type_ " SV_U GAP NU_SCOPE
      CLUSTER_OPERATION_IDENTITY("0")
      GAP MATCHING("_Value_")
      GROUP_CLUSTER_SIZE GAP
      TANGLED_EXEC_WAIT;

  InstructionDesc[OpGroupNonUniformFMax].opDesc =
      "A floating point maximum <<Group_Operation,group operation>> of all "
      "_Value_ operands contributed by all <<TangledInvocations, tangled invocations>> "
      "within the _Execution_ scope." GAP
      "_Result Type_ " SV_FP GAP NU_SCOPE
      CLUSTER_OPERATION_IDENTITY("-INF")
      GAP MATCHING("_Value_") GROUP_IMPLEMENTATION_DEFINED GROUP_NANS
      GROUP_CLUSTER_SIZE GAP
      TANGLED_EXEC_WAIT;

  InstructionDesc[OpGroupNonUniformBitwiseAnd].opDesc =
      "A bitwise and <<Group_Operation,group operation>> of all _Value_ "
      "operands contributed by all <<TangledInvocations, tangled invocations>> within the _Execution_ scope."
      GAP "_Result Type_ " SV_I GAP NU_SCOPE
      CLUSTER_OPERATION_IDENTITY("~0")
      GAP MATCHING("_Value_")
      GROUP_CLUSTER_SIZE GAP
      TANGLED_EXEC_WAIT;

  InstructionDesc[OpGroupNonUniformBitwiseOr].opDesc =
      "A bitwise or <<Group_Operation,group operation>> of all _Value_ "
      "operands contributed by all <<TangledInvocations, tangled invocations>> within the _Execution_ scope."
      GAP "_Result Type_ " SV_I GAP NU_SCOPE
      CLUSTER_OPERATION_IDENTITY("0")
      GAP MATCHING("_Value_")
      GROUP_CLUSTER_SIZE GAP
      TANGLED_EXEC_WAIT;

  InstructionDesc[OpGroupNonUniformBitwiseXor].opDesc =
      "A bitwise xor <<Group_Operation,group operation>> of all _Value_ "
      "operands contributed by all <<TangledInvocations, tangled invocations>> within the _Execution_ scope."
      GAP "_Result Type_ " SV_I GAP NU_SCOPE
      CLUSTER_OPERATION_IDENTITY("0")
      GAP MATCHING("_Value_")
      GROUP_CLUSTER_SIZE GAP
      TANGLED_EXEC_WAIT;

  InstructionDesc[OpGroupNonUniformLogicalAnd].opDesc =
      "A logical and <<Group_Operation,group operation>> of all _Value_ "
      "operands contributed by all <<TangledInvocations, tangled invocations>> within the _Execution_ scope."
      GAP "_Result Type_ " SV_B GAP NU_SCOPE
      CLUSTER_OPERATION_IDENTITY("~0")
      GAP MATCHING("_Value_")
      GROUP_CLUSTER_SIZE GAP
      TANGLED_EXEC_WAIT;

  InstructionDesc[OpGroupNonUniformLogicalOr].opDesc =
      "A logical or <<Group_Operation,group operation>> of all _Value_ "
      "operands contributed by all <<TangledInvocations, tangled invocations>> within the _Execution_ scope."
      GAP "_Result Type_ " SV_B GAP NU_SCOPE
      CLUSTER_OPERATION_IDENTITY("0")
      GAP MATCHING("_Value_")
      GROUP_CLUSTER_SIZE GAP
      TANGLED_EXEC_WAIT;

  InstructionDesc[OpGroupNonUniformLogicalXor].opDesc =
      "A logical xor <<Group_Operation,group operation>> of all _Value_ "
      "operands contributed by all <<TangledInvocations, tangled invocations>> within the _Execution_ scope."
      GAP "_Result Type_ " SV_B GAP NU_SCOPE
      CLUSTER_OPERATION_IDENTITY("0")
      GAP MATCHING("_Value_")
      GROUP_CLUSTER_SIZE GAP
      TANGLED_EXEC_WAIT;

  InstructionDesc[OpGroupNonUniformQuadBroadcast].opDesc =
      "Result is the _Value_ of the <<Invocation,invocation>> within the <<Quad, quad>> with "
      "a <<QuadIndex,quad index>> equal to _Index_."
      GAP
      "_Result Type_ " SV_FP_I_B
      GAP
      "_Execution_ is a <<Scope_-id-, _Scope_>>, but has no effect on the behavior of this instruction. "
      "It must be *Subgroup*."
      GAP MATCHING("_Value_")
      GAP
      "_Index_ " S_U
      GAP
      "Before *version 1.5*, _Index_ must come from a <<ConstantInstruction,_constant instruction_>>. "
      "Starting with *version 1.5*, _Index_ must be <<DynamicallyUniform, dynamically uniform>>."
      GAP
      "If the value of _Index_ is greater than or equal to 4, or refers to an "
      "invocation not part of the <<TangledInvocations, tangled invocations>> within the <<Quad, quad>>, "
      "the resulting value is <<Poison,poison>>." GAP
      QUAD_TANGLED_EXEC_WAIT;

  InstructionDesc[OpGroupNonUniformQuadSwap].opDesc =
      "Swap the _Value_ of the <<Invocation,invocation>> within the <<Quad, quad>> with "
      "another invocation in the quad using _Direction_." GAP
      "_Result Type_ " SV_FP_I_B
      GAP
      "_Execution_ is a <<Scope_-id-, _Scope_>>, but has no effect on the behavior of this instruction. "
      "It must be *Subgroup*."
      GAP MATCHING("_Value_") GAP
      "_Direction_ is the kind of swap to perform." GAP
      "_Direction_ " S_U GAP "_Direction_ must come from a "
      "<<ConstantInstruction,_constant instruction_>>." GAP
      "The value returned in _Result_ is the value provided to _Value_ by another "
      "invocation in the same quad scope instance. "
      "The invocation providing this value is determined according to _Direction_."
      GAP
      "A _Direction_ of 0 indicates a horizontal swap;"
      GAP
      " - Invocations with <<QuadIndex,quad indices>> of 0 and 1 swap values\n"
      " - Invocations with <<QuadIndex,quad indices>> of 2 and 3 swap values\n"
      GAP
      "A _Direction_ of 1 indicates a vertical swap;"
      GAP
      " - Invocations with <<QuadIndex,quad indices>> of 0 and 2 swap values\n"
      " - Invocations with <<QuadIndex,quad indices>> of 1 and 3 swap values\n"
      GAP
      "A _Direction_ of 2 indicates a diagonal swap;"
      GAP
      " - Invocations with <<QuadIndex,quad indices>> of 0 and 3 swap values\n"
      " - Invocations with <<QuadIndex,quad indices>> of 1 and 2 swap values\n"
      GAP
      "_Direction_ must be one of the above values." GAP
      "If a <<TangledInvocations, tangled invocation>> within the <<Quad, quad>> reads _Value_ from an invocation not part of the <<TangledInvocations, tangled invocation>> within the same <<Quad, quad>>, the resulting "
      "value is <<Poison,poison>>." GAP
      QUAD_TANGLED_EXEC_WAIT;

  InstructionDesc[OpSDot].opDesc =
      "Signed integer dot product of _Vector 1_ and _Vector 2_."
      GAP
      "_Result Type_ must be an scalar integer type whose _Width_ must be greater than or equal to that of the components of "
      "_Vector 1_ and _Vector 2_."
      GAP
      "_Vector 1_ and _Vector 2_ must have the same type."
      GAP
      "_Vector 1_ and _Vector 2_ must be either 32-bit integers (enabled by the *DotProductInput4x8BitPacked* <<Capability, capability>>) or vectors of integer type "
      "(enabled by the *DotProductInput4x8Bit* or *DotProductInputAll* <<Capability, capability>>)."
      GAP
      "When _Vector 1_ and _Vector 2_ are scalar integer types, _Packed Vector Format_ must be specified to select how the integers are "
      "to be interpreted as vectors."
      GAP
      "All components of the input vectors are sign-extended to the bit width of the result’s type. The sign-extended input vectors are "
      "then multiplied component-wise and all components of the vector resulting from the component-wise multiplication are added "
      "together. The resulting value will equal the low-order N bits of the correct result R, where N is the result width and R is "
      "computed with enough precision to avoid overflow and underflow.";

  InstructionDesc[OpUDot].opDesc =
      "Unsigned integer dot product of _Vector 1_ and _Vector 2_."
      GAP
      "_Result Type_ must be an scalar integer type with _Signedness_ of 0 whose _Width_ must be greater than or equal to that of the components "
      "of _Vector 1_ and _Vector 2_."
      GAP
      "_Vector 1_ and _Vector 2_ must have the same type."
      GAP
      "_Vector 1_ and _Vector 2_ must be either 32-bit integers (enabled by the *DotProductInput4x8BitPacked* <<Capability, capability>>) or vectors of integer type with "
      "_Signedness_ of 0 (enabled by the *DotProductInput4x8Bit* or *DotProductInputAll* <<Capability, capability>>)."
      GAP
      "When _Vector 1_ and _Vector 2_ are scalar integer types, _Packed Vector Format_ must be specified to select how the integers are to "
      "be interpreted as vectors."
      GAP
      "All components of the input vectors are zero-extended to the bit width of the result’s type. The zero-extended input vectors are "
      "then multiplied component-wise and all components of the vector resulting from the component-wise multiplication are added "
      "together. The resulting value will equal the low-order N bits of the correct result R, where N is the result width and R is "
      "computed with enough precision to avoid overflow and underflow.";

  InstructionDesc[OpSUDot].opDesc =
      "Mixed-signedness integer dot product of _Vector 1_ and _Vector 2_. Components of _Vector 1_ are treated as signed, components of "
      "_Vector 2_ are treated as unsigned."
      GAP
      "_Result Type_ must be an scalar integer type whose _Width_ must be greater than or equal to that of the components of "
      "_Vector 1_ and _Vector 2_."
      GAP
      "_Vector 1_ and _Vector 2_ must be either 32-bit integers (enabled by the *DotProductInput4x8BitPacked* <<Capability, capability>>) or vectors of integer type with "
      "the same number of components and same component _Width_ (enabled by the *DotProductInput4x8Bit* or *DotProductInputAll* <<Capability, capability>>). When "
      "_Vector 1_ and _Vector 2_ are vectors, the components of _Vector 2_ must have a _Signedness_ of 0."
      GAP
      "When _Vector 1_ and _Vector 2_ are scalar integer types, _Packed Vector Format_ must be specified to select how the integers are to "
      "be interpreted as vectors."
      GAP
      "All components of _Vector 1_ are sign-extended to the bit width of the result’s type. All components of _Vector 2_ are "
      "zero-extended to the bit width of the result’s type. The sign- or zero-extended input vectors are then multiplied component-wise "
      "and all components of the vector resulting from the component-wise multiplication are added together. The resulting value will "
      "equal the low-order N bits of the correct result R, where N is the result width and R is computed with enough precision to avoid "
      "overflow and underflow.";

  InstructionDesc[OpSDotAccSat].opDesc =
      "Signed integer dot product of _Vector 1_ and _Vector 2_ and signed saturating addition of the result with _Accumulator_."
      GAP
      "_Result Type_ must be an scalar integer type whose _Width_ must be greater than or equal to that of the components of "
      "_Vector 1_ and _Vector 2_."
      GAP
      "_Vector 1_ and _Vector 2_ must have the same type."
      GAP
      "_Vector 1_ and _Vector 2_ must be either 32-bit integers (enabled by the *DotProductInput4x8BitPacked* <<Capability, capability>>) or vectors of integer type "
      "(enabled by the *DotProductInput4x8Bit* or *DotProductInputAll* <<Capability, capability>>)."
      GAP
      "The type of _Accumulator_ must be the same as _Result Type_."
      GAP
      "When _Vector 1_ and _Vector 2_ are scalar integer types, _Packed Vector Format_ must be specified to select how the integers are "
      "to be interpreted as vectors."
      GAP
      "All components of the input vectors are sign-extended to the bit width of the result’s type. The sign-extended input vectors are "
      "then multiplied component-wise and all components of the vector resulting from the component-wise multiplication are added "
      "together. Finally, the resulting sum is added to the input accumulator. This final addition is saturating."
      GAP
      "If any of the multiplications or additions, with the exception of the final accumulation, overflow or underflow, the result of the "
      "instruction is <<Poison,_poison_>>.";

  InstructionDesc[OpUDotAccSat].opDesc =
      "Unsigned integer dot product of _Vector 1_ and _Vector 2_ and unsigned saturating addition of the result with _Accumulator_."
      GAP
      "_Result Type_ must be an scalar integer type with _Signedness_ of 0 whose _Width_ must be greater than or equal to that of the components "
      "of _Vector 1_ and _Vector 2_."
      GAP
      "_Vector 1_ and _Vector 2_ must have the same type."
      GAP
      "_Vector 1_ and _Vector 2_ must be either 32-bit integers (enabled by the *DotProductInput4x8BitPacked* <<Capability, capability>>) or vectors of integer type with "
      "_Signedness_ of 0 (enabled by the *DotProductInput4x8Bit* or *DotProductInputAll* <<Capability, capability>>)."
      GAP
      "The type of _Accumulator_ must be the same as _Result Type_."
      GAP
      "When _Vector 1_ and _Vector 2_ are scalar integer types, _Packed Vector Format_ must be specified to select how the integers are to "
      "be interpreted as vectors."
      GAP
      "All components of the input vectors are zero-extended to the bit width of the result’s type. The zero-extended input vectors are "
      "then multiplied component-wise and all components of the vector resulting from the component-wise multiplication are added "
      "together. Finally, the resulting sum is added to the input accumulator. This final addition is saturating."
      GAP
      "If any of the multiplications or additions, with the exception of the final accumulation, overflow or underflow, the result of the "
      "instruction is <<Poison,_poison_>>.";

  InstructionDesc[OpSUDotAccSat].opDesc =
      "Mixed-signedness integer dot product of _Vector 1_ and _Vector 2_ and signed saturating addition of the result with _Accumulator_. "
      "Components of _Vector 1_ are treated as signed, components of _Vector 2_ are treated as unsigned."
      GAP
      "_Result Type_ must be an scalar integer type whose _Width_ must be greater than or equal to that of the components of "
      "_Vector 1_ and _Vector 2_."
      GAP
      "_Vector 1_ and _Vector 2_ must be either 32-bit integers (enabled by the *DotProductInput4x8BitPacked* <<Capability, capability>>) or vectors of integer type with "
      "the same number of components and same component _Width_ (enabled by the *DotProductInput4x8Bit* or *DotProductInputAll* <<Capability, capability>>). When "
      "_Vector 1_ and _Vector 2_ are vectors, the components of _Vector 2_ must have a _Signedness_ of 0."
      GAP
      "The type of _Accumulator_ must be the same as _Result Type_."
      GAP
      "When _Vector 1_ and _Vector 2_ are scalar integer types, _Packed Vector Format_ must be specified to select how the integers are to "
      "be interpreted as vectors."
      GAP
      "All components of _Vector 1_ are sign-extended to the bit width of the result’s type. All components of _Vector 2_ are "
      "zero-extended to the bit width of the result’s type. The sign- or zero-extended input vectors are then multiplied component-wise "
      "and all components of the vector resulting from the component-wise multiplication are added together. Finally, the resulting sum is "
      "added to the input accumulator. This final addition is saturating."
      GAP
      "If any of the multiplications or additions, with the exception of the final accumulation, overflow or underflow, the result of the "
      "instruction is <<Poison,_poison_>>.";
}

//
// Functions that translate non-spirv-declared enums to English.
//

const char* GetOperandDesc(OperandClass operand)
{
    switch (operand) {
    case OperandId:                    return "<id>";
    case OperandVariableIds:           return "<id>, <id>, ...";
    case OperandVariableLiterals:      return "Literal, Literal, ...";
    case OperandOptionalLiteral:       return "Optional literal(s)";
    case OperandOptionalLiteralString: return "Optional <<Literal, Literal>>";
    case OperandOptionalLiteralStrings:return "Optional <<Literal, Literals>>";
    case OperandVariableIdLiteral:     return "<id> 1, literal 1, <id> 2, literal 2, ...";
    case OperandVariableLiteralId:     return "literal 1, label <id> 1, literal 2, label <id> 2, ...";
    case OperandLiteralNumber:         return "Literal";
    case OperandAnySizeLiteralNumber:  return "Literal";
    case OperandLiteralString:         return "Literal";
    case OperandSource:                return "Source Language";
    case OperandExecutionModel:        return "Execution Model";
    case OperandAddressing:            return "Addressing Model";
    case OperandMemory:                return "Memory Model";
    case OperandExecutionMode:         return "Execution Mode";
    case OperandStorage:               return "Storage Class";
    case OperandDimensionality:        return "Dim";
    case OperandDecoration:            return "Decoration";
    case OperandBuiltIn:               return "BuiltIn";
    case OperandSelect:                return "Selection Control";
    case OperandLoop:                  return "Loop Control";
    case OperandFunction:              return "Function Control";
    case OperandSamplerAddressingMode: return "Sampler Addressing Mode";
    case OperandSamplerFilterMode:     return "Sampler Filter Mode";
    case OperandSamplerImageFormat:    return "Image Format";
    case OperandImageChannelOrder:     return "Image Channel Order";
    case OperandImageChannelDataType:  return "Image Channel Data Type";
    case OperandImageOperands:         return "Image Operands";
    case OperandFPFastMath:            return "FP Fast Math Mode";
    case OperandFPRoundingMode:        return "FP Rounding Mode";
    case OperandLinkageType:           return "Linkage Type";
    case OperandFuncParamAttr:         return "Function Parameter Attribute";
    case OperandAccessQualifier:       return "Access Qualifier";
    case OperandMemorySemantics:       return "Memory Semantics <id>";
    case OperandMemoryOperands:        return "Memory Operands";
    case OperandScope:                 return "Scope <id>";
    case OperandGroupOperation:        return "Group Operation";
    case OperandKernelEnqueueFlags:    return "Kernel Enqueue Flags";
    case OperandKernelProfilingInfo:   return "Kernel Profiling Info";
    case OperandCapability:            return "Capability";
    case OperandRayFlags:                            return "Ray Flags";
    case OperandRayQueryIntersection:                return "Ray Query Intersection";
    case OperandRayQueryCommittedIntersectionType:   return "Ray Query Committed Type";
    case OperandRayQueryCandidateIntersectionType:   return "Ray Query Candidate Type";
    case OperandFragmentShadingRate:                 return "Fragment Shading Rate";
    case OperandFPDenormMode:          return "FP Denorm Mode";
    case OperandFPOperationMode:       return "FP Operation Mode";
    case OperandQuantizationModes:     return "Quantization Mode";
    case OperandOverflowModes:         return "Overflow Mode";
    case OperandPackedVectorFormat:    return "Packed Vector Format";
    case OperandCooperativeMatrixOperands:      return "Cooperative Matrix Operands";
    case OperandCooperativeMatrixLayout:        return "Cooperative Matrix Layout";
    case OperandCooperativeMatrixUse:           return "Cooperative Matrix Use";
    case OperandInitializationModeQualifier:    return "Initialization Mode Qualifier";
    case OperandHostAccessQualifier:    return "Host Access Qualifier";
    case OperandLoadCacheControl:       return "Load Cache Control";
    case OperandStoreCacheControl:      return "Store Cache Control";
    case OperandNamedMaximumNumberOfRegisters: return "Named Maximum Number of Registers";
    case OperandRawAccessChainOperands: return "Raw Access Chain Operands";
    case OperandFPEncoding:             return "FP Encoding";
    case OperandTensorAddressingOperands:    return "Tensor Addressing Operands";
    case OperandCooperativeMatrixReduce:     return "Cooperative Matrix Reduce Mode";
    case OperandTensorClampMode:       return "Tensor Clamp Mode";
    case OperandMatrixMultiplyAccumulateOperands: return "Matrix Multiply Accumulate Operands";
    case OperandCooperativeVectorMatrixLayout: return "Cooperative Vector Matrix Layout";
    case OperandComponentType: return "Cooperative Vector Matrix Component Type";
    case OperandTensorOperands: return "Tensor Operands";
    case OperandGatherModes:    return "Gather Modes";
    case OperandOpcode:                return "Op";

    default:                           return "Reserved";
    }
}

const char* AddressingDesc(int addr)
{
    switch (addr) {
    case 0:  return "No variables that are pointers or other physical pointers. No arithmetic or casting on pointers.";
    case 1:  return "";
    case 2:  return "";

    default: return "Bad";
    }
}

// OpImageSparseSampleProj* instruction were removed without
// being actually removed. We keep the enum reservation
// in the grammar but we no longer print out the entry for them.
bool IgnoreInstruction(const InstructionValue &inst)
{
    switch (inst.value)
    {
    case OpImageSparseSampleProjImplicitLod:
    case OpImageSparseSampleProjExplicitLod:
    case OpImageSparseSampleProjDrefImplicitLod:
    case OpImageSparseSampleProjDrefExplicitLod:
        return true;
    default:
        return false;
    }
}

bool IsExtensionEnabled(const InstructionValue &inst) {
    switch (inst.value)
    {
    case OpExtInstWithForwardRefsKHR:
    case OpMemberDecorateString:
    case OpDecorateString:
    case OpDecorateId:
    case OpGroupIAddNonUniformAMD:
    case OpGroupFAddNonUniformAMD:
    case OpGroupFMinNonUniformAMD:
    case OpGroupUMinNonUniformAMD:
    case OpGroupSMinNonUniformAMD:
    case OpGroupFMaxNonUniformAMD:
    case OpGroupUMaxNonUniformAMD:
    case OpGroupSMaxNonUniformAMD:
        return true;
    default:
        return false;
    }
}

} // end anonymous namespace

namespace spv {

//
// Set of functions for printing documentation.
// Currently, this is being printed out as asciidoc.
//

void SafeTag(std::string& tag)
{
    std::transform(tag.begin(), tag.end(), tag.begin(), [](char ch) {
        switch (ch) {
        case ' ':
            return '_';
        case '<':
        case '>':
            return '-';
        default:
            return ch;
        }
    });
}

// Section title for the following table.
void PrintSectionHeader(const char* desc, const char* tag = 0)
{
    std::string safe = tag ? tag : desc;
    SafeTag(safe);
    printf("\n[[%s]]\n=== %s\n", safe.c_str(), desc);
}
void PrintSubSectionHeader(const char* desc, const char* tag = 0)
{
    std::string safe = tag ? tag : desc;
    SafeTag(safe);
    printf("\n[[%s]]\n==== %s\n", safe.c_str(), desc);
}

void PrintMagicNumber()
{
    PrintSectionHeader("Magic Number", "Magic");
    printf("Magic number for a SPIR-V module.\n\n"
           "TIP: *Endianness:* A module is defined as a stream of words, not a stream of bytes. "
           "However, if stored as a stream of bytes (e.g., in a file), "
           "the magic number can be used to deduce what endianness to apply to convert the byte stream back to a word stream."
           "\n");

    printf("[cols=\"^1\",options=\"header\",width = \"20%%\"]\n");
    printf("|====\n");
    printf("| Magic Number \n");
    printf("| 0x%8.8x \n", spv::MagicNumber);
    printf("|====\n");
}

void PrintEnumSectionHeader(OperandClass opClass, bool mask)
{
    // Boiler-plate header:
    PrintSubSectionHeader(GetOperandDesc(opClass));
    if (mask) {
        switch (opClass) {
        case OperandMemorySemantics:
        case OperandKernelProfilingInfo:
            printf("\nThe _<id>_'s value is a mask; "
                   "it can be formed by combining the bits from multiple rows in the table below.\n\n");
            break;
        default:
            printf("\nThis is a literal mask; it can be formed by combining the bits from multiple rows in the table below.\n\n");
            break;
        }
    }
    if (OperandClassParams[opClass].desc)
        printf("%s\n\n", OperandClassParams[opClass].desc);

    // Find all instructions that use this operand class,
    // and generate a list of them.
    std::vector<InstructionValue*> usedBy;
    for (auto& inst : InstructionDesc) {
        if (IgnoreInstruction(inst))
            continue;
        for (int op = 0; op < inst.operands.getNum(); ++op) {
            if (opClass == inst.operands.getClass(op)) {
                usedBy.push_back(&inst);
                break;
            }
        }
    }

    // Emit English version of the generate list.
    if (usedBy.size() > 0 && usedBy.size() < 4) {
        // Short lists use a comma-sentence style.
        printf("Used by ");
        bool commaNeeded = false;
        for (int u = 0; u < (int)usedBy.size(); ++u) {
            if (commaNeeded)
                printf(", ");
            else if (usedBy.size() > 2)
                commaNeeded = true;
            if ((int)usedBy.size() >= 2 && u == (int)usedBy.size() - 1)
                printf(" and ");
            printf("<<%s,*%s*>>", usedBy[u]->name.c_str(), usedBy[u]->name.c_str());
        }
        printf(".\n");
    } else if (usedBy.size() >= 4) {
        // Long lists use a bullet-list style.
        printf("Used by:\n\n");
        for (int u = 0; u < (int)usedBy.size(); ++u) {
            printf("- <<%s,*%s*>>\n", usedBy[u]->name.c_str(), usedBy[u]->name.c_str());
        }
        printf("\n");
    }
}

void StripId(std::string& s)
{
    if (s.size() > 5 && s.compare(s.size() - 5, 5, " <id>") == 0)
        s.resize(s.size() - 5);
}

// Header for the two-column tables for things like Execution Model.
// It gets extra columns if they have capabilities or extensions.
void PrintImmediateHeader(OperandClass opClass, bool caps, bool mask)
{
    PrintEnumSectionHeader(opClass, mask);

    int numberWidth = mask ? 4 : 2;

    printf("// PrintImmediateHeader\n");
    if (caps)
        printf("[cols=\"^.^%d,16a,15\",role=\"fixedtable\",options=\"header\",width = \"100%%\"]\n", numberWidth);
    else
        printf("[cols=\"^.^%d,15a\",role=\"fixedtable\",options=\"header\",width = \"50%%\"]\n", numberWidth);

    printf("|====\n");
    std::string tableTitle = GetOperandDesc(opClass);
    StripId(tableTitle);
    printf("2+^.^| %s", tableTitle.c_str());
    if (caps) {
        if (opClass == OperandCapability) {
            printf("| Implicitly Declares");
        } else {
            printf("| <<Capability,Enabling Capabilities>>");
        }
    }
    printf("\n");
}

// Header for the operand-included tables for things like Execution Mode that
// include their own operands.
void PrintImmediateOpHeader(OperandClass opClass, bool mask, int extraOperands)
{
    PrintEnumSectionHeader(opClass, mask);

    printf("// PrintImmediateOpHeader\n");
    printf("[cols=\"^4,20a,%d*5a,22a\",role=\"fixedtable\",options=\"header\",width = \"100%%\"]\n", extraOperands);
    printf("|====\n");
    std::string tableTitle = GetOperandDesc(opClass);
    StripId(tableTitle);
    printf("2+^.^| %s ", tableTitle.c_str());
    printf("%d+<.^| Extra Operands ", extraOperands);
    printf("| <<Capability,Enabling Capabilities>>\n");
}

// Header for the FP Encoding which includes the valid widths.
void PrintImmediateOpFPEncodingHeader(OperandClass opClass)
{
    PrintEnumSectionHeader(opClass, /*mask=*/false);

    printf("// PrintImmediateOpFPEncodingHeader\n");
    printf("[cols=\"^4,20a,>.<10,22a\",role=\"fixedtable\",options=\"header\",width = \"100%%\"]\n");
    printf("|====\n");
    std::string tableTitle = GetOperandDesc(opClass);
    StripId(tableTitle);
    printf("2+^.^| %s ", tableTitle.c_str());
    printf("^.^| Width(s) <.^| <<Capability,Enabling Capabilities>>\n");
}

// Prints a sequence of strings in the style of capabilities.
template <typename StringList>
void PrintStrings(const StringList& container)
{
    for (int r = 0; r < (int)container.size(); ++r) {
        printf("*%s*", container[r].c_str());
        if (r < (int)container.size() - 1)
            printf(", ");
    }
}

std::set<std::string> ExtensionsNotInRegistry{"SPV_INTEL_debug_module",
                                              "SPV_INTEL_float_controls2",
                                              "SPV_INTEL_function_pointers",
                                              "SPV_INTEL_inline_assembly",
                                              "SPV_INTEL_memory_access_aliasing",
                                              "SPV_INTEL_optnone",
                                              "SPV_INTEL_variable_length_array",
                                              "SPV_INTEL_vector_compute",
                                              "SPV_INTEL_function_variants",
                                              "SPV_INTEL_bindless_images"};

// Prints a sequence of strings in the style of capabilities.
template <typename StringList>
void PrintExtensionStrings(const StringList& container)
{
    for (int r = 0; r < (int)container.size(); ++r) {
        const auto& ext = container[r];
        if (!ExtensionsNotInRegistry.count(ext)) {
            std::string vendor = ext.substr(4, ext.find('_', 4) - 4);
            if (vendor == "NVX" || vendor == "AMDX")
                vendor.pop_back();
            printf("{spirv}/%s/%s.html[*%s*]", vendor.c_str(), ext.c_str(), ext.c_str());
        }
        else
            printf("*%s*", ext.c_str());
        if (r < (int)container.size() - 1)
            printf(", ");
    }
}

void PrintMinVersion(const std::string& version)
{
    if (version.size() == 0)
        return;

    if (version.compare("None") == 0)
        printf("<<Unified, Reserved>>.");
    else
        printf("<<Unified, Missing before>> *version %s*.\n", version.c_str());
}

void PrintMaxVersion(const std::string& version)
{
    if (version.size() == 0)
        return;

    printf("<<Unified, Missing after>> *version %s*.\n", version.c_str());
}

// For when we visit an opcode, deal with the aliases by adding them
// with some syntax to the name of the opcode.
void FormatNameAndAliases(const EnumValue& e, std::string& nameAndAliases) {
    nameAndAliases = e.name;
    if (e.hasAliases()) {
        nameAndAliases.append(" (");
        std::for_each(e.aliases.begin(), e.aliases.end() - 1, [&nameAndAliases](const std::string& value) {
            nameAndAliases.append(value);
            nameAndAliases.append(", ");});
        nameAndAliases.append(e.aliases.back());
        nameAndAliases.append(")");
    }
}

void FormatExtensions(const EnumValue& e) {
    if (e.extensions.size() > 1)
        printf("Also see extensions: ");
    else
        printf("Also see extension: ");
    PrintExtensionStrings(e.extensions);
}

// Print the numeric value, name, an description.
void PrintImmediateValueNameDesc(const EnumValue& e,
                                 bool caps_column, bool hex)
{
    if (hex) {
        unsigned print_value = 1u << e.value;
        // Ugly little hack.
        // TODO(dneto): Make e.value store the *mask* value (so we can represent 0)
        // and we would derive the shift value.
        if (e.name.find("None") == 0)
            print_value = 0;
        printf("| 0x%x ", print_value);
    }
    else
        printf("| %u ", e.value);

    std::string nameAndAliases;
    FormatNameAndAliases(e, nameAndAliases);

    printf(" a| *%s*", nameAndAliases.c_str());
    if (e.desc)
        printf(" +\n%s", e.desc);
}

// Print capabilities and extensions.
void PrintImmediateCapsExts(const EnumValue& e, bool caps_column)
{
    if (caps_column)
        printf(" a|");

    bool printedContentYet = false;

    if (e.capabilities.size() > 0) {
        PrintStrings(e.capabilities);
        printedContentYet = true;
    }
    if ((e.firstVersion.size() > 0 && e.firstVersion != "1.0") || e.lastVersion.size() > 0) {
        if (printedContentYet)
            printf(GAP);
        if (e.firstVersion != "1.0")
            PrintMinVersion(e.firstVersion);
        PrintMaxVersion(e.lastVersion);
        printedContentYet = true;
    }
    if (e.extensions.size() > 0) {
        if (printedContentYet)
            printf(GAP);
        if (e.extensions.size() > 1)
            printf("Also see extensions: ");
        else
            printf("Also see extension: ");
        PrintExtensionStrings(e.extensions);
        printedContentYet = true;
    }
}

// Print Width for FPEncoding.
void PrintFPEncodingWidth(const EnumValue& e)
{
    printf("| ");
    auto& width_set = FPEncodingWidths[static_cast<FPEncoding>(e.value)];
    auto it = width_set.begin();
    auto end = width_set.end();
    while (it != end) {
        printf("%i", *it);
        ++it;
        if (it != end) {
          printf(", ");
        }
    }
    printf("\n");
}

void PrintImmediateRow(int imm, const char* name, bool caps_column, bool hex)
{
    PrintImmediateRow(EnumValue(imm, name, Aliases(), EnumCaps(), std::string(), std::string(), Extensions(),
                                OperandParameters()),
                      caps_column, hex);
}

void PrintImmediateRow(const EnumValue& e,
                       bool caps_column, bool hex)
{
    PrintImmediateValueNameDesc(e, caps_column, hex);
    PrintImmediateCapsExts(e, caps_column);
    printf("\n");
}

void PrintImmediateOpFPEncodingRow(const EnumValue &e, bool caps_column, bool hex)
{
    PrintImmediateValueNameDesc(e, caps_column, hex);
    PrintFPEncodingWidth(e);
    PrintImmediateCapsExts(e, caps_column);
    printf("\n");
}

// Print a row for the operand-included tables, including the operands.
void PrintImmediateOpRow(const EnumValue& e,
                         bool caps_column, bool hex, int extraOperandsToShow)
{
    if (extraOperandsToShow) {
        PrintImmediateValueNameDesc(e, caps_column, hex);
        PrintOperands(e.operands, extraOperandsToShow);
        PrintImmediateCapsExts(e, caps_column);
        printf("\n");
    } else {
        PrintImmediateRow(e, caps_column, hex);
    }
}

// Print operands.
void PrintOperands(const OperandParameters& operands, int reservedOperands)
{
    int numArgs = operands.getNum();
    if (numArgs == 0)
        printf(" %d+|", reservedOperands);

    for (int arg = 0; arg < numArgs; ++arg) {
        if (arg == numArgs - 1 && reservedOperands > arg + 1)
            printf(" %d+| ", reservedOperands - arg);
        else
            printf(" | ");
        std::string operandClassDesc = EmphText(GetOperandDesc(operands.getClass(arg)));
        if (operands.isOptional(arg))
            printf("Optional +\n");
        if (operands.getClass(arg) <= OperandVariableLiteralId)
            printf("%s", operandClassDesc.c_str());
        else {
            std::string safe = GetOperandDesc(operands.getClass(arg));
            SafeTag(safe);
            printf("<<%s,%s>>", safe.c_str(), operandClassDesc.c_str());
        }
        printf(" +\n%s", operands.getDesc(arg));
    }
}

// Print the end of any of the tables.
void PrintImmediateFooter()
{
    printf("|====\n");
}

// Print all the non-opcode tables
void PrintImmediates()
{
    PrintSectionHeader("Enumerants");
    // For each table of enums ...
    for (int operand_source = OperandSource; operand_source < OperandOpcode; ++operand_source) {
        auto& enumSet = OperandClassParams[operand_source];

        // Get the number of
        // extra operands to show in this table.
        int extraOperandsToShow = 0;
        for (auto& value : enumSet) {
            extraOperandsToShow = std::max(extraOperandsToShow, value.operands.getNum());
        }

        // TODO(dneto): Somehow we've only chosen to show operands for
        // Decoration and ExecutionMode. That's not uniform.
        // Shouldn't we also show it for Image Operands and Loop Control, for
        // example?
        switch(operand_source) {
        case OperandDecoration:
        case OperandExecutionMode:
            break;
        default:
            extraOperandsToShow = 0;
            break;
        }

        // Print the table header
        if (extraOperandsToShow)
            PrintImmediateOpHeader((OperandClass)operand_source, enumSet.bitmask, extraOperandsToShow);
        else {
            // TODO(vlom): Special case for FP Encoding (Width(s) column). That's 3 special cases,
            // but we might want to address it differently.
            if (operand_source == OperandFPEncoding)
                PrintImmediateOpFPEncodingHeader((OperandClass)operand_source);
            else
                PrintImmediateHeader((OperandClass)operand_source, /*hasCaps=*/true, enumSet.bitmask);
        }

        // Print an extra "None" row for 0 for mask enums
        if (enumSet.bitmask) {
            switch (operand_source) {
            case OperandMemorySemantics:
                PrintImmediateRow(0, "None (Relaxed)", /*hasCaps=*/true, true);
                break;
            default:
                PrintImmediateRow(0, "None", /*hasCaps=*/true, true);
                break;
            }
        }

        // A table row for each valid enumerant value
        for (auto& e : enumSet) {
            if (operand_source == OperandFPEncoding) {
                PrintImmediateOpFPEncodingRow(e, /*hasCaps=*/true, enumSet.bitmask);
            }
            else if (extraOperandsToShow) {
                PrintImmediateOpRow(e, /*hasCaps=*/true, enumSet.bitmask, extraOperandsToShow);
            } else {
                PrintImmediateRow(e, /*hasCaps=*/true, enumSet.bitmask);
            }
        }
        PrintImmediateFooter();
    }
}

// Print just one class of opcodes
void PrintOpcodeClass(const PrintingClass& printClass)
{
    for (auto& inst : InstructionDesc) {
        if (IgnoreInstruction(inst))
            continue;
        if (printClass.tag != inst.printingClass)
            continue;
        const unsigned op = inst.value;
        std::string nameAndAliases;
        FormatNameAndAliases(inst, nameAndAliases);

        // emit extra page break, if requested
        if (InstrPageBreaks.find((Op)op) != InstrPageBreaks.end())
            printf("<<<\n");

        // Compute word count and number of operands
        int numUsedOperands = inst.operands.getNum();
        int minVariableUsed = 0;
        int numVariableCells = 0;
        if (numUsedOperands > 0) {
            // Operands can be optional either because of the class, or because
            // the instruction has them explicitly listed as optional.
            // If because of their class, they are only the last operand.
            // If because of the instruction, it could be because of multiple operands.
            for (int operand = 0; operand < numUsedOperands; ++operand) {
                if (inst.operands.isOptional(operand))
                    ++numVariableCells;
            }
            switch (inst.operands.getClass(numUsedOperands - 1)) {
            case OperandVariableIds:
            case OperandOptionalLiteral:
            case OperandOptionalLiteralString:
            case OperandOptionalLiteralStrings:
            case OperandVariableLiterals:
            case OperandVariableIdLiteral:
            case OperandVariableLiteralId:
                // count this as optional, but don't double count it if the instruction already
                // said it was optional
                if (! inst.operands.isOptional(numUsedOperands - 1))
                    ++numVariableCells;
                break;
            case OperandAnySizeLiteralNumber:
            case OperandLiteralString:
                ++numVariableCells;
                minVariableUsed = 1;
                break;
            default:
                break;
            }
        }

        int wordCount = 1; // opcode
        if (inst.hasType())
            ++wordCount;
        if (inst.hasResult())
            ++wordCount;
        wordCount += numUsedOperands;
        int cellCount = wordCount + 1; // two cells for first word

        // data for capabilities
        bool capabilities = inst.capabilities.size() > 0 || inst.firstVersion.size() > 0;
        int capabilityCellCount = capabilities ? std::max(1, (cellCount + 2) / 4) : 0;
        float nonCapRatio = ((float)cellCount - (float)capabilityCellCount) / (float)cellCount;

        // Table start
        int width = 100;
        if (numVariableCells > 0)
            printf("[%%unbreakable,cols=\"3,2,%d*3\",role=\"fixedtable\",width=\"%d%%\"]\n", cellCount - 2, width);
        else if (cellCount <= 2)
            printf("[%%unbreakable,cols=\"2,1\",role=\"fixedtable\",width=\"%d%%\"]\n", width);
        else
            printf("[%%unbreakable,cols=\"1,2,%d*3\",role=\"fixedtable\",width=\"%d%%\"]\n", cellCount - 2, width);
        printf("|=====\n");

        // Name
        printf("%d+a|[[%s]]*%s*", cellCount - capabilityCellCount, inst.name.c_str(), nameAndAliases.c_str());

        // Semantics
        printf(" +\n +\n%s\n", inst.opDesc);

        // Capabilities
        if (capabilities) {
            printf("%d+|", capabilityCellCount);
            if (inst.capabilities.size() > 0) {
                printf("<<Capability,Capability>>: +\n");
                PrintStrings(inst.capabilities);
                printf("\n");
            }
            if ((inst.firstVersion.size() > 0 && inst.firstVersion != "1.0") || inst.lastVersion.size() > 0) {
                if (inst.capabilities.size() > 0)
                    printf(GAP);
                if (inst.firstVersion != "1.0")
                    PrintMinVersion(inst.firstVersion);
                PrintMaxVersion(inst.lastVersion);
            }
            if (inst.extensions.size() > 0 && IsExtensionEnabled(inst)) {
                printf(GAP);
                FormatExtensions(inst);
                printf("\n");
            }
        }

        // Word Count
        if (numVariableCells)
            printf("| %d + variable ", wordCount - numVariableCells + minVariableUsed);
        else
            printf("| %d ", wordCount);

        // Opcode
        printf("| %d\n", op);

        // Rest of words
        if (inst.hasType())
            printf(" | %s", "_<id>_ +\n_Result Type_");
        if (inst.hasResult())
            printf(" | %s", "<<ResultId,_Result <id>_ >>");
        PrintOperands(inst.operands, 0);
        printf("\n");

        // Table end
        printf("\n|=====\n");
    }
}

// Print the opcode table.
void PrintOpcodes()
{
    // start instructions on a new page.
    printf("<<<\n");
    PrintSectionHeader("Instructions", "Instructions");

    // Print decoder key
    printf("Form for each instruction:\n"
           "[cols=\"4*1\",width=\"70%%\"]\n"
           "|=====\n"
           "3+| *Opcode Name* (name-alias, name-alias, ...)"
           GAP "Instruction description."
           GAP "_Word Count_ is the high-order 16 bits of word 0 of the instruction, holding its total <<WordCount,_WordCount_>>. "
               "If the instruction takes a variable number of operands, _Word Count_ also says \"+ variable\", after stating the minimum size of the instruction."
           GAP "_Opcode_ is the low-order 16 bits of word 0 of the instruction, holding its opcode enumerant."
           GAP "_Results_, when present, are any <<ResultId,_Result <id>_ >> or _Result Type_ created by the instruction. "
               "Each _Result <id>_ is always 32 bits."
           GAP "_Operands_, when present, are any literals, other instruction's _Result <id>_, etc., consumed by the instruction. "
               "Each operand is always 32 bits."
           "| <<Capability,Capability>> *Enabling Capabilities* +\n(when needed)\n"
           "| <<WordCount,_Word Count_>> | _Opcode_ | _Results_ | _Operands_\n"
           "|=====\n");

    bool pageBreak = false;
    for (const auto& opClass : InstructionPrintingClasses) {
        if (pageBreak)
            printf("<<<\n"); // page break, except for the first set
        pageBreak = true;
        printf("[[%s]]\n==== %s\n", opClass.tag.c_str(), opClass.heading.c_str());
        PrintOpcodeClass(opClass);
    }
}

// Main function for printing out the documentation.
void PrintDoc()
{
    ParameterizeSpec();

    printf("[[Binary]]\n== Binary Form\n");

    printf("This section contains the exact form for all instructions, starting with the numerical values for all fields. "
           "See <<PhysicalLayout, Physical Layout>> for the order words appear in.\n\n");

    PrintMagicNumber();
    PrintImmediates();
    PrintOpcodes();
}

};  // end namespace spv
