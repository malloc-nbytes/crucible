type temp = int
type label = int
type string_id = int

type operand =
  | Void
  | Temp of temp
  | Proc of Symbol.t
  | Param of Symbol.t
  | I32 of int
  | String of string_id

type binop =
  | Add
  | Sub
  | Mul
  | Div

type instruction =
  | Binop of
      { dst : temp
      ; ty : Type.t
      ; op : binop
      ; lhs : operand
      ; rhs : operand
      }
  | Call of
      { dst : temp option
      ; ty : Type.t
      ; callee : operand
      ; args : operand list
      }

type terminator =
  | Ret of operand option
  | Jmp of label
  | Branch of
      { cond : operand
      ; if_true : label
      ; if_false : label
      }

type block =
  { label : label
  ; instrs : instruction list
  ; terminator : terminator
  }

type linkage =
  | Internal
  | Export
  | Extern
  | Extern_Export

type proc =
  { sym : Symbol.t
  ; linkage : linkage
  ; params : Symbol.t list
  ; return_type : Type.t
  ; entry : label option
  ; blocks : block list
  }

type string_literal =
  { id : string_id
  ; value : string
  }

type program =
  { strings : string_literal list
  ; procs : proc list
  }

let binop_to_string = function
  | Add -> "add"
  | Sub -> "sub"
  | Mul -> "mul"
  | Div -> "Div"

let binop_of_token = function
  | Token.Plus          -> Add
  | Token.Minus         -> Sub
  | Token.Asterisk      -> Mul
  | Token.Forward_Slash -> Div
  | _ -> failwith "unsupported TAC binary operator"

let operand_to_string = function
  | Void      -> "void"
  | Temp id   -> "%" ^ string_of_int id
  | Proc p   -> "@proc" ^ string_of_int p.id
  | Param p   -> "%arg" ^ string_of_int p.id
  | I32 v     -> string_of_int v
  | String id -> "@str" ^ string_of_int id

let instruction_to_string = function
  | Binop {dst; ty; op; lhs; rhs} ->
     Printf.sprintf "%%%d = %s %s %s, %s"
       dst
       (binop_to_string op)
       (Type.to_string ty)
       (operand_to_string lhs)
       (operand_to_string rhs)
  | Call {dst; ty; callee; args} ->
     let result = match dst with
       | Some dst -> Printf.sprintf "%%%d = " dst
       | None -> ""
     in
     Printf.sprintf "%scall %s %s(%s)"
       result
       (Type.to_string ty)
       (operand_to_string callee)
       (args |> List.map operand_to_string |> String.concat ", ")

let terminator_to_string = function
  | Ret None -> "ret"
  | Ret (Some v) ->
     "ret " ^ operand_to_string v
  | Jmp lbl ->
     "jmp L" ^ string_of_int lbl
  | Branch {cond; if_true; if_false} ->
     Printf.sprintf "branch %s, L%d, L%d"
       (operand_to_string cond)
       if_true
       if_false

let block_to_string blk =
  let instructions =
    List.map (fun i -> "  " ^ instruction_to_string i) blk.instrs
  in
  String.concat "\n"
    (("L" ^ string_of_int blk.label ^ ":") ::
       instructions @
       ["  " ^ terminator_to_string blk.terminator])

let linkage_to_string = function
  | Internal -> ""
  | Export -> "export "
  | Extern -> "extern "
  | Extern_Export -> "extern export "

let is_extern = function
  | Extern | Extern_Export -> true
  | Internal | Export -> false

let proc_to_string proc =
  let params =
    proc.params
    |> List.map (fun (sym : Symbol.t) ->
           "%arg" ^ string_of_int sym.id ^ ": " ^ Type.to_string sym.ty)
    |> String.concat ", "
  in
  if is_extern proc.linkage then
    Printf.sprintf "%sproc %s(%s%s): %s;"
      (linkage_to_string proc.linkage)
      proc.sym.name
      params
      (match proc.sym.ty with Proc {variadic = true; _} -> " ... " | _ -> "")
      (Type.to_string proc.return_type)
  else
    let blks =
      proc.blocks
      |> List.map block_to_string
      |> String.concat "\n"
    in
    Printf.sprintf "%sproc %s(%s%s): %s {\n%s\n}"
      (linkage_to_string proc.linkage)
      proc.sym.name
      params
      (match proc.sym.ty with Proc {variadic = true; _} -> ", ..." | _ -> "")
      (Type.to_string proc.return_type)
      blks

let program_to_string prog =
  let strings = List.map (
                    fun s -> Printf.sprintf "@str%d = \"%s\""
                               s.id (String.escaped s.value)
                  ) prog.strings
  in
  let procs = prog.procs |> List.map proc_to_string in
  String.concat "\n\n" (strings @ procs)
