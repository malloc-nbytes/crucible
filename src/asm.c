#include "asm.h"
#include "visitor.h"
#include "global.h"
#include "types.h"
#include "lexer.h"

#include <forge/err.h>
#include <forge/utils.h>
#include <forge/cstr.h>
#include <forge/str.h>
#include <forge/array.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>

/*
calling order: rdi, rsi, rdx, rcx, r8, r9.

8-byte register Bytes 0-3 Bytes 0-1 Byte 0
  %rax %eax %ax %al
  %rcx %ecx %cx %cl
  %rdx %edx %dx %dl
  %rbx %ebx %bx %bl
  %rsi %esi %si %sil
  %rdi %edi %di %dil
  %rsp %esp %sp %spl
  %rbp %ebp %bp %bpl
  %r8 %r8d %r8w %r8b
  %r9 %r9d %r9w %r9b
  %r10 %r10d %r10w %r10b
  %r11 %r11d %r11w %r11b
  %r12 %r12d %r12w %r12b
  %r13 %r13d %r13w %r13b
  %r14 %r14d %r14w %r14b
  %r15 %r15d %r15w %r15b
 */

#define REGAT(i, j, mat) mat[(i) * g_regs_c + (j)]

static char *g_regs[] = {
        "rcx", "ecx",  "cx",   "cl",
        "rdx", "edx",  "dx",   "dl",
        "rsi", "esi",  "si",   "sil",
        "rdi", "edi",  "di",   "dil",
        "r8",  "r8d",  "r8w",  "r8b",
        "r9",  "r9d",  "r9w",  "r9b",
        "r10", "r10d", "r10w", "r10b",
        "r11", "r11d", "r11w", "r11b",
};

const size_t g_regs_n = sizeof(g_regs)/sizeof(*g_regs);
#define g_regs_r 8
#define g_regs_c 4

static int g_inuse_regs[g_regs_r * g_regs_c] = {
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
};

typedef struct {
        FILE *out;
        str_array globals;
        str_array data_section;
        str_array externs;
        str_array pushed_regs;
} asm_context;

static void
write_txt(asm_context *ctx,
          const char  *txt,
          int          newline)
{
        assert(ctx->out);

        fwrite(txt, 1, strlen(txt), ctx->out);
        if (newline) fwrite("\n", 1, 1, ctx->out);
}

static void
take_txt(asm_context *ctx,
         char        *txt,
         int          newline)
{
        assert(ctx->out);

        fwrite(txt, 1, strlen(txt), ctx->out);

        if (newline) fwrite("\n", 1, 1, ctx->out);

        free(txt);
}

static const char *
get_reg_from_size(const char *reg64, int sz)
{
        char *err = NULL;

        if (!strcmp(reg64, "rax")) {
                switch (sz) {
                case 8: return "rax";
                case 4: return "eax";
                case 2: return "ax";
                case 1: return "al";
                default: err = "invalid size"; goto bad;
                }
        }

        for (size_t i = 0; i < g_regs_r; ++i) {
                if (!strcmp(REGAT(i, 0, g_regs), reg64)) {
                        switch (sz) {
                        case 8: return REGAT(i, 0, g_regs);
                        case 4: return REGAT(i, 1, g_regs);
                        case 2: return REGAT(i, 2, g_regs);
                        case 1: return REGAT(i, 3, g_regs);
                        default: err = "invalid size"; goto bad;
                        }
                }
        }

        err = "no available registers";
 bad:
        forge_err_wargs("get_reg_from_size(): could not get (%s) register of size %d: %s",
                        reg64, sz, err);
        return NULL; // unreachable
}

static char *
genlbl(void)
{
        static int i = 0;
        char buf[256] = {0};
        sprintf(buf, "t%d", i);
        ++i;
        return strdup(buf);
}

/* static int */
/* lvalue(asm_context *ctx, expr *e) */
/* { */
/*         switch (e->kind) { */
/*         case EXPR_KIND_IDENTIFIER: { */
/*                 write_txt(ctx, "sub rbp, 4", 1); */
/*         } break; */
/*         default: { */
/*                 forge_err_wargs("lvalue(): the lvalue of `%d` is unimplemented", */
/*                                 (int)e->kind); */
/*         } break; */
/*         } */

/*         return NULL; // unreachable */
/* } */

char *
int_to_cstr(int i)
{
        int digits = 0;
        if (i < 10) digits = 1;
        if (i < 100) digits = 2;
        if (i < 1000) digits = 3;
        if (i < 10000) digits = 4;
        if (i < 100000) digits = 5;
        if (i < 1000000) digits = 6;
        if (i < 10000000) digits = 7;
        if (i < 100000000) digits = 8;
        if (i < 1000000000) digits = 9;
        else digits = 10;

        char *s = (char *)malloc(digits + 1);
        sprintf(s, "%d", i);
        s[digits-1] = 0;
        return s;
}

static void
free_reg_literal(const char *reg)
{
        for (size_t i = 0; i < g_regs_n; ++i) {
                if (!strcmp(reg, g_regs[i])) {
                        assert(g_inuse_regs[i]);
                        g_inuse_regs[i] = 0;
                        return;
                }
        }

        //forge_err_wargs("could not free register %s, was not alloc'd", reg);
}

static int
alloc_reg(int sz)
{
        for (size_t i = 0; i < g_regs_r; ++i) {
                int ok = 1;
                for (size_t j = 0; j < g_regs_c; ++j) {
                        if (REGAT(i, j, g_inuse_regs)) {
                                ok = 0;
                                break;
                        }
                }
                if (!ok) continue;
                switch (sz) {
                case 8:
                        REGAT(i, 0, g_inuse_regs) = 1;
                        return i * g_regs_c + 0;
                case 4:
                        REGAT(i, 1, g_inuse_regs) = 1;
                        return i * g_regs_c + 1;
                case 2:
                        REGAT(i, 2, g_inuse_regs) = 1;
                        return i * g_regs_c + 2;
                case 1:
                        REGAT(i, 3, g_inuse_regs) = 1;
                        return i * g_regs_c + 3;
                default: forge_err_wargs("alloc_reg(): cannot alloc register with size %d", sz);
                }
        }
        forge_err("alloc_reg(): no more registers");
}

static int
alloc_param_regs(int sz)
{
        /* static const char *param_regs[] = { */
        /*         "rdi", "edi", "di", "dil",  // Index 12-15 */
        /*         "rsi", "esi", "si", "sil",  // Index 8-11 */
        /*         "rdx", "edx", "dx", "dl",   // Index 4-7 */
        /*         "rcx", "ecx", "cx", "cl",   // Index 0-3 */
        /*         "r8",  "r8d", "r8w", "r8b", // Index 16-19 */
        /*         "r9",  "r9d", "r9w", "r9b"  // Index 20-23 */
        /* }; */

        static const size_t param_reg_indices[] = {12, 8, 4, 0, 16, 20};
        static const size_t param_reg_count = sizeof(param_reg_indices)/sizeof(*param_reg_indices);
        int col;

        switch (sz) {
        case 8: col = 0; break;
        case 4: col = 1; break;
        case 2: col = 2; break;
        case 1: col = 3; break;
        default: forge_err_wargs("alloc_param_regs(): cannot allocate register with size %d", sz);
        }

        for (size_t i = 0; i < param_reg_count; ++i) {
                size_t base_idx = param_reg_indices[i];
                int row = base_idx / g_regs_c;
                if (!REGAT(row, col, g_inuse_regs)) {
                        int reg_free = 1;
                        for (size_t j = 0; j < g_regs_c; ++j) {
                                if (REGAT(row, j, g_inuse_regs)) {
                                        reg_free = 0;
                                        break;
                                }
                        }
                        if (reg_free) {
                                REGAT(row, col, g_inuse_regs) = 1;
                                return base_idx + col;
                        }
                }
        }

        forge_err("alloc_param_regs(): no available parameter registers");
}

static void
free_reg(int reg)
{
        g_inuse_regs[reg] = 0;
}

static void
push_inuse_regs(asm_context *ctx)
{
        for (size_t i = 0; i < g_regs_r; ++i) {
                for (size_t j = 0; j < g_regs_c; ++j) {
                        if (REGAT(i, j, g_inuse_regs)) {
                                char *reg = REGAT(i, 0, g_regs);
                                take_txt(ctx, forge_cstr_builder("push ", reg, NULL), 1);
                                dyn_array_append(ctx->pushed_regs, reg);
                        }
                }
        }
}

static void
pop_regs(asm_context *ctx)
{
        for (size_t i = 0; i < ctx->pushed_regs.len; ++i) {
                take_txt(ctx, forge_cstr_builder("pop ", ctx->pushed_regs.data[i], NULL), 1);
        }
        dyn_array_clear(ctx->pushed_regs);
}

static const char *
szspec(int sz)
{
        switch (sz) {
        case 8: return "QWORD";
        case 4: return "DWORD";
        case 2: return "WORD";
        case 1: return "BYTE";
        default: forge_err_wargs("get_size_specifier(): cannot get size specfifier for size %d", sz);
        }
        return NULL; // unreachable
}

static void
prologue(asm_context *ctx, int rsp_n)
{
        // Next multiple of 16.
        int aligned_rsp_n = (rsp_n + 15) & ~15;
        char rsp[1024] = {0};
        sprintf(rsp, "%d", aligned_rsp_n);

        write_txt(ctx, "push rbp", 1);
        write_txt(ctx, "mov rbp, rsp", 1);
        take_txt(ctx, forge_cstr_builder("sub rsp, ", rsp, NULL), 1);
}

static void
epilogue(asm_context *ctx)
{
        write_txt(ctx, "leave", 1);
        write_txt(ctx, "ret", 1);
}

static void *
visit_expr_bin(visitor *v, expr_bin *e)
{
        asm_context *ctx = (asm_context *)v->context;

        char *v1 = e->lhs->accept(e->lhs, v);
        char *v2 = e->rhs->accept(e->rhs, v);

        const char *spec = szspec(type_to_int(e->lhs->type));

        int regi = alloc_reg(type_to_int(e->lhs->type));
        char *reg = g_regs[regi];

        take_txt(ctx, forge_cstr_builder("mov ", spec, " ", reg, ", ", v1, NULL), 1);

        switch (e->op->ty) {
        case TOKEN_TYPE_PLUS:
                take_txt(ctx, forge_cstr_builder("add ", spec, " ", reg, ", ", v2, NULL), 1);
                break;
        case TOKEN_TYPE_MINUS:
                take_txt(ctx, forge_cstr_builder("sub ", spec, " ", reg, ", ", v2, NULL), 1);
                break;
        case TOKEN_TYPE_ASTERISK:
                take_txt(ctx, forge_cstr_builder("imul ", spec, " ", reg, ", ", v2, NULL), 1);
                break;
        case TOKEN_TYPE_FORWARDSLASH: {
                // Note: Division requires rax and rdx
                write_txt(ctx, "xor rdx, rdx", 1); // Clear (for dividend)
                take_txt(ctx, forge_cstr_builder("mov ", spec, " rax, ", v1, NULL), 1);
                take_txt(ctx, forge_cstr_builder("idiv ", spec, " ", v2, NULL), 1);
                take_txt(ctx, forge_cstr_builder("mov ", spec, " ", reg, ", rax", NULL), 1);
        } break;
        case TOKEN_TYPE_PERCENT: {
                // Note: Modulo requires rax and rdx
                write_txt(ctx, "xor rdx, rdx", 1); // Clear (for dividend)
                take_txt(ctx, forge_cstr_builder("mov ", spec, " rax, ", v1, NULL), 1);
                take_txt(ctx, forge_cstr_builder("idiv ", spec, " ", v2, NULL), 1);
                take_txt(ctx, forge_cstr_builder("mov ", spec, " ", reg, ", rdx", NULL), 1);
        } break;
        case TOKEN_TYPE_DOUBLE_EQUALS:
        case TOKEN_TYPE_BANG_EQUALS:
        case TOKEN_TYPE_LESSTHAN:
        case TOKEN_TYPE_GREATERTHAN:
        case TOKEN_TYPE_LESSTHAN_EQUALS:
        case TOKEN_TYPE_GREATERTHAN_EQUALS: {
                char *lbl_true = genlbl();
                char *lbl_done = genlbl();
                const char *cmp_op = NULL;
                switch (e->op->ty) {
                case TOKEN_TYPE_DOUBLE_EQUALS:      cmp_op = "je";  break;
                case TOKEN_TYPE_BANG_EQUALS:        cmp_op = "jne"; break;
                case TOKEN_TYPE_LESSTHAN:           cmp_op = "jl";  break;
                case TOKEN_TYPE_GREATERTHAN:        cmp_op = "jg";  break;
                case TOKEN_TYPE_LESSTHAN_EQUALS:    cmp_op = "jle"; break;
                case TOKEN_TYPE_GREATERTHAN_EQUALS: cmp_op = "jge"; break;
                default: forge_err_wargs("unimplemented comparison op `%s`", e->op->lx);
                }
                take_txt(ctx, forge_cstr_builder("cmp ", spec, " ", reg, ", ", v2, NULL), 1);
                take_txt(ctx, forge_cstr_builder(cmp_op, " ", lbl_true, NULL), 1);
                take_txt(ctx, forge_cstr_builder("mov ", spec, " ", reg, ", 0", NULL), 1);
                take_txt(ctx, forge_cstr_builder("jmp ", lbl_done, NULL), 1);
                take_txt(ctx, forge_cstr_builder(lbl_true, ":", NULL), 1);
                take_txt(ctx, forge_cstr_builder("mov ", spec, " ", reg, ", 1", NULL), 1);
                take_txt(ctx, forge_cstr_builder(lbl_done, ":", NULL), 1);

                free(lbl_true);
                free(lbl_done);
        } break;
        case TOKEN_TYPE_DOUBLE_AMPERSAND: {
                char *lbl_false = genlbl();
                char *lbl_done = genlbl();

                // lhs
                take_txt(ctx, forge_cstr_builder("cmp ", spec, " ", reg, ", 0", NULL), 1);
                take_txt(ctx, forge_cstr_builder("je ", lbl_false, NULL), 1);

                // rhs
                take_txt(ctx, forge_cstr_builder("cmp ", spec, " ", v2, ", 0", NULL), 1);
                take_txt(ctx, forge_cstr_builder("je ", lbl_false, NULL), 1);

                // Both true
                take_txt(ctx, forge_cstr_builder("mov ", spec, " ", reg, ", 1", NULL), 1);
                take_txt(ctx, forge_cstr_builder("jmp ", lbl_done, NULL), 1);
                take_txt(ctx, forge_cstr_builder(lbl_false, ":", NULL), 1);
                take_txt(ctx, forge_cstr_builder("mov ", spec, " ", reg, ", 0", NULL), 1);
                take_txt(ctx, forge_cstr_builder(lbl_done, ":", NULL), 1);

                free(lbl_false);
                free(lbl_done);
        } break;
        case TOKEN_TYPE_DOUBLE_PIPE: {
                char *lbl_true = genlbl();
                char *lbl_done = genlbl();

                // lhs
                take_txt(ctx, forge_cstr_builder("cmp ", spec, " ", reg, ", 0", NULL), 1);
                take_txt(ctx, forge_cstr_builder("jne ", lbl_true, NULL), 1);

                // rhs
                take_txt(ctx, forge_cstr_builder("cmp ", spec, " ", v2, ", 0", NULL), 1);
                take_txt(ctx, forge_cstr_builder("jne ", lbl_true, NULL), 1);

                // Both false
                take_txt(ctx, forge_cstr_builder("mov ", spec, " ", reg, ", 0", NULL), 1);
                take_txt(ctx, forge_cstr_builder("jmp ", lbl_done, NULL), 1);
                take_txt(ctx, forge_cstr_builder(lbl_true, ":", NULL), 1);
                take_txt(ctx, forge_cstr_builder("mov ", spec, " ", reg, ", 1", NULL), 1);
                take_txt(ctx, forge_cstr_builder(lbl_done, ":", NULL), 1);

                free(lbl_true);
                free(lbl_done);
        } break;
        default: forge_err_wargs("unimplemented binop `%s`", e->op->lx);
        }

        free_reg_literal(v1);
        free_reg_literal(v2);

        return reg;
}

static void *
visit_expr_identifier(visitor *v, expr_identifier *e)
{
        asm_context *ctx = (asm_context *)v->context;

        assert(e->resolved);

        int regi = alloc_reg(type_to_int(e->resolved->ty));
        char *reg = g_regs[regi];

        if (e->resolved->extern_ || ((expr *)e)->type->kind == TYPE_KIND_PROC) {
                take_txt(ctx, forge_cstr_builder("mov ", reg, ", ", e->id->lx, NULL), 1);
        } else {
                char *offset_s = int_to_cstr(e->resolved->stack_offset);
                const char *spec = szspec(type_to_int(e->resolved->ty));
                take_txt(ctx, forge_cstr_builder("mov ", spec, " ", reg, ", [rbp-", offset_s, "]", NULL), 1);
        }

        return reg;
}

static void *
visit_expr_integer_literal(visitor *v, expr_integer_literal *e)
{
        NOOP(v);
        return e->i->lx;
}

static void *
visit_expr_string_literal(visitor *v, expr_string_literal *e)
{
        // TODO: handle escape sequences
        asm_context *ctx = (asm_context *)v->context;


        char *lbl = genlbl();
        forge_str out = forge_str_create();
        forge_str_concat(&out, lbl);
        forge_str_concat(&out, ": db ");
        const char *actual = e->s->lx;
        int alpha = 0;

        for (size_t i = 0; actual[i]; ++i) {
                if (actual[i] == '\n' || actual[i] == '\t') {
                        if (alpha) forge_str_concat(&out, "\"");
                        if (i != 0) forge_str_concat(&out, ", ");

                        switch (actual[i]) {
                        case '\n': forge_str_concat(&out, " 10 "); break;
                        case '\t': forge_str_concat(&out, " 9 ");  break;
                        default:   assert(0);
                        }

                        alpha = 0;
                } else {
                        if (!alpha) {
                                if (i != 0) forge_str_concat(&out, ", ");
                                forge_str_concat(&out, "\"");
                        }
                        forge_str_append(&out, actual[i]);
                        alpha = 1;
                }
        }

        if (alpha) {
                forge_str_append(&out, '"');
        }
        forge_str_concat(&out, ", 0");

        dyn_array_append(ctx->data_section, out.data);
        return lbl;
}

static void *
visit_expr_proccall(visitor *v, expr_proccall *e)
{
        assert(e->args.len <= 6 && "only 6 procedure arguments are supported right now");

        asm_context *ctx = (asm_context *)v->context;

        push_inuse_regs(ctx);

        int_array pregs = dyn_array_empty(int_array);

        for (size_t i = 0; i < e->args.len; ++i) {
                expr *arg = e->args.data[i];
                char *value = arg->accept(arg, v);

                int pregi = alloc_param_regs(type_to_int(arg->type));
                char *preg = g_regs[pregi];
                const char *spec = szspec(type_to_int(arg->type));

                dyn_array_append(pregs, pregi);

                take_txt(ctx, forge_cstr_builder("mov ", spec, " ", preg, ", ", value, NULL), 1);

                free_reg_literal(value);
        }

        assert(e->lhs->type->kind == TYPE_KIND_PROC);
        type_proc *proc_ty = (type_proc *)e->lhs->type;
        if (proc_ty->variadic) {
                write_txt(ctx, "xor rax, rax", 1);
        }

        char *callee = e->lhs->accept(e->lhs, v);

        for (size_t i = 0; i < pregs.len; ++i) {
                free_reg(pregs.data[i]);
        } dyn_array_free(pregs);

        take_txt(ctx, forge_cstr_builder("call ", callee, NULL), 1);
        free_reg_literal(callee);
        pop_regs(ctx);

        return (void *)get_reg_from_size("rax", type_to_int(proc_ty->rettype));
}

static void *
visit_expr_mut(visitor *v, expr_mut *e)
{
        asm_context *ctx = (asm_context *)v->context;

        switch (e->lhs->kind) {
        case EXPR_KIND_IDENTIFIER: {
                sym *sym = ((expr_identifier *)e->lhs)->resolved;
                assert(sym);

                const char *spec = szspec(type_to_int(sym->ty));
                const char *offset = int_to_cstr(sym->stack_offset);

                char *rvalue = e->rhs->accept(e->rhs, v);
                take_txt(ctx, forge_cstr_builder("mov ", spec, " [rbp-", offset, "], ", rvalue, NULL), 1);

                return rvalue;
        } break;
        default: {
                forge_err_wargs("lvalue(): the lvalue of `%d` is unimplemented",
                                (int)e->lhs->kind);
        } break;
        }

        return NULL; // unreachable
}

static void *
visit_stmt_let(visitor *v, stmt_let *s)
{
        asm_context *ctx = (asm_context *)v->context;

        char *value = (char *)s->e->accept(s->e, v);

        int offset = s->resolved->stack_offset;
        char *offset_s = int_to_cstr(offset);
        const char *spec = szspec(type_to_int(s->e->type));

        take_txt(ctx, forge_cstr_builder("mov ", spec, " [rbp-", offset_s, "], ", value, NULL), 1);
        free(offset_s);

        free_reg_literal(value);

        return NULL;
}

static void *
visit_stmt_expr(visitor *v, stmt_expr *s)
{
        char *value = s->e->accept(s->e, v);
        free_reg_literal(value);

        return NULL;
}

static void *
visit_stmt_block(visitor *v, stmt_block *s)
{
        for (size_t i = 0; i < s->stmts.len; ++i) {
                stmt *stmt = s->stmts.data[i];
                stmt->accept(stmt, v);
        }
        return NULL;
}

static void *
visit_stmt_proc(visitor *v, stmt_proc *s)
{
        asm_context *ctx = (asm_context *)v->context;

        if (s->export) {
                dyn_array_append(ctx->globals, s->id->lx);
        }

        take_txt(ctx, forge_cstr_builder(s->id->lx, ":", NULL), 1);
        prologue(ctx, s->rsp);

        const char *param_regs[] = { "rdi", "rsi", "rdx", "rcx", "r8", "r9" };

        // Put procedure parameters onto the stack.
        for (size_t i = 0; i < s->params.len; ++i) {
                sym *param = s->params.data[i].resolved;
                assert(param);
                int sz = type_to_int(param->ty);
                char *offset = int_to_cstr(param->stack_offset);
                take_txt(ctx, forge_cstr_builder("mov ", szspec(sz), " [rbp-", offset, "], ",
                                                 get_reg_from_size(param_regs[i], sz), NULL),
                         1);
        }

        // TODO: procedure parameters
        s->blk->accept(s->blk, v);

        epilogue(ctx);
        return NULL;
}

static void *
visit_stmt_return(visitor *v, stmt_return *s)
{
        asm_context *ctx = (asm_context *)v->context;

        char *value = s->e->accept(s->e, v);
        int sz = type_to_int(s->e->type);
        const char *ret_reg = get_reg_from_size("rax", sz);
        take_txt(ctx, forge_cstr_builder("mov ", szspec(sz), " ",
                                         ret_reg, ", ",
                                         value, NULL), 1);
        free_reg_literal(value);
        write_txt(ctx, "leave", 1);
        write_txt(ctx, "ret", 1);
        return (void *)ret_reg;
}

static void *
visit_stmt_exit(visitor *v, stmt_exit *s)
{
        asm_context *ctx = (asm_context *)v->context;

        char *reg = NULL;

        if (s->e) {
                reg = s->e->accept(s->e, v);
        }

        if (reg) {
                const char *syscall_reg = get_reg_from_size("rdi", type_to_int(s->e->type));
                const char *spec = szspec(type_to_int(s->e->type));
                take_txt(ctx, forge_cstr_builder("mov ", spec, " ", syscall_reg, ", ", reg, NULL), 1);
                write_txt(ctx, "mov rax, 60", 1);
                free_reg_literal(reg);
        } else {
                write_txt(ctx, "mov rax, 60", 1);
                write_txt(ctx, "mov rdi, 0", 1);
        }

        write_txt(ctx, "syscall", 1);

        return NULL;
}

static void *
visit_stmt_extern_proc(visitor *v, stmt_extern_proc *s)
{
        asm_context *ctx = (asm_context *)v->context;
        dyn_array_append(ctx->externs, forge_cstr_builder("extern ", s->id->lx, NULL));
        return NULL;
}

static void *
visit_stmt_if(visitor *v, stmt_if *s)
{
        asm_context *ctx = (asm_context *)v->context;

        char *cond = s->e->accept(s->e, v);
        const char *spec = szspec(type_to_int(s->e->type));

        char *lbl_else = s->else_ ? genlbl() : NULL;
        char *lbl_done = genlbl();

        // Compare condition to 0
        take_txt(ctx, forge_cstr_builder("cmp ", spec, " ", cond, ", 0", NULL), 1);

        // Jump to `else` (if present) or done if `false`
        take_txt(ctx, forge_cstr_builder("je ", s->else_ ? lbl_else : lbl_done, NULL), 1);

        s->then->accept(s->then, v);

        if (s->else_) {
                take_txt(ctx, forge_cstr_builder("jmp ", lbl_done, NULL), 1);
                take_txt(ctx, forge_cstr_builder(lbl_else, ":", NULL), 1);
                s->else_->accept(s->else_, v);
                free(lbl_else);
        }

        take_txt(ctx, forge_cstr_builder(lbl_done, ":", NULL), 1);
        free(lbl_done);

        free_reg_literal(cond);

        return NULL;
}

static void *
visit_stmt_while(visitor *v, stmt_while *s)
{
        asm_context *ctx     = (asm_context *)v->context;
        char *cond           = s->e->accept(s->e, v);
        const char *spec     = szspec(type_to_int(s->e->type));
        char *lbl_loop_begin = genlbl();
        char *lbl_loop_end   = genlbl();

        take_txt(ctx, forge_cstr_builder(lbl_loop_begin, ":", NULL), 1);
        take_txt(ctx, forge_cstr_builder("cmp ", spec, " ", cond, ", 0", NULL), 1);

        take_txt(ctx, forge_cstr_builder("je ", lbl_loop_end, NULL), 1);

        (void)s->body->accept(s->body, v);
        take_txt(ctx, forge_cstr_builder("jmp ", lbl_loop_begin, NULL), 1);

        take_txt(ctx, forge_cstr_builder(lbl_loop_end, ":", NULL), 1);

        free(lbl_loop_begin);
        free(lbl_loop_end);

        return NULL;
}

static visitor *
asm_visitor_alloc(asm_context *ctx)
{
        return visitor_alloc(
                (void *)ctx,
                visit_expr_bin,
                visit_expr_identifier,
                visit_expr_integer_literal,
                visit_expr_string_literal,
                visit_expr_proccall,
                visit_expr_mut,
                visit_stmt_let,
                visit_stmt_expr,
                visit_stmt_block,
                visit_stmt_proc,
                visit_stmt_return,
                visit_stmt_exit,
                visit_stmt_extern_proc,
                visit_stmt_if,
                visit_stmt_while
        );
}

static void
init(asm_context *ctx)
{
        char *asm_fp = forge_cstr_builder(g_config.filepath, ".asm", NULL);
        ctx->out = fopen(asm_fp, "w");
        free(asm_fp);

        if (!ctx->out) {
                perror("fopen");
                exit(1);
        }

        ctx->globals = dyn_array_empty(str_array);
        ctx->data_section = dyn_array_empty(str_array);
        ctx->externs = dyn_array_empty(str_array);
        ctx->pushed_regs = dyn_array_empty(str_array);

        write_txt(ctx, "section .text", 1);
}

static void
cleanup(asm_context *ctx)
{
        fclose(ctx->out);
        dyn_array_free(ctx->globals);
        dyn_array_free(ctx->data_section);
}

static void
write_globals(asm_context *ctx)
{
        for (size_t i = 0; i < ctx->globals.len; ++i) {
                write_txt(ctx, "global ", 0);
                write_txt(ctx, ctx->globals.data[i], 1);
        }
}

static void
write_data_section(asm_context *ctx)
{
        write_txt(ctx, "section .data", 1);
        for (size_t i = 0; i < ctx->data_section.len; ++i) {
                take_txt(ctx, ctx->data_section.data[i], 1);
        }
}

static void
write_externs(asm_context *ctx)
{
        for (size_t i = 0; i < ctx->externs.len; ++i) {
                take_txt(ctx, ctx->externs.data[i], 1);
        }
}

void
asm_gen(program *p, symtbl *tbl)
{
        NOOP(tbl, free_reg, alloc_param_regs);

        asm_context ctx = {0};
        visitor *v = asm_visitor_alloc(&ctx);
        init((asm_context *)v->context);

        for (size_t i = 0; i < p->stmts.len; ++i) {
                stmt *s = p->stmts.data[i];
                s->accept(s, v);
        }

        write_globals(&ctx);
        write_externs(&ctx);
        write_data_section(&ctx);
        write_txt(&ctx, "section .note.GNU-stack", 1);

        cleanup((asm_context *)v->context);
}
