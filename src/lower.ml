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

let rec lower_expr builder = function
  | Expr.Integer {i; _} ->
     Tac.I32 (int_of_string i.lx)

  | Expr.String {s; _} ->
     Tac.String (intern_string builder.program s.lx)

  | Expr.Identifier {sym = Some ({kind = Symbol.Proc; _} as sym); _} ->
     Tac.Proc sym

  | Expr.Identifier {sym; _} ->
     let sym = require_symbol "identifier" sym in
     lookup_symbol builder sym

  | Expr.Binary {lhs; op; rhs; _} as expression ->
     let lhs = lower_expr builder lhs in
     let rhs = lower_expr builder rhs in
     let dst = new_temp builder in
     let ty  = require_resolved_type expression in
     emit builder
       (Tac.Binop
          { dst
          ; ty
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
     match dst with
     | Some dst -> Tac.Temp dst
     | None -> Tac.Void

let rec lower_stmt builder = function
  | Stmt.Proc _ ->
     failwith "nested procedures are not supported by TAC lowering"

  | Stmt.Let {sym; e; _} ->
     let symbol = require_symbol "let declaration" sym in
     let value = lower_expr builder e in
     bind_symbol builder symbol value

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

and lower_stmts builder = function
  | [] -> ()
  | stmt :: stmts ->
     if not builder.current_open then
       failwith "unreachable statement after a terminating statement";
     lower_stmt builder stmt;
     lower_stmts builder stmts

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

  List.iter (fun symbol -> bind_symbol builder symbol (Tac.Param symbol)) params;

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
