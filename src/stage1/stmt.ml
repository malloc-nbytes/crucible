type node =
  { loc : Location.t
  }

type linkage =
  | Internal
  | Export
  | Extern
  | Extern_Export

type parameter =
  { id : Token.t
  ; ty : Type.t
  ; sym : Symbol.t option
  }

type t =
  | Proc of proc
  | Let of let_
  | Expr of expr
  | Return of return
  | If of if_
  | Block of block
  | While of while_

and proc =
  { node : node
  ; linkage : linkage
  ; id : Token.t
  ; params : parameter list
  ; variadic : bool
  ; rty : Type.t
  ; body : t option
  ; sym : Symbol.t option
  }

and let_ =
  { node : node
  ; id : Token.t
  ; ty : Type.t
  ; e : Expr.t
  ; sym : Symbol.t option
  }

and expr =
  { node : node
  ; e : Expr.t
  }

and return =
  { node : node
  ; e : Expr.t option
  }

and if_ =
  { node : node
  ; cond : Expr.t
  ; then_ : t
  ; else_ : t option
  }

and block =
  { node : node
  ; stmts : t list
  }

and while_ =
  { node : node
  ; cond : expr
  ; body : t
  }
