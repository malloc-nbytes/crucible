open Vis
open Stmt
open Expr

type context =
  { scope : Symbol.t Scope.t
  ; return_type : Type.t option
  ; next_symbol_id : int
  }

type vis_type = (context, Expr.t, Stmt.t) Vis.t

let new_symbol
      (kind : Symbol.kind)
      (id : Token.t)
      (ty : Type.t)
      (v : vis_type)
    : Symbol.t * vis_type =
  let sym =
    Symbol.
    { id = v.context.next_symbol_id
    ; name = id.lx
    ; ty
    ; kind
    ; loc = id.loc
    }
  in
  sym,
  { v with
    context =
      { v.context with
        next_symbol_id = v.context.next_symbol_id + 1
      }
  }

let resolve_expr_binary
      (v : vis_type)
      ({lhs; op; rhs; _} as e : Expr.binary)
    : Expr.t * vis_type =
  let lhs, v = accept_expr v lhs in
  let rhs, v = accept_expr v rhs in
  let lty, rty = Expr.get_type lhs, Expr.get_type rhs in
  if not @@ Type.check lty rty then
    raise @@ Err.Incompatible_Types (Expr.get_location lhs, lty, rty)
  else if not @@ Type.binop_check lty op.k rty then
    raise @@ Err.Incompatible_Binop_Types (Expr.get_location lhs, lty, op.k, rty)
  else
    Expr.Binary
      { node = {e.node with ty = lty}
      ; lhs
      ; op
      ; rhs
      }, v

let resolve_expr_string
      (v : vis_type)
      (e : Expr.string_)
    : Expr.t * vis_type =
  Expr.String
    { e with
      node = {e.node with ty = Ptr U8}
    }, v

let resolve_expr_identifier
      ({context; _}as v : vis_type)
      ({node; id = {lx = name; _}; _} as e : Expr.identifier)
    : Expr.t * vis_type =
  match Scope.get name context.scope with
  | None -> raise @@ Err.Identifier_Not_Defined (e.node.loc, name)
  | Some sym ->
     Expr.Identifier
       { e with
         node = {node with ty = sym.ty}
       ; sym = Some sym
       }, v

let resolve_expr_integer
      (v : vis_type)
      (e : Expr.integer)
    : Expr.t * vis_type =
  Expr.Integer
    { e with
      node = {e.node with ty = I32}
    }, v

let resolve_stmt_block
      (v : vis_type)
      ({stmts; _} as s : Stmt.block)
    : Stmt.t * vis_type =
  let rec aux acc v = function
    | [] -> List.rev acc, v
    | hd :: tl ->
       let s, v = accept_stmt v hd in
       aux (s :: acc) v tl
  in
  let stmts, v = aux [] {v with context = {v.context with scope = Scope.push v.context.scope}} stmts in
  Stmt.Block {s with stmts}, {v with context = {v.context with scope = Scope.pop v.context.scope}}

let resolve_stmt_if
      (v : vis_type)
      ({cond; then_; else_; _} as s : Stmt.if_)
    : Stmt.t * vis_type =
  let cond, v = accept_expr v cond in
  let then_, v = accept_stmt v then_ in
  let else_, v = match else_ with
    | Some else_ ->
       let else_, v = accept_stmt v else_ in
       Some else_, v
    | None -> None, v
  in
  Stmt.If
    { s with
      cond
    ; then_
    ; else_
    }, v

let resolve_stmt_return
      ({context = {return_type; _}; _} as v : vis_type)
      ({node = {loc; _}; e; _} as s : Stmt.return)
    : Stmt.t * vis_type =
    match e, return_type with
    | _, None -> raise @@ Err.Illegal_Statement loc
    | None, Some Void ->
       Stmt.Return {s with e = None}, v
    | None, Some t ->
       raise @@ Err.Incompatible_Types (loc, Void, t)
    | Some e, Some return_type ->
       let e, v = accept_expr v e in
       if not @@ Type.check (Expr.get_type e) return_type then
         raise @@ Err.Incompatible_Types (Expr.get_location e, Expr.get_type e, return_type)
       else
         Stmt.Return {s with e = Some e}, v

let resolve_stmt_expr
      (v : vis_type)
      ({e; _} as s : Stmt.expr)
    : Stmt.t * vis_type =
  let e, v = accept_expr v e in
  Stmt.Expr { s with e }, v

let resolve_stmt_let
      ({context; _} as v : vis_type)
      ({id; ty; _} as s : Stmt.let_)
    : Stmt.t * vis_type =
  let name = id.lx in
  if Scope.contains name context.scope then
    raise @@ Err.Identifier_Already_Defined (s.node.loc, name)
  else
    let e, v = accept_expr v s.e in
    if not @@ Type.check ty (Expr.get_type e) then
      raise @@ Err.Incompatible_Types (Expr.get_location e, ty, (Expr.get_type e))
    else
      let sym, v = new_symbol Symbol.Local id ty v in
      Stmt.Let
        { s with
          e
        ; sym = Some sym
        },
      { v with
        context =
          { v.context with
            scope = Scope.add name sym v.context.scope
          }
      }

let resolve_stmt_proc
      ({context; _} as v : vis_type)
      ({id; rty; params; body; _} as s : Stmt.proc)
    : Stmt.t * vis_type =
  let name = id.lx in
  if Scope.contains name context.scope then
    raise @@ Err.Identifier_Already_Defined (s.node.loc, name)
  else
    let proc_ty =
      Type.Proc
        { rty
        ; ptys = List.map (fun (p : Stmt.parameter) -> p.ty) params
        }
    in
    let sym, v = new_symbol Symbol.Proc id proc_ty v in
    let outer_scope = Scope.add name sym v.context.scope in
    let body_v =
      { v with
        context =
          { v.context with
            scope = Scope.push outer_scope
          ; return_type = Some rty
          }
      }
    in
    let params, body_v =
      List.fold_left
        (fun (params, v) (param : Stmt.parameter) ->
          let name = param.id.lx in
          if Scope.contains name v.context.scope then
            raise @@ Err.Identifier_Already_Defined (param.id.loc, name)
          else
            let symbol, v =
              new_symbol Symbol.Param param.id param.ty v
            in
            let scope = Scope.add name symbol v.context.scope in
            {param with sym = Some symbol} :: params,
            {v with context = {v.context with scope}})
        ([], body_v)
        params
    in
    let params = List.rev params in
    let body, v = match body with
      | None ->
         None,
         { body_v with
           context =
             { body_v.context with
               scope = outer_scope
             ; return_type = context.return_type
             }
         }
      | Some body ->
         let body, body_v = accept_stmt body_v body in
         Some body,
         { body_v with
           context =
             { body_v.context with
               scope = outer_scope
             ; return_type = context.return_type
             }
         }
    in
    Stmt.Proc
      { s with
        params
      ; body
      ; sym = Some sym
      },
    v

let analyze stmts =
  try
    let rec aux v = function
      | [] -> []
      | hd :: tl ->
         let s, v = accept_stmt v hd in
         s :: aux v tl
    in aux Vis.
       { context = { scope = Scope.empty
                   ; return_type = None
                   ; next_symbol_id = 0
                   }
       ; expr_integer = resolve_expr_integer
       ; expr_identifier = resolve_expr_identifier
       ; expr_string = resolve_expr_string
       ; expr_binary = resolve_expr_binary

       ; stmt_proc = resolve_stmt_proc
       ; stmt_let = resolve_stmt_let
       ; stmt_expr = resolve_stmt_expr
       ; stmt_return = resolve_stmt_return
       ; stmt_if = resolve_stmt_if
       ; stmt_block = resolve_stmt_block
       } stmts
  with
  | Err.Illegal_Statement l ->
     let _ = Printf.eprintf "%s: illegal statement\n"
               (Location.to_string l) in
     failwith "semantic error"
  | Err.Incompatible_Types (l, t, t') ->
     let _ = Printf.eprintf "%s: incompatible types of `%s' and `%s'\n"
               (Location.to_string l)
               (Type.to_string t)
               (Type.to_string t') in
     failwith "semantic error"
  | Err.Incompatible_Binop_Types (l, t, op, t') ->
     let _ = Printf.eprintf "%s: incompatible types for binary operator `%s' of `%s' and `%s'\n"
               (Location.to_string l)
               (Token.kind_to_string op)
               (Type.to_string t)
               (Type.to_string t') in
     failwith "semantic error"
  | Err.Identifier_Not_Defined (l, id) ->
     let _ = Printf.eprintf "%s: identifier `%s' is not defined\n"
               (Location.to_string l) id in
     failwith "semantic error"
  | Err.Identifier_Already_Defined (l, id) ->
     let _ = Printf.eprintf "%s: identifier `%s' is already defined\n"
               (Location.to_string l) id in
     failwith "semantic error"
