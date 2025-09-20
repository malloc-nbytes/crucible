#include "asm.h"
#include "visitor.h"

#include <forge/err.h>
#include <forge/utils.h>
#include <forge/cstr.h>

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

static const char *g_regs[] = {
        "rax", "eax",  "ax",   "al",
        "rcx", "ecx",  "cx",   "cl",
        "rdx", "edx",  "dx",   "dl",
        "rsi", "esi",  "si",   "sil",
        "rdi", "edi",  "di",   "dil",
        "r8",  "r8d",  "r8w",  "r8b",
        "r9",  "r9d",  "r9w",  "r9b",
        "r10", "r10d", "r10w", "r10b",
        "r11", "r11d", "r11w", "r11b"
};

const size_t g_regs_n = sizeof(g_regs)/sizeof(*g_regs);
#define g_regs_r 9
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
        0, 0, 0, 0,
};

typedef struct {
        FILE *out;
} asm_context;

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
                        //return REGAT(i, 0, g_regs);
                case 4:
                        REGAT(i, 1, g_inuse_regs) = 1;
                        return i * g_regs_c + 1;
                        //return REGAT(i, 1, g_regs);
                case 2:
                        REGAT(i, 2, g_inuse_regs) = 1;
                        return i * g_regs_c + 2;
                        //return REGAT(i, 2, g_regs);
                case 1:
                        REGAT(i, 3, g_inuse_regs) = 1;
                        return i * g_regs_c + 3;
                        //return REGAT(i, 3, g_regs);
                default: forge_err_wargs("alloc_reg(): cannot alloc register with size %d", sz);
                }
        }
        forge_err("alloc_reg(): no more registers");
}

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

static void
prologue(asm_context *ctx, int rsp_n)
{
        // TODO: stack overflow
        char rsp[1024] = {0};
        sprintf(rsp, "%d", rsp_n);

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
        NOOP(v, e);
        forge_todo("");
}

static void *
visit_expr_identifier(visitor *v, expr_identifier *e)
{
        NOOP(v, e);
        forge_todo("");
}

static void *
visit_expr_integer_literal(visitor *v, expr_integer_literal *e)
{
        NOOP(v, e);
        forge_todo("");
}

static void *
visit_expr_string_literal(visitor *v, expr_string_literal *e)
{
        NOOP(v, e);
        forge_todo("");
}

static void *
visit_expr_proccall(visitor *v, expr_proccall *e)
{
        NOOP(v, e);
        forge_todo("");
}

static void *
visit_stmt_let(visitor *v, stmt_let *s)
{
        NOOP(v, s);
        forge_todo("");
}

static void *
visit_stmt_expr(visitor *v, stmt_expr *s)
{
        NOOP(v, s);
        forge_todo("");
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

        take_txt(ctx, forge_cstr_builder(s->id->lx, ":", NULL), 1);
        prologue(ctx, s->rsp);

        // TODO: procedure parameters
        s->blk->accept(s->blk, v);

        epilogue(ctx);
        return NULL;
}

static void *
visit_stmt_return(visitor *v, stmt_return *s)
{
        NOOP(v, s);
        forge_todo("");
}

static void *
visit_stmt_exit(visitor *v, stmt_exit *s)
{
        NOOP(v, s);
        forge_todo("");
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
                visit_stmt_let,
                visit_stmt_expr,
                visit_stmt_block,
                visit_stmt_proc,
                visit_stmt_return,
                visit_stmt_exit
        );
}

static void
init(asm_context *ctx)
{
        ctx->out = fopen("out.asm", "w");
        if (!ctx->out) {
                perror("fopen");
                exit(1);
        }

        write_txt(ctx, "section .text", 1);
}

static void
cleanup(asm_context *ctx)
{
        fclose(ctx->out);
}

void
asm_gen(program *p, symtbl *tbl)
{
        NOOP(tbl);

        asm_context ctx = {0};
        visitor *v = asm_visitor_alloc(&ctx);
        init((asm_context *)v->context);

        for (size_t i = 0; i < p->stmts.len; ++i) {
                stmt *s = p->stmts.data[i];
                s->accept(s, v);
        }

        cleanup((asm_context *)v->context);
}
