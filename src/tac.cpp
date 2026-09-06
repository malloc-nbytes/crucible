#include "tac.hpp"

#include <exception>
#include <format>
#include <utility>

struct TAC_error : public std::exception {
        std::string msg;

        TAC_error(std::string msg)
                : msg(std::move(msg)) {}

        const char *
        what(void) const noexcept override
        {
                return this->msg.c_str();
        }
};

struct unknown_binary_operator_error : TAC_error {
        token_kind k;

        unknown_binary_operator_error(token_kind k)
                : TAC_error(std::format("cannot derive TAC binary operator from token `%s'",
                                        token_kind_to_cstring(k))),
                k(k) {}
};

static std::string
TAC_binary_operator_to_string(TAC_binary_operator op)
{
        switch (op) {
        case TAC_BINARY_OPERATOR_ADD:           return "add";
        case TAC_BINARY_OPERATOR_SUB:           return "sub";
        case TAC_BINARY_OPERATOR_MUL:           return "mul";
        case TAC_BINARY_OPERATOR_DIV:           return "div";
        case TAC_BINARY_OPERATOR_MOD:           return "mod";
        case TAC_BINARY_OPERATOR_OR:            return "or";
        case TAC_BINARY_OPERATOR_AND:           return "and";
        case TAC_BINARY_OPERATOR_XOR:           return "xor";
        case TAC_BINARY_OPERATOR_LESS:          return "lt";
        case TAC_BINARY_OPERATOR_GREATER:       return "gt";
        case TAC_BINARY_OPERATOR_LESS_EQUAL:    return "le";
        case TAC_BINARY_OPERATOR_GREATER_EQUAL: return "ge";
        case TAC_BINARY_OPERATOR_EQUAL:         return "eq";
        case TAC_BINARY_OPERATOR_NOT_EQUAL:     return "ne";
        }
        std::unreachable();
}

static TAC_binary_operator
token_kind_to_TAC_binary_operator(token_kind k)
{
        switch (k) {
        case TOKEN_KIND_PLUS:               return TAC_BINARY_OPERATOR_ADD;
        case TOKEN_KIND_HYPHEN:             return TAC_BINARY_OPERATOR_SUB;
        case TOKEN_KIND_ASTERISK:           return TAC_BINARY_OPERATOR_MUL;
        case TOKEN_KIND_FORWARD_SLASH:      return TAC_BINARY_OPERATOR_DIV;
        case TOKEN_KIND_PERCENT:            return TAC_BINARY_OPERATOR_MOD;
        case TOKEN_KIND_PIPE:               return TAC_BINARY_OPERATOR_OR;
        case TOKEN_KIND_AMPERSAND:          return TAC_BINARY_OPERATOR_AND;
        case TOKEN_KIND_UPTICK:             return TAC_BINARY_OPERATOR_XOR;
        case TOKEN_KIND_LESSTHAN:           return TAC_BINARY_OPERATOR_LESS;
        case TOKEN_KIND_GREATERTHAN:        return TAC_BINARY_OPERATOR_GREATER;
        case TOKEN_KIND_LESSTHAN_EQUALS:    return TAC_BINARY_OPERATOR_LESS_EQUAL;
        case TOKEN_KIND_GREATERTHAN_EQUALS: return TAC_BINARY_OPERATOR_GREATER_EQUAL;
        case TOKEN_KIND_EQUALS:             return TAC_BINARY_OPERATOR_EQUAL;
        case TOKEN_KIND_BANG_EQUALS:        return TAC_BINARY_OPERATOR_NOT_EQUAL;
        default:                            throw unknown_binary_operator_error(k);
        }
        std::unreachable();
}

static std::string
TAC_operand_to_string(const TAC_operand *const op)
{
        switch (op->k) {
        case TAC_OPERAND_KIND_VOID:   return "void";
        case TAC_OPERAND_KIND_TEMP:   return std::format("%%%d",      ((TAC_operand_temp *)op)->temp);
        case TAC_OPERAND_KIND_PROC:   return std::format("@proc%d",   ((TAC_operand_proc *)op)->sym->id);
        case TAC_OPERAND_KIND_PARAM:  return std::format("%%arg%d",   ((TAC_operand_param *)op)->sym->id);
        case TAC_OPERAND_KIND_LOCAL:  return std::format("%%local%d", ((TAC_operand_local *)op)->sym->id);
        case TAC_OPERAND_KIND_I32:    return std::format("%d",        ((TAC_operand_i32 *)op)->i);
        case TAC_OPERAND_KIND_STRING: return std::format("@str%d",    ((TAC_operand_string *)op)->id);
        }
        std::unreachable();
}

static std::string
TAC_instruction_to_string(const TAC_instruction *const ins)
{
        switch (ins->k) {
        case TAC_INSTRUCTION_KIND_BINOP: {
                const TAC_instruction_binop *const b = (TAC_instruction_binop *)ins;
                return std::format("%%%d = %s %s %s, %s",
                                   b->dst,
                                   TAC_binary_operator_to_string(b->op),
                                   type_to_string(b->ty),
                                   TAC_operand_to_string(b->lhs),
                                   TAC_operand_to_string(b->rhs));
        } break;
        case TAC_INSTRUCTION_KIND_LOAD: {
                const TAC_instruction_load *const l = (TAC_instruction_load *)ins;
                return std::format("%%%d = load %s %s",
                                   l->dst,
                                   type_to_string(l->ty),
                                   TAC_operand_to_string(l->src));
        } break;
        case TAC_INSTRUCTION_KIND_STORE: {
                const TAC_instruction_store *const s = (TAC_instruction_store *)ins;
                return std::format("store %s %s, %s",
                                   type_to_string(s->ty),
                                   TAC_operand_to_string(s->src),
                                   TAC_operand_to_string(s->dst));
        } break;
        }
        std::unreachable();
}

static std::string
TAC_terminator_to_string(const TAC_terminator *const t)
{
        switch (t->k) {
        case TAC_TERMINATOR_KIND_RET: {
                const TAC_terminator_ret *const r = (TAC_terminator_ret *)t;
                if (r->value == std::nullopt)
                        return "ret";
                return std::format("ret %s", TAC_operand_to_string(r->value.value()));
        } break;
        case TAC_TERMINATOR_KIND_JMP: {
                const TAC_terminator_jmp *const j = (TAC_terminator_jmp *)t;
                return std::format("jmp L%d", j->label);
        } break;
        case TAC_TERMINATOR_KIND_BRANCH: {
                const TAC_terminator_branch *const b = (TAC_terminator_branch *)t;
                return std::format("branch %s, L%d, L%d",
                                   TAC_operand_to_string(b->cond),
                                   b->if_true,
                                   b->if_false);
        } break;
        }
}

static std::string
TAC_block_to_string(const TAC_block *const blk)
{
        std::string res;

        res = std::format("L%d:\n", blk->label);

        for (size_t i = 0; i < blk->instructions.size(); ++i) {
                if (i != 0)
                        res += "\n";
                res += "  " + TAC_instruction_to_string(blk->instructions.at(i));
        }

        res += TAC_terminator_to_string(blk->terminator);

        return res;
}

static std::string
TAC_linkage_to_string(TAC_linkage l)
{
        switch (l) {
        case TAC_LINKAGE_INTERNAL:      return "";
        case TAC_LINKAGE_EXPORT:        return "export ";
        case TAC_LINKAGE_EXTERN:        return "extern ";
        case TAC_LINKAGE_EXTERN_EXPORT: return "export extern ";
        }
}

static inline
int is_extern(TAC_linkage l)
{
        return l >= TAC_LINKAGE_EXTERN;
}

static std::string
TAC_proc_to_string(const TAC_proc *const p)
{
        std::string linkage;
        std::string name;
        std::string params;
        std::string variadic;
        std::string return_type;
        std::string blocks;

        linkage = TAC_linkage_to_string(p->linkage);
        name    = p->sym->name;

        for (size_t i = 0; i < p->params.size(); ++i) {
                if (i != 0)
                        params += ", ";
                const symbol *const sym = p->params.at(i);
                params += std::format("%%arg%d: %s",
                                      sym->id, type_to_string(sym->ty));
        }

        variadic = ((type_proc *)p)->variadic ? "..." : "";

        return_type = type_to_string(p->return_type);

        for (size_t i = 0; i < p->blocks.size(); ++i) {
                if (i != 0)
                        blocks += "\n";
                blocks += TAC_block_to_string(p->blocks.at(i));
        }

        return linkage + "proc " + name + "(" + params + variadic + ": " + return_type + "{\n" + blocks + "}";
}

static std::string
TAC_string_value_to_string(const std::string &value)
{
        std::string result;

        for (size_t i = 0; i < value.size(); ++i) {
                const unsigned char c = value.at(i);
                switch (c) {
                case '\b': result += "\\b"; break;
                case '\t': result += "\\t"; break;
                case '\n': result += "\\n"; break;
                case '\r': result += "\\r"; break;
                case '"':  result += "\\\""; break;
                case '\\': result += "\\\\"; break;
                default:
                        if (c < 32 || c >= 127)
                                result += std::format("\\{:03d}", c);
                        else
                                result += c;
                        break;
                }
        }

        return result;
}

std::string
TAC_program_to_string(const TAC_program *const p)
{
        std::string result;
        bool first = true;

        for (size_t i = 0; i < p->strings.size(); ++i) {
                const TAC_string &s = p->strings.at(i);
                if (!first)
                        result += "\n\n";
                result += std::format("@str{} = \"{}\"",
                                      s.id, TAC_string_value_to_string(s.value));
                first = false;
        }

        for (size_t i = 0; i < p->procs.size(); ++i) {
                const TAC_proc &proc = p->procs.at(i);
                if (!first)
                        result += "\n\n";
                result += TAC_proc_to_string(&proc);
                first = false;
        }

        return result;
}
