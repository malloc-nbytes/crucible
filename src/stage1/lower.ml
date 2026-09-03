type program_state =
  { strings : (string, Tac.string_id) Hashtbl.t
  ; mutable next_string_id : int
  ; mutable strings_rev : Tac.string_literal list
  }

type builder =
  { program : program_state
  ; mutable next_temp : int
  ; mutable next_label : int
  ; mutable current_label : int
  ; mutable current_instrs_rev : Tac.instruction list
  ; mutable current_open : bool
  ; mutable completed_blocks_rev : Tac.block list
  ; bindings : (int, Tac.operand) Hashtbl.t
  }

let fail_unresolved what =
  failwith @@ "lowering invariant violated: unresolved " ^ what

let require_symbol what = function
  | Some sym -> sym
  | None -> fail_unresolved what

let new_temp builder =
  let temp = builder.next_temp in
  builder.next_temp <- builder.next_temp + 1;
  temp

let new_label builder =
  let lbl = builder.next_label in
  builder.next_label <- builder.next_label + 1;
  lbl

let emit builder instruction =
  if not builder.current_open then
    failwith "cannot emit an instruction into a terminated TAC block";
  builder.current_instrs_rev <- instruction :: builder.current_instrs_rev

let start_block builder label =
  if builder.current_open then
    failwith "cannot start TAC block before terminating the current block";
  builder.current_label <- label;
  builder.current_instrs_rev <- [];
  builder.current_open <- true

let end_block builder terminator =
  if not builder.current_open then
    failwith "cannot terminate already terminated TAC block";
  let block =
    Tac.
    { label = builder.current_label
    ; instrs = List.rev builder.current_instrs_rev
    ; terminator
    }
  in
  builder.completed_blocks_rev <- block :: builder.completed_blocks_rev;
  builder.current_instrs_rev <- [];
  builder.current_open <- false

let intern_string program value =
  match Hashtbl.find_opt program.strings value with
  | Some id -> id
  | None ->
     let id = program.next_string_id in
     program.next_string_id <- program.next_string_id + 1;
     Hashtbl.add program.strings value id;
     program.strings_rev <-
       Tac.{id; value} :: program.strings_rev;
     id

let bind_symbol builder (sym : Symbol.t) value =
  Hashtbl.replace builder.bindings sym.id value

let lookup_symbol builder (sym : Symbol.t) =
  match Hashtbl.find_opt builder.bindings sym.id with
  | Some value -> value
  | None ->
     failwith @@
       Printf.sprintf "lowering invariant violated: symbol %s (#%d) has no TAC binding"
         sym.name
         sym.id

let require_resolved_type expr =
  match Expr.get_type expr with
  | Type.Undefined ->
     failwith "lowering invariant violated: expression type is undefined"
  | ty -> ty

let lower_identifier builder ({node; sym; _} : Expr.identifier) =
  let sym = require_symbol "identifier" sym in
  match sym.kind with
  | Symbol.Proc -> Tac.Proc sym
  | Symbol.Local | Symbol.Param ->
     let src = lookup_symbol builder sym in
     (match node.ty with
      | Type.Array _ -> src
      | _ ->
         let dst = new_temp builder in
         emit builder (Tac.Load {dst; ty = node.ty; src});
         Tac.Temp dst)

let lower_assignment_target builder = function
  | Expr.Identifier {sym; _} ->
     let sym = require_symbol "assignment target" sym in
     lookup_symbol builder sym
  | _ -> failwith "lowering invariant violated: assignment target is not an identifier"

let rec lower_address builder = function
  | Expr.Identifier {sym; _} ->
     let sym = require_symbol "address-of target" sym in
     let src = lookup_symbol builder sym in
     let dst = new_temp builder in
     let ty = match sym.ty with
       | Type.Array (element_type, _) -> Type.Ptr element_type
       | ty -> Type.Ptr ty
     in
     emit builder (Tac.Address_of {dst; ty; src});
     Tac.Temp dst
  | Expr.Index {lhs; idx; _} as expression ->
     let element_type = require_resolved_type expression in
     let src = lower_expr builder lhs in
     let idx = lower_expr builder idx in
     let dst = new_temp builder in
     emit builder (Tac.Index_address {dst; element_type; src; idx});
     Tac.Temp dst
  | Expr.Unary {op = {k = Token.Asterisk; _}; rhs; _} -> lower_expr builder rhs
  | _ -> failwith "lowering invariant violated: address-of target is not addressable"

and lower_index_store builder lhs rhs op ty =
  match lhs with
  | Expr.Index {lhs = array; idx; _} ->
     let array = lower_expr builder array in
     let idx = lower_expr builder idx in
     let value = match Type.compound_assignment_binop op with
       | None -> lower_expr builder rhs
       | Some binop ->
          let old = new_temp builder in
          emit builder (Tac.Index_load {dst = old; element_type = ty; src = array; idx});
          let rhs = lower_expr builder rhs in
          let dst = new_temp builder in
          emit builder
            (Tac.Binop
               { dst
               ; ty
               ; result_ty = ty
               ; op = Tac.binop_of_token binop
               ; lhs = Tac.Temp old
               ; rhs
               });
          Tac.Temp dst
     in
     emit builder (Tac.Index_store {element_type = ty; src = value; idx; dst = array});
     value
  | _ -> assert false

and lower_deref_store builder pointer rhs op element_type =
  let pointer = lower_expr builder pointer in
  let value = match Type.compound_assignment_binop op with
    | None -> lower_expr builder rhs
    | Some binop ->
       let old = new_temp builder in
       emit builder (Tac.Deref_load {dst = old; element_type; src = pointer});
       let rhs = lower_expr builder rhs in
       let dst = new_temp builder in
       emit builder
         (Tac.Binop
            { dst
            ; ty = element_type
            ; result_ty = element_type
            ; op = Tac.binop_of_token binop
            ; lhs = Tac.Temp old
            ; rhs
            });
       Tac.Temp dst
  in
  emit builder (Tac.Deref_store {element_type; src = value; dst = pointer});
  value

and lower_expr builder = function
  | Expr.Integer {i; _} ->
     Tac.I32 (int_of_string i.lx)

  | Expr.String {s; _} ->
     Tac.String (intern_string builder.program s.lx)

  | Expr.Identifier e -> lower_identifier builder e

  | Expr.Binary {lhs; op; rhs; _} as expression ->
     let ty  = require_resolved_type expression in
     if Type.is_assignment op.k then
       (match lhs with
        | Expr.Index _ -> lower_index_store builder lhs rhs op.k ty
        | Expr.Unary {op = {k = Token.Asterisk; _}; rhs = pointer; _} ->
           lower_deref_store builder pointer rhs op.k ty
        | _ ->
           let dst = lower_assignment_target builder lhs in
           let lhs = match Type.compound_assignment_binop op.k with
             | None -> None
             | Some _ -> Some (lower_expr builder lhs)
           in
           let rhs = lower_expr builder rhs in
           let value = match lhs, Type.compound_assignment_binop op.k with
             | None, None -> rhs
             | Some lhs, Some op ->
                let temp = new_temp builder in
                emit builder
                  (Tac.Binop
                     { dst = temp
                     ; ty
                     ; result_ty = ty
                     ; op = Tac.binop_of_token op
                     ; lhs
                     ; rhs
                     });
                Tac.Temp temp
             | _ -> assert false
           in
           emit builder (Tac.Store {ty; src = value; dst});
           value)
     else
       let operand_ty = require_resolved_type lhs in
       let lhs = lower_expr builder lhs in
       let rhs = lower_expr builder rhs in
       let dst = new_temp builder in
       emit builder
         (Tac.Binop
            { dst
            ; ty = operand_ty
            ; result_ty = ty
            ; op = Tac.binop_of_token op.k
            ; lhs
            ; rhs
         });
       Tac.Temp dst

  | Expr.Call {lhs; args; _} as expression ->
     let callee = lower_expr builder lhs in
     let args = List.map (lower_expr builder) args in
     let ty = require_resolved_type expression in
     let dst = match ty with
       | Type.Void -> None
       | _ -> Some (new_temp builder)
     in
     emit builder (Tac.Call {dst; ty; callee; args});
     (match dst with
      | Some dst -> Tac.Temp dst
      | None -> Tac.Void)

  | Expr.Unary {op = {k = Token.Ampersand; _}; rhs; _} ->
     lower_address builder rhs

  | Expr.Unary {op = {k = Token.Asterisk; _}; rhs; _} as expression ->
     let element_type = require_resolved_type expression in
     let src = lower_expr builder rhs in
     let dst = new_temp builder in
     (match Expr.get_type rhs with
      | Type.Array _ ->
         emit builder (Tac.Index_load {dst; element_type; src; idx = Tac.I32 0})
      | Type.Ptr _ ->
         emit builder (Tac.Deref_load {dst; element_type; src})
      | _ -> assert false);
     Tac.Temp dst

  | Expr.Cast {target; rhs; _} ->
     let source_ty = require_resolved_type rhs in
     let source_ty, src = match source_ty with
       | Type.Array (element_type, _) ->
          Type.Ptr element_type, lower_address builder rhs
       | _ -> source_ty, lower_expr builder rhs
     in
     let dst = new_temp builder in
     emit builder (Tac.Cast {dst; source_ty; target_ty = target; src});
     Tac.Temp dst

  | Expr.Unary _ ->
     failwith "lowering invariant violated: unsupported unary operator"

  | Expr.Index {lhs; idx; _} as expression ->
     let element_type = require_resolved_type expression in
     let src = lower_expr builder lhs in
     let idx = lower_expr builder idx in
     let dst = new_temp builder in
     emit builder (Tac.Index_load {dst; element_type; src; idx});
     Tac.Temp dst

  | Expr.Array _ ->
     failwith "lowering invariant violated: array literal requires an array declaration"

let rec lower_stmt builder = function
  | Stmt.Proc _ ->
     failwith "nested procedures are not supported by TAC lowering"

  | Stmt.Let {sym; e; _} ->
     let symbol = require_symbol "let declaration" sym in
     let dst = Tac.Local symbol in
     bind_symbol builder symbol dst;
     (match e with
      | Expr.Array {exprs; _} ->
         let element_type = match symbol.ty with
           | Type.Array (element_type, _) -> element_type
           | _ -> failwith "lowering invariant violated: array literal has non-array type"
         in
         let elements = List.map (lower_expr builder) exprs in
         emit builder (Tac.Array {dst; element_type; elements})
      | _ ->
         let value = lower_expr builder e in
         emit builder (Tac.Store {ty = symbol.ty; src = value; dst}))

  | Stmt.Expr {e; _} ->
     ignore @@ lower_expr builder e

  | Stmt.Block {stmts; _} ->
     lower_stmts builder stmts

  | Stmt.Return {e; _} ->
     let value = match e with
       | None -> None
       | Some e -> Some (lower_expr builder e)
     in
     end_block builder (Tac.Ret value)

  | Stmt.If {cond; then_; else_; _} ->
     lower_if builder cond then_ else_

  | Stmt.While {cond; body; _} ->
     lower_while builder cond body

and lower_stmts builder = function
  | [] -> ()
  | stmt :: stmts ->
     if not builder.current_open then
       failwith "unreachable statement after a terminating statement";
     lower_stmt builder stmt;
     lower_stmts builder stmts

and lower_while builder cond body =
  let cond_lbl = new_label builder in
  let body_lbl = new_label builder in
  let exit_lbl = new_label builder in
  end_block builder (Tac.Jmp cond_lbl);

  start_block builder cond_lbl;
  let cond = lower_expr builder cond.Stmt.e in
  end_block builder
    (Tac.Branch
       { cond
       ; if_true = body_lbl
       ; if_false = exit_lbl
       });

  start_block builder body_lbl;
  lower_stmt builder body;
  if builder.current_open then
    end_block builder (Tac.Jmp cond_lbl);

  start_block builder exit_lbl

and lower_if builder cond then_ else_ =
  let cond = lower_expr builder cond in
  let then_lbl = new_label builder in
  match else_ with
  | None ->
     let join_lbl = new_label builder in
     end_block builder
       (Tac.Branch
          { cond
          ; if_true = then_lbl
          ; if_false = join_lbl
       });
     start_block builder then_lbl;
     lower_stmt builder then_;
     if builder.current_open then
       end_block builder (Tac.Jmp join_lbl);
     start_block builder join_lbl
  | Some else_ ->
     let else_lbl = new_label builder in
     let join_lbl = new_label builder in
     end_block builder
       (Tac.Branch
          { cond
          ; if_true = then_lbl
          ; if_false = else_lbl });

     start_block builder then_lbl;
     lower_stmt builder then_;
     let then_falls_through = builder.current_open in
     if then_falls_through then
       end_block builder (Tac.Jmp join_lbl);

     start_block builder else_lbl;
     lower_stmt builder else_;
     let else_falls_through = builder.current_open in
     if else_falls_through then
       end_block builder (Tac.Jmp join_lbl);

     if then_falls_through || else_falls_through then
       start_block builder join_lbl

let new_builder program =
  { program
  ; next_temp = 0
  ; next_label = 0
  ; current_label = 0
  ; current_instrs_rev = []
  ; current_open = false
  ; completed_blocks_rev = []
  ; bindings = Hashtbl.create 16
  }

let linkage_of_stmt = function
  | Stmt.Internal -> Tac.Internal
  | Stmt.Export -> Tac.Export
  | Stmt.Extern -> Tac.Extern
  | Stmt.Extern_Export -> Tac.Extern_Export

let lower_proc program ({sym; linkage; params; rty; body; _} : Stmt.proc) =
  let symbol = require_symbol "procedure declaration" sym in
  let linkage = linkage_of_stmt linkage in
  let params =
    List.map (fun (param : Stmt.parameter) ->
        require_symbol "procedure parameter" param.sym) params
  in
  if Tac.is_extern linkage then
    Tac.
    { sym = symbol
    ; linkage
    ; params
    ; return_type = rty
    ; entry = None
    ; blocks = []
    }
  else
    let body = match body with
      | Some body -> body
      | None -> failwith "lowering invariant violated: internal procedure has no body"
    in
  let builder = new_builder program in
  let entry = new_label builder in
  start_block builder entry;

  List.iter
    (fun symbol ->
      let dst = Tac.Local symbol in
      bind_symbol builder symbol dst;
      emit builder (Tac.Store {ty = symbol.ty; src = Tac.Param symbol; dst}))
    params;

  lower_stmt builder body;

  if builder.current_open then
    (match rty with
     | Type.Void ->
        end_block builder (Tac.Ret None)
     | _ ->
        failwith @@
          Printf.sprintf "procedure `%s' reaches its end without returning `%s'"
            symbol.name
            (Type.to_string rty));

  Tac.
  { sym = symbol
  ; linkage
  ; params
  ; return_type = rty
  ; entry = Some entry
  ; blocks = List.rev builder.completed_blocks_rev
  }

let lower stmts =
  let program =
    { strings = Hashtbl.create 16
    ; next_string_id = 0
    ; strings_rev = []
    }
  in
  let procs =
    List.map
      (function
       | Stmt.Proc proc -> lower_proc program proc
       | _ ->
          failwith @@ "top-level TAC lowering currently accepts only procedure declarations") stmts
  in
  Tac.
  { strings = List.rev program.strings_rev
  ; procs
  }
