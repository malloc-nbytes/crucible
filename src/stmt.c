#include "grammar.h"
#include "mem.h"

#include <assert.h>

stmt_block *
stmt_block_alloc(stmtp_ar stmts)
{
        assert(0);
}

stmt_proc *
stmt_proc_alloc(token      *id,
                typep_ar    ptypes,
                tokenp_ar   pids,
                type       *rtype,
                stmt_block *blk,
                uint32_t    meta)
{
        stmt_proc *s;

        s = (stmt_proc *)alloc(sizeof(stmt_proc));

        s->id        = id;
        s->ptypes    = ptypes;
        s->pids      = pids;
        s->rtype     = rtype;
        s->blk       = blk;
        s->meta      = meta;
        s->base.kind = STMT_KIND_PROC;
        s->base.loc  = (loc){0};

        return s;
}
