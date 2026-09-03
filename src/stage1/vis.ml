type ('c, 'e, 's) t =
  { context         : 'c
  ; expr_integer    : ('c, 'e, 's) t -> Expr.integer    -> 'e * ('c, 'e, 's) t
  ; expr_identifier : ('c, 'e, 's) t -> Expr.identifier -> 'e * ('c, 'e, 's) t
  ; expr_string     : ('c, 'e, 's) t -> Expr.string_    -> 'e * ('c, 'e, 's) t
  ; expr_binary     : ('c, 'e, 's) t -> Expr.binary     -> 'e * ('c, 'e, 's) t
  ; expr_call       : ('c, 'e, 's) t -> Expr.call       -> 'e * ('c, 'e, 's) t
  ; expr_index      : ('c, 'e, 's) t -> Expr.index      -> 'e * ('c, 'e, 's) t
  ; expr_array      : ('c, 'e, 's) t -> Expr.array_     -> 'e * ('c, 'e, 's) t

  ; stmt_proc    : ('c, 'e, 's) t -> Stmt.proc   -> 's * ('c, 'e, 's) t
  ; stmt_let     : ('c, 'e, 's) t -> Stmt.let_   -> 's * ('c, 'e, 's) t
  ; stmt_expr    : ('c, 'e, 's) t -> Stmt.expr   -> 's * ('c, 'e, 's) t
  ; stmt_return  : ('c, 'e, 's) t -> Stmt.return -> 's * ('c, 'e, 's) t
  ; stmt_if      : ('c, 'e, 's) t -> Stmt.if_    -> 's * ('c, 'e, 's) t
  ; stmt_block   : ('c, 'e, 's) t -> Stmt.block  -> 's * ('c, 'e, 's) t
  ; stmt_while   : ('c, 'e, 's) t -> Stmt.while_ -> 's * ('c, 'e, 's) t
  }

let accept_expr v = function
  | Expr.Integer    e -> v.expr_integer    v e
  | Expr.Identifier e -> v.expr_identifier v e
  | Expr.String     e -> v.expr_string     v e
  | Expr.Binary     e -> v.expr_binary     v e
  | Expr.Call       e -> v.expr_call       v e
  | Expr.Index      e -> v.expr_index      v e
  | Expr.Array      e -> v.expr_array      v e

let accept_stmt v = function
  | Stmt.Proc   s -> v.stmt_proc   v s
  | Stmt.Let    s -> v.stmt_let    v s
  | Stmt.Expr   s -> v.stmt_expr   v s
  | Stmt.Return s -> v.stmt_return v s
  | Stmt.If     s -> v.stmt_if     v s
  | Stmt.Block  s -> v.stmt_block  v s
  | Stmt.While  s -> v.stmt_while  v s
