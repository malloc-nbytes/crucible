#include "lower.hpp"
#include "grammar.hpp"

#include <cassert>
#include <exception>
#include <utility>
#include <format>

struct lowering_error : public std::exception {
        std::string msg;

        lowering_error(std::string msg)
                : msg(std::move(msg)) {}

        const char *
        what(void) const noexcept override
        {
                return this->msg.c_str();
        }
};

struct lowering_unresolved_error : lowering_error {
        lowering_unresolved_error(std::string msg)
                : lowering_error(msg) {}
};

struct invariant_violated_error : lowering_error {
        invariant_violated_error(std::string msg)
                : lowering_error(std::format("lowering invariant violated: ", msg)) {}
};

struct emit_after_terminated_TAC_block_error : lowering_error {
        emit_after_terminated_TAC_block_error()
                : lowering_error("cannot emit an instruction into a terminated TAC block") {}
};

struct start_TAC_block_before_terminating_previous_error : lowering_error {
        start_TAC_block_before_terminating_previous_error()
                : lowering_error("cannot start TAC block before terminating the current block") {}
};

struct double_terminated_TAC_block_error : lowering_error {
        double_terminated_TAC_block_error()
                : lowering_error("cannot terminate already terminated TAC block") {}
};

static inline void
fail_unresolved(std::string &what)
{
        throw lowering_unresolved_error(std::format("lowering invariant violated: unresolved %s", what));
}

static symbol *
require_symbol(std::optional<symbol *> sym, std::string what)
{
        if (sym == std::nullopt)
                fail_unresolved(what);
        return sym.value();
}

static inline int
new_temp(builder *bldr)
{
        return bldr->next_temp++;
}

static inline int
new_label(builder *bldr)
{
        return bldr->next_label++;
}

static void
emit(builder *bldr, TAC_instruction *instr)
{
        if (!bldr->current_open)
                throw emit_after_terminated_TAC_block_error();
        bldr->current_instrs.push_back(instr);
}

static void
start_block(builder *bldr, int label)
{
        if (!bldr->current_open)
                throw start_TAC_block_before_terminating_previous_error();

        bldr->current_label  = label;
        bldr->current_instrs = {};
        bldr->current_open   = 1;
}

static void
end_block(builder *bldr, TAC_terminator *term)
{
        if (!bldr->current_open)
                throw double_terminated_TAC_block_error();

        TAC_block blk = TAC_block {
                .label        = bldr->current_label,
                .instructions = bldr->current_instrs,
                .terminator   = term,
        };

        bldr->completed_blocks.push_back(blk);

        bldr->current_instrs = {};
        bldr->current_open   = 0;
}

static STRING_ID
intern_string(program_state *program, std::string &value)
{
        auto it = program->string_ids.find(value);

        if (it != program->string_ids.end())
                return it->second;

        STRING_ID id = program->next_string_id++;

        program->string_ids.insert({value, id});
        program->strings.push_back(TAC_string { .id = id, .value = value });

        return id;
}

static void
bind_symbol(builder *bldr, symbol *sym, TAC_operand *op)
{
        bldr->bindings[sym->id] = op;
}

static TAC_operand *
lookup_symbol(builder *bldr, symbol *sym)
{
        auto it = bldr->bindings.find(sym->id);

        if (it != bldr->bindings.end())
                return it->second;

        throw invariant_violated_error(std::format("symbol %s has no TAC binding", sym->name));

        std::unreachable();
}

static type *
require_resolved_type(expr *e)
{
        if (e->ty->k == TYPE_KIND_UNDEFINED)
                throw invariant_violated_error(std::format("expression type is undefined"));
        return e->ty;
}

static TAC_operand *
lower_identifier(builder *bldr, expr_identifier *e)
{
        symbol *sym;

        sym = require_symbol(e->sym, "identifier");

        switch (sym->k) {
        case SYMBOL_KIND_PROC: {
                return (TAC_operand *)new TAC_operand_proc {
                        .base = { .k = TAC_OPERAND_KIND_PROC, },
                        .sym  = sym,
                };
        } break;
        case SYMBOL_KIND_LOCAL:
        case SYMBOL_KIND_PARAM: {
                TAC_operand *src = lookup_symbol(bldr, sym);
                // if (e->ty->k == TYPE_KIND_ARRAY || e->ty->k == TYPE_KIND_STRUCT) {
                //         return src;
                // }
                int dst = new_temp(bldr);
                emit(bldr, (TAC_instruction *)new TAC_instruction_load {
                                .base = {
                                        .k = TAC_INSTRUCTION_KIND_LOAD,
                                },
                                .dst = dst,
                                .ty  = e->base.ty,
                                .src = src
                        });
                return (TAC_operand *)new TAC_operand_temp {
                        .base = { .k = TAC_OPERAND_KIND_TEMP, },
                        .temp = dst,
                };
        } break;
        }
        std::unreachable();
}

static TAC_operand *
lower_assignmnet_target(builder *bldr, expr *e)
{
        if (e->k != EXPR_KIND_IDENTIFIER)
                throw invariant_violated_error("assignment target is not an identifier");

        return lookup_symbol(bldr, ((expr_identifier *)e)->sym);
}


static TAC_operand *
lower_expr(builder *bldr, expr *e)
{
        switch (e->k) {
        case EXPR_KIND_INT:
                return (TAC_operand *)new TAC_operand_i32 {
                        .base = { .k = TAC_OPERAND_KIND_I32, },
                        .i = std::stoi(((expr_int *)e)->i->lx),
                };
        case EXPR_KIND_IDENTIFIER: return lower_identifier(bldr, (expr_identifier *)e);
        case EXPR_KIND_BINARY: {
                return NULL;
        } break;
        case EXPR_KIND_UNARY:
                assert(0 && "todo");
                return NULL;
        }
        std::unreachable();
}
