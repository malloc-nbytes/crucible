open Vis
open Stmt
open Expr

type context =
  { scope : Symbol.t Scope.t
  ; types : Type.t Scope.t
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

let is_assignable = function
  | Expr.Identifier {sym = Some {kind = Symbol.Local | Symbol.Param; _}; _} -> true
  | Expr.Index _ -> true
  | Expr.Member _ -> true
  | Expr.Unary {op = {k = Token.Asterisk; _}; _} -> true
  | _ -> false

let is_addressable = function
  | Expr.Identifier {sym = Some {kind = Symbol.Local | Symbol.Param; _}; _} -> true
  | Expr.Index _ -> true
  | Expr.Member _ -> true
  | Expr.Unary {op = {k = Token.Asterisk; _}; _} -> true
  | _ -> false

let rec resolve_type types = function
  | Type.Custom name ->
     (match Scope.get name types with
      | Some ty -> ty
      | None -> failwith @@ "unknown type `" ^ name ^ "`")
  | Type.Ptr ty -> Type.Ptr (resolve_type types ty)
  | Type.Array (ty, length) -> Type.Array (resolve_type types ty, length)
  | Type.Proc {rty; ptys; variadic} ->
     Type.Proc
       { rty = resolve_type types rty
       ; ptys = List.map (resolve_type types) ptys
       ; variadic
       }
  | ty -> ty

let resolve_expr_unary
      (v : vis_type)
      ({op; rhs; _} as e : Expr.unary)
    : Expr.t * vis_type =
  let rhs, v = accept_expr v rhs in
  match op.k, Expr.get_type rhs with
  | Token.Asterisk, Type.Ptr ty
    | Token.Asterisk, Type.Array (ty, _) ->
     Expr.Unary {e with node = {e.node with ty}; rhs}, v
  | Token.Asterisk, ty ->
     raise @@ Err.Invalid_Dereference_Target (Expr.get_location rhs, ty)
  | Token.Ampersand, Type.Array (ty, _) when is_addressable rhs ->
     Expr.Unary {e with node = {e.node with ty = Type.Ptr ty}; rhs}, v
  | Token.Ampersand, ty when is_addressable rhs ->
     Expr.Unary {e with node = {e.node with ty = Type.Ptr ty}; rhs}, v
  | Token.Ampersand, _ ->
     raise @@ Err.Invalid_Address_Of_Target (Expr.get_location rhs)
  | _ -> assert false

let resolve_expr_cast
      (v : vis_type)
      ({target; rhs; _} as e : Expr.cast)
    : Expr.t * vis_type =
  let rhs, v = accept_expr v rhs in
  let source = Expr.get_type rhs in
  if not @@ Type.is_castable source target then
    raise @@ Err.Invalid_Cast (e.node.loc, source, target)
  else
    Expr.Cast {e with node = {e.node with ty = target}; rhs}, v

let resolve_expr_binary
      (v : vis_type)
      ({lhs; op; rhs; _} as e : Expr.binary)
    : Expr.t * vis_type =
  let lhs, v = accept_expr v lhs in
  let rhs, v = accept_expr v rhs in
  let lty, rty = Expr.get_type lhs, Expr.get_type rhs in
  if Type.is_assignment op.k && not @@ is_assignable lhs then
    raise @@ Err.Invalid_Assignment_Target (Expr.get_location lhs)
  else if not @@ Type.check lty rty then
    raise @@ Err.Incompatible_Types (Expr.get_location lhs, lty, rty)
  else if not @@ Type.binop_check lty op.k rty then
    raise @@ Err.Incompatible_Binop_Types (Expr.get_location lhs, lty, op.k, rty)
  else
    let ty = if Type.is_comparison op.k then Type.I32 else lty in
    Expr.Binary
      { node = {e.node with ty}
      ; lhs
      ; op
      ; rhs
      }, v

let resolve_expr_call
      (v : vis_type)
      ({lhs; args; _} as e : Expr.call)
    : Expr.t * vis_type =
  let lhs, v = accept_expr v lhs in
  let args, v =
    List.fold_left
      (fun (args, v) arg ->
        let arg, v = accept_expr v arg in
        arg :: args, v)
      ([], v)
      args
  in
  let args = List.rev args in
  match Expr.get_type lhs with
  | Type.Proc {rty; ptys; variadic} ->
     let argc = List.length args in
     let paramc = List.length ptys in
     if (variadic && argc < paramc) || (not variadic && argc <> paramc) then
       raise @@
         Err.Invalid_Argument_Count (Expr.get_location (Expr.Call e), paramc, argc)
     else
       let rec check_args args ptys =
         match args, ptys with
         | _, [] -> ()
         | arg :: args, pty :: ptys ->
            let aty = Expr.get_type arg in
            if not @@ Type.check pty aty then
              raise @@ Err.Incompatible_Types (Expr.get_location arg, pty, aty)
            else
              check_args args ptys
         | [], _ :: _ -> assert false
       in
       check_args args ptys;
       Expr.Call
         { node = {e.node with ty = rty}
         ; lhs
         ; args
         }, v
  | ty -> raise @@ Err.Invalid_Call_Target (Expr.get_location lhs, ty)

let resolve_expr_index
      (v : vis_type)
      ({lhs; idx; _} as e : Expr.index)
    : Expr.t * vis_type =
  let lhs, v = accept_expr v lhs in
  let idx, v = accept_expr v idx in
  match Expr.get_type lhs with
  | Type.Array (ty, _) ->
     if not @@ Type.check (Expr.get_type idx) Type.I32 then
       raise @@ Err.Invalid_Index_Type (Expr.get_location idx, Expr.get_type idx)
     else
       Expr.Index {node = {e.node with ty}; lhs; idx}, v
  | Type.Ptr ty ->
     if not @@ Type.check (Expr.get_type idx) Type.I32 then
       raise @@ Err.Invalid_Index_Type (Expr.get_location idx, Expr.get_type idx)
     else
       Expr.Index {node = {e.node with ty}; lhs; idx}, v
  | ty -> raise @@ Err.Invalid_Index_Target (Expr.get_location lhs, ty)

let resolve_expr_member
      (v : vis_type)
      ({lhs; id; _} as e : Expr.member)
    : Expr.t * vis_type =
  let lhs, v = accept_expr v lhs in
  let struct_ty = match Expr.get_type lhs with
    | Type.Struct _ as ty -> ty
    | Type.Ptr (Type.Struct _ as ty) -> ty
    | ty -> failwith @@ "cannot access a member of type `" ^ Type.to_string ty ^ "`"
  in
  match Type.field_by_name id.lx struct_ty with
  | Some field ->
     Expr.Member {e with node = {e.node with ty = field.ty}; lhs; field = Some field}, v
  | None -> failwith @@ "struct has no field `" ^ id.lx ^ "`"

let resolve_expr_array
      (v : vis_type)
      ({exprs; _} as e : Expr.array_)
    : Expr.t * vis_type =
  let exprs, v =
    List.fold_left
      (fun (exprs, v) expr ->
        let expr, v = accept_expr v expr in
        expr :: exprs, v)
      ([], v)
      exprs
  in
  let exprs = List.rev exprs in
  match exprs with
  | [] -> raise @@ Err.Empty_Array_Literal e.node.loc
  | first :: rest ->
     let ty = Expr.get_type first in
     List.iter
       (fun expr ->
         if not @@ Type.check ty (Expr.get_type expr) then
           raise @@ Err.Incompatible_Types
                       (Expr.get_location expr, ty, Expr.get_type expr))
       rest;
     Expr.Array
       {node = {e.node with ty = Type.Array (ty, List.length exprs)}; exprs}, v

let resolve_expr_struct
      (v : vis_type)
      ({id; fields; _} as e : Expr.struct_)
    : Expr.t * vis_type =
  let ty = resolve_type v.context.types (Type.Custom id.lx) in
  let expected_fields = match ty with
    | Type.Struct {fields; _} -> fields
    | _ -> failwith @@ "type `" ^ id.lx ^ "` is not a struct"
  in
  let fields, v =
    List.fold_left
      (fun (fields, v) (field_id, value) ->
        let value, v = accept_expr v value in
        (field_id, value) :: fields, v)
      ([], v)
      fields
  in
  let fields = List.rev fields in
  if List.length fields <> List.length expected_fields then
    failwith "struct literal has the wrong number of fields"
  else
    List.iter
      (fun (expected : Type.struct_field) ->
        match List.find_opt
                (fun ((field : Token.t), _) -> field.lx = expected.Type.name)
                fields with
        | None -> failwith @@ "struct literal is missing field `" ^ expected.name ^ "`"
        | Some (_, value) when not @@ Type.check expected.ty (Expr.get_type value) ->
           raise @@ Err.Incompatible_Types
                       (Expr.get_location value, expected.ty, Expr.get_type value)
        | Some _ -> ())
      expected_fields;
    Expr.Struct {e with node = {e.node with ty}; fields}, v

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

let resolve_stmt_while
      (v : vis_type)
      ({cond; body; _} as s : Stmt.while_)
    : Stmt.t * vis_type =
  let e, v = accept_expr v cond.e in
  let body, v = accept_stmt v body in
  Stmt.While
    { s with
      cond = {cond with e}
    ; body
    }, v

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
    let ty = resolve_type context.types ty in
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

let resolve_stmt_struct
      ({context; _} as v : vis_type)
      ({id; fields; _} as s : Stmt.struct_)
    : Stmt.t * vis_type =
  if Scope.contains id.lx context.types then
    raise @@ Err.Identifier_Already_Defined (id.loc, id.lx)
  else
    let fields =
      List.map
        (fun (field : Stmt.struct_field) ->
          field.id.lx, resolve_type context.types field.ty)
        fields
    in
    let ty = Type.make_struct id.lx fields in
    Stmt.Struct {s with ty = Some ty},
    {v with context = {v.context with types = Scope.add id.lx ty context.types}}

let resolve_stmt_proc
      ({context; _} as v : vis_type)
      ({id; rty; params; variadic; body; _} as s : Stmt.proc)
    : Stmt.t * vis_type =
  let rty = resolve_type context.types rty in
  let params =
    List.map
      (fun (param : Stmt.parameter) ->
        {param with ty = resolve_type context.types param.ty})
      params
  in
  let name = id.lx in
  if Scope.contains name context.scope then
    raise @@ Err.Identifier_Already_Defined (s.node.loc, name)
  else
    let proc_ty =
      Type.Proc
        { rty
        ; ptys = List.map (fun (p : Stmt.parameter) -> p.ty) params
        ; variadic
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
                   ; types = Scope.empty
                   ; return_type = None
                   ; next_symbol_id = 0
                   }
       ; expr_integer    = resolve_expr_integer
       ; expr_identifier = resolve_expr_identifier
       ; expr_string     = resolve_expr_string
       ; expr_binary     = resolve_expr_binary
       ; expr_call       = resolve_expr_call
       ; expr_unary      = resolve_expr_unary
       ; expr_cast       = resolve_expr_cast
       ; expr_index      = resolve_expr_index
       ; expr_array      = resolve_expr_array
       ; expr_struct     = resolve_expr_struct
       ; expr_member     = resolve_expr_member

       ; stmt_proc   = resolve_stmt_proc
       ; stmt_let    = resolve_stmt_let
       ; stmt_expr   = resolve_stmt_expr
       ; stmt_return = resolve_stmt_return
       ; stmt_if     = resolve_stmt_if
       ; stmt_block  = resolve_stmt_block
       ; stmt_while  = resolve_stmt_while
       ; stmt_struct = resolve_stmt_struct
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
  | Err.Invalid_Call_Target (l, ty) ->
     let _ = Printf.eprintf "%s: cannot call expression of type `%s'\n"
               (Location.to_string l) (Type.to_string ty) in
     failwith "semantic error"
  | Err.Invalid_Argument_Count (l, expected, got) ->
     let _ = Printf.eprintf "%s: procedure expects %d arguments but got %d\n"
               (Location.to_string l) expected got in
     failwith "semantic error"
  | Err.Identifier_Not_Defined (l, id) ->
     let _ = Printf.eprintf "%s: identifier `%s' is not defined\n"
               (Location.to_string l) id in
     failwith "semantic error"
  | Err.Identifier_Already_Defined (l, id) ->
     let _ = Printf.eprintf "%s: identifier `%s' is already defined\n"
               (Location.to_string l) id in
     failwith "semantic error"
  | Err.Invalid_Assignment_Target l ->
     let _ = Printf.eprintf "%s: invalid assignment target\n"
               (Location.to_string l) in
     failwith "semantic error"
  | Err.Empty_Array_Literal l ->
     let _ = Printf.eprintf "%s: cannot infer the type of an empty array literal\n"
               (Location.to_string l) in
     failwith "semantic error"
  | Err.Invalid_Index_Target (l, ty) ->
     let _ = Printf.eprintf "%s: cannot index expression of type `%s'\n"
               (Location.to_string l) (Type.to_string ty) in
     failwith "semantic error"
  | Err.Invalid_Index_Type (l, ty) ->
     let _ = Printf.eprintf "%s: array index has type `%s', expected `i32'\n"
               (Location.to_string l) (Type.to_string ty) in
     failwith "semantic error"
  | Err.Invalid_Dereference_Target (l, ty) ->
     let _ = Printf.eprintf "%s: cannot dereference expression of type `%s'\n"
               (Location.to_string l) (Type.to_string ty) in
     failwith "semantic error"
  | Err.Invalid_Address_Of_Target l ->
     let _ = Printf.eprintf "%s: invalid address-of target\n"
               (Location.to_string l) in
     failwith "semantic error"
  | Err.Invalid_Cast (l, source, target) ->
     let _ = Printf.eprintf "%s: cannot cast `%s' to `%s'\n"
               (Location.to_string l)
               (Type.to_string source) (Type.to_string target) in
     failwith "semantic error"
