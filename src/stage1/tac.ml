type temp = int
type label = int
type string_id = int

type operand =
  | Void
  | Temp of temp
  | Proc of Symbol.t
  | Param of Symbol.t
  | Local of Symbol.t
  | I32 of int
  | String of string_id

type binop =
  | Add
  | Sub
  | Mul
  | Div
  | Mod
  | Or
  | And
  | Xor
  | Less
  | Greater
  | Less_equal
  | Greater_equal
  | Equal
  | Not_equal

type instruction =
  | Binop of
      { dst : temp
      ; ty : Type.t
      ; result_ty : Type.t
      ; op : binop
      ; lhs : operand
      ; rhs : operand
      }
  | Load of
      { dst : temp
      ; ty : Type.t
      ; src : operand
      }
  | Store of
      { ty : Type.t
      ; src : operand
      ; dst : operand
      }
  | Call of
      { dst : temp option
      ; ty : Type.t
      ; callee : operand
      ; args : operand list
      }
  | Address_of of
      { dst : temp
      ; ty : Type.t
      ; src : operand
      }
  | Index_address of
      { dst : temp
      ; element_type : Type.t
      ; src : operand
      ; idx : operand
      }
  | Deref_load of
      { dst : temp
      ; element_type : Type.t
      ; src : operand
      }
  | Deref_store of
      { element_type : Type.t
      ; src : operand
      ; dst : operand
      }
  | Array of
      { dst : operand
      ; element_type : Type.t
      ; elements : operand list
      }
  | Index_load of
      { dst : temp
      ; element_type : Type.t
      ; src : operand
      ; idx : operand
      }
  | Index_store of
      { element_type : Type.t
      ; src : operand
      ; idx : operand
      ; dst : operand
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
  | Div -> "div"
  | Mod -> "mod"
  | Or -> "or"
  | And -> "and"
  | Xor -> "xor"
  | Less -> "lt"
  | Greater -> "gt"
  | Less_equal -> "le"
  | Greater_equal -> "ge"
  | Equal -> "eq"
  | Not_equal -> "ne"

let binop_of_token = function
  | Token.Plus          -> Add
  | Token.Minus         -> Sub
  | Token.Asterisk      -> Mul
  | Token.Forward_Slash -> Div
  | Token.Percent       -> Mod
  | Token.Pipe          -> Or
  | Token.Ampersand     -> And
  | Token.Uptick        -> Xor
  | Token.Lessthan          -> Less
  | Token.Greaterthan       -> Greater
  | Token.Lessthan_Equals   -> Less_equal
  | Token.Greaterthan_Equals -> Greater_equal
  | Token.Double_Equals     -> Equal
  | Token.Bang_Equals       -> Not_equal
  | _ -> failwith "unsupported TAC binary operator"

let operand_to_string = function
  | Void      -> "void"
  | Temp id   -> "%" ^ string_of_int id
  | Proc p   -> "@proc" ^ string_of_int p.id
  | Param p  -> "%arg" ^ string_of_int p.id
  | Local p  -> "%local" ^ string_of_int p.id
  | I32 v    -> string_of_int v
  | String id -> "@str" ^ string_of_int id

let instruction_to_string = function
  | Binop {dst; ty; op; lhs; rhs; _} ->
     Printf.sprintf "%%%d = %s %s %s, %s"
       dst
       (binop_to_string op)
       (Type.to_string ty)
       (operand_to_string lhs)
       (operand_to_string rhs)
  | Load {dst; ty; src} ->
     Printf.sprintf "%%%d = load %s %s"
       dst
       (Type.to_string ty)
       (operand_to_string src)
  | Store {ty; src; dst} ->
     Printf.sprintf "store %s %s, %s"
       (Type.to_string ty)
       (operand_to_string src)
       (operand_to_string dst)
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
  | Address_of {dst; ty; src} ->
     Printf.sprintf "%%%d = address_of %s %s"
       dst (Type.to_string ty) (operand_to_string src)
  | Index_address {dst; element_type; src; idx} ->
     Printf.sprintf "%%%d = index_address %s %s, %s"
       dst (Type.to_string element_type)
       (operand_to_string src) (operand_to_string idx)
  | Deref_load {dst; element_type; src} ->
     Printf.sprintf "%%%d = deref_load %s %s"
       dst (Type.to_string element_type) (operand_to_string src)
  | Deref_store {element_type; src; dst} ->
     Printf.sprintf "deref_store %s %s, %s"
       (Type.to_string element_type)
       (operand_to_string src) (operand_to_string dst)
  | Array {dst; element_type; elements} ->
     Printf.sprintf "array %s {%s}, %s"
       (Type.to_string element_type)
       (elements |> List.map operand_to_string |> String.concat ", ")
       (operand_to_string dst)
  | Index_load {dst; element_type; src; idx} ->
     Printf.sprintf "%%%d = index_load %s %s, %s"
       dst (Type.to_string element_type)
       (operand_to_string src) (operand_to_string idx)
  | Index_store {element_type; src; idx; dst} ->
     Printf.sprintf "index_store %s %s, %s, %s"
       (Type.to_string element_type)
       (operand_to_string src) (operand_to_string idx) (operand_to_string dst)

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
