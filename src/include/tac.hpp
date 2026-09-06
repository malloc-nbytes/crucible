#ifndef TAC_HPP_INCLUDED
#define TAC_HPP_INCLUDED

#include "symbol.hpp"

#include <vector>
#include <optional>

using   TEMP      = int;
using   LABEL     = int;
using   STRING_ID = int;

typedef enum {
        TAC_OPERAND_KIND_VOID = 0,
        TAC_OPERAND_KIND_TEMP,
        TAC_OPERAND_KIND_PROC,
        TAC_OPERAND_KIND_PARAM,
        TAC_OPERAND_KIND_LOCAL,
        TAC_OPERAND_KIND_I32,
        TAC_OPERAND_KIND_STRING,
} TAC_operand_kind;

typedef struct { TAC_operand_kind k; } TAC_operand;

typedef struct {
        TAC_operand base;
} TAC_operand_void;

typedef struct {
        TAC_operand     base;
        TEMP            temp;
} TAC_operand_temp;

typedef struct {
        TAC_operand      base;
        symbol          *sym;
} TAC_operand_proc;

typedef struct {
        TAC_operand      base;
        symbol          *sym;
} TAC_operand_param;

typedef struct {
        TAC_operand      base;
        symbol          *sym;
} TAC_operand_local;

typedef struct {
        TAC_operand     base;
        int             i;
} TAC_operand_i32;

typedef struct {
        TAC_operand     base;
        STRING_ID       id;
} TAC_operand_string;

typedef enum {
        TAC_BINARY_OPERATOR_ADD = 0,
        TAC_BINARY_OPERATOR_SUB,
        TAC_BINARY_OPERATOR_MUL,
        TAC_BINARY_OPERATOR_DIV,
        TAC_BINARY_OPERATOR_MOD,
        TAC_BINARY_OPERATOR_OR,
        TAC_BINARY_OPERATOR_AND,
        TAC_BINARY_OPERATOR_XOR,
        TAC_BINARY_OPERATOR_LESS,
        TAC_BINARY_OPERATOR_GREATER,
        TAC_BINARY_OPERATOR_LESS_EQUAL,
        TAC_BINARY_OPERATOR_GREATER_EQUAL,
        TAC_BINARY_OPERATOR_EQUAL,
        TAC_BINARY_OPERATOR_NOT_EQUAL,
} TAC_binary_operator;

typedef enum {
        TAC_INSTRUCTION_KIND_BINOP = 0,
        TAC_INSTRUCTION_KIND_LOAD,
        TAC_INSTRUCTION_KIND_STORE,
} TAC_instruction_kind;

typedef struct { TAC_instruction_kind k; } TAC_instruction;

typedef struct {
        TAC_instruction          base;
        TEMP                     dst;
        type                    *ty;
        type                    *result_ty;
        TAC_binary_operator      op;
        TAC_operand             *lhs;
        TAC_operand             *rhs;
} TAC_instruction_binop;

typedef struct {
        TAC_instruction  base;
        TEMP             dst;
        type            *ty;
        TAC_operand     *src;
} TAC_instruction_load;

typedef struct {
        TAC_instruction  base;
        type            *ty;
        TAC_operand     *src;
        TAC_operand     *dst;
} TAC_instruction_store;

typedef enum {
        TAC_TERMINATOR_KIND_RET = 0,
        TAC_TERMINATOR_KIND_JMP,
        TAC_TERMINATOR_KIND_BRANCH,
} TAC_terminator_kind;

typedef struct { TAC_terminator_kind k; } TAC_terminator;

typedef struct {
        TAC_terminator                  base;
        std::optional<TAC_operand*>     value;
} TAC_terminator_ret;

typedef struct {
        TAC_terminator  base;
        LABEL           label;
} TAC_terminator_jmp;

typedef struct {
        TAC_terminator   base;
        TAC_operand     *cond;
        LABEL            if_true;
        LABEL            if_false;
} TAC_terminator_branch;

typedef struct {
        LABEL                            label;
        std::vector<TAC_instruction *>   instructions;
        TAC_terminator                  *terminator;
} TAC_block;

typedef enum {
        TAC_LINKAGE_INTERNAL = 0,
        TAC_LINKAGE_EXPORT,
        TAC_LINKAGE_EXTERN, // must be here
        TAC_LINKAGE_EXTERN_EXPORT, // must be here
} TAC_linkage;

typedef struct {
        symbol                  *sym;
        TAC_linkage              linkage;
        std::vector<symbol *>    params;
        type                    *return_type;
        std::optional<LABEL>     entry;
        std::vector<TAC_block *> blocks;
} TAC_proc;

typedef struct {
        STRING_ID       id;
        std::string     value;
} TAC_string;

typedef struct {
        std::vector<TAC_string> strings;
        std::vector<TAC_proc>   procs;
} TAC_program;

std::string TAC_program_to_string(const TAC_program *const p);

#endif // TAC_HPP_INCLUDED
