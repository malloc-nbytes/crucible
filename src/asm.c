#include "asm.h"
#include "visitor.h"
#include "global.h"

#include <forge/err.h>
#include <forge/utils.h>
#include <forge/cstr.h>
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
        str_array globals;
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

static void
free_reg(int reg)
{
        g_inuse_regs[reg] = 0;
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
        NOOP(v);
        return e->i->lx;
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

char *
int_to_cstr(int i)
{
        /* int digits = 0; */
        /* for (digits = 0; i > 0; i /= 10, ++digits) { */
        /*         printf("%d\n", i); */
        /* } */
        /* char *s = (char *)malloc(digits + 1); */
        /* sprintf(s, "%d", i); */
        /* s[digits-1] = 0; */
        /* return s; */

        //if (n < 0) n = (n == INT_MIN) ? INT_MAX : -n;
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

static void *
visit_stmt_let(visitor *v, stmt_let *s)
{
        asm_context *ctx = (asm_context *)v->context;

        char *res = (char *)s->e->accept(s->e, v);

        int offset = s->resolved->stack_offset;
        char *offset_s = int_to_cstr(offset);
        //printf("%s\n", offset_s);
        take_txt(ctx, forge_cstr_builder("mov DWORD [rsp-", offset_s, "], ", res, NULL), 1);
        free(offset_s);

        return NULL;
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

        if (s->export) {
                dyn_array_append(ctx->globals, s->id->lx);
        }

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
        asm_context *ctx = (asm_context *)v->context;

        char *value = NULL;

        if (s->e) {
                value = s->e->accept(s->e, v);
        }

        write_txt(ctx, "mov rax, 60", 1);

        if (value) {
                take_txt(ctx, forge_cstr_builder("mov rdi, ", value, NULL), 1);
        } else {
                write_txt(ctx, "mov rdi, 0", 1);
        }

        write_txt(ctx, "syscall", 1);

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
        char *asm_fp = forge_cstr_builder(g_config.filepath, ".asm", NULL);
        ctx->out = fopen(asm_fp, "w");
        free(asm_fp);

        if (!ctx->out) {
                perror("fopen");
                exit(1);
        }

        ctx->globals = dyn_array_empty(str_array);

        write_txt(ctx, "section .text", 1);
}

static void
cleanup(asm_context *ctx)
{
        fclose(ctx->out);
}

static void
add_globals(asm_context *ctx)
{
        for (size_t i = 0; i < ctx->globals.len; ++i) {
                write_txt(ctx, "global ", 0);
                write_txt(ctx, ctx->globals.data[i], 1);
        }
}

void
asm_gen(program *p, symtbl *tbl)
{
        NOOP(tbl, free_reg, alloc_reg);

        asm_context ctx = {0};
        visitor *v = asm_visitor_alloc(&ctx);
        init((asm_context *)v->context);

        for (size_t i = 0; i < p->stmts.len; ++i) {
                stmt *s = p->stmts.data[i];
                s->accept(s, v);
        }

        add_globals((asm_context *)v->context);

        cleanup((asm_context *)v->context);
}
