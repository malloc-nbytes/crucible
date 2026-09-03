type width =
  | W8
  | W32
  | W64

type register =
  | Rax | Rbx | Rcx | Rdx | Rsi | Rdi | Rbp | Rsp
  | R8  | R9  | R10 | R11 | R12 | R13 | R14 | R15

type operand =
  | Imm    of int64
  | Reg    of register
  | Mem    of {base : register; offset : int}
  | Symbol of string
  | Rip_relative of string

type condition =
  | Equal
  | Not_equal
  | Less
  | Less_equal
  | Greater
  | Greater_equal
  | Below
  | Below_equal
  | Above
  | Above_equal

type instruction =
  | Label         of string
  | Comment       of string
  | Push          of register
  | Pop           of register
  | Mov           of width * operand * operand
  | Lea           of register * operand
  | Add           of width * operand * operand
  | Sub           of width * operand * operand
  | Imul          of width * operand * operand
  | Or            of width * operand * operand
  | And           of width * operand * operand
  | Cdq
  | Cqo
  | Div           of width * operand
  | Idiv          of width * operand
  | Xor           of width * register * register
  | Cmp           of width * operand * operand
  | Test          of width * operand * operand
  | Set           of condition * register
  | Jmp           of string
  | Jcc           of condition * string
  | Call          of string
  | Call_indirect of operand
  | Ret

type frame =
  { local_bytes : int
  ; callee_saved : register list
  ; stack_bytes : int
  }

type argument_location =
  | Argument_register of register
  | Incoming_stack    of int

let align_up value alignment =
  if value < 0 then invalid_arg "x86 frame size cannot be negative";
  ((value + alignment - 1) / alignment) * alignment

let is_callee_saved = function
  | Rbx | Rbp | R12 | R13 | R14 | R15 -> true
  | _ -> false

let make_frame ~local_bytes ~callee_saved =
  List.iter
    (fun reg ->
      if not @@ is_callee_saved reg then
        invalid_arg "x86 frame saves a caller-saved register")
    callee_saved;
  let saved_bytes = 8 * List.length callee_saved in
  let locals = align_up local_bytes 8 in
  let stack_bytes =
    if (saved_bytes + locals) mod 16 = 0 then locals else locals + 8
  in
  {local_bytes; callee_saved; stack_bytes}

let frame_size frame =
  8 + (8 * List.length frame.callee_saved) + frame.stack_bytes

let width_of_type = function
  | Type.U8 -> W8
  | Type.I32 | Type.U32 -> W32
  | Type.I64 | Type.U64 | Type.Ptr _ -> W64
  | Type.Array (t, len) -> assert false
  | Type.Void | Type.Undefined | Type.Proc _ ->
     invalid_arg "x86 value width is undefined for this type"

let integer_argument_register = function
  | 0 -> Some Rdi
  | 1 -> Some Rsi
  | 2 -> Some Rdx
  | 3 -> Some Rcx
  | 4 -> Some R8
  | 5 -> Some R9
  | _ -> None

let argument_location index =
  if index < 0 then invalid_arg "x86 argument index cannot be negative";
  match integer_argument_register index with
  | Some reg -> Argument_register reg
  | None -> Incoming_stack (16 + (8 * (index - 6)))

let return_register = Rax

let register_to_string width = function
  | Rax -> (match width with W8 -> "al"   | W32 -> "eax"  | W64 -> "rax")
  | Rbx -> (match width with W8 -> "bl"   | W32 -> "ebx"  | W64 -> "rbx")
  | Rcx -> (match width with W8 -> "cl"   | W32 -> "ecx"  | W64 -> "rcx")
  | Rdx -> (match width with W8 -> "dl"   | W32 -> "edx"  | W64 -> "rdx")
  | Rsi -> (match width with W8 -> "sil"  | W32 -> "esi"  | W64 -> "rsi")
  | Rdi -> (match width with W8 -> "dil"  | W32 -> "edi"  | W64 -> "rdi")
  | Rbp -> (match width with W8 -> "bpl"  | W32 -> "ebp"  | W64 -> "rbp")
  | Rsp -> (match width with W8 -> "spl"  | W32 -> "esp"  | W64 -> "rsp")
  | R8  -> (match width with W8 -> "r8b"  | W32 -> "r8d"  | W64 -> "r8")
  | R9  -> (match width with W8 -> "r9b"  | W32 -> "r9d"  | W64 -> "r9")
  | R10 -> (match width with W8 -> "r10b" | W32 -> "r10d" | W64 -> "r10")
  | R11 -> (match width with W8 -> "r11b" | W32 -> "r11d" | W64 -> "r11")
  | R12 -> (match width with W8 -> "r12b" | W32 -> "r12d" | W64 -> "r12")
  | R13 -> (match width with W8 -> "r13b" | W32 -> "r13d" | W64 -> "r13")
  | R14 -> (match width with W8 -> "r14b" | W32 -> "r14d" | W64 -> "r14")
  | R15 -> (match width with W8 -> "r15b" | W32 -> "r15d" | W64 -> "r15")

let size_to_string = function
  | W8 -> "byte ptr"
  | W32 -> "dword ptr"
  | W64 -> "qword ptr"

let operand_to_string width = function
  | Imm value -> Int64.to_string value
  | Reg reg -> register_to_string width reg
  | Symbol name -> name
  | Rip_relative name -> "[rip + " ^ name ^ "]"
  | Mem {base; offset = 0} ->
     Printf.sprintf "%s [%s]" (size_to_string width)
       (register_to_string W64 base)
  | Mem {base; offset} ->
     Printf.sprintf "%s [%s %c %d]" (size_to_string width)
       (register_to_string W64 base)
       (if offset < 0 then '-' else '+')
       (abs offset)

let condition_to_string = function
  | Equal         -> "e"
  | Not_equal     -> "ne"
  | Less          -> "l"
  | Less_equal    -> "le"
  | Greater       -> "g"
  | Greater_equal -> "ge"
  | Below         -> "b"
  | Below_equal   -> "be"
  | Above         -> "a"
  | Above_equal   -> "ae"

let instruction_to_string = function
  | Label name -> name ^ ":"
  | Comment text -> "  # " ^ text
  | Push reg -> "  push " ^ register_to_string W64 reg
  | Pop reg -> "  pop " ^ register_to_string W64 reg
  | Mov (width, dst, src) ->
     Printf.sprintf "  mov %s, %s" (operand_to_string width dst) (operand_to_string width src)
  | Lea (dst, src) ->
     Printf.sprintf "  lea %s, %s"
       (register_to_string W64 dst) (operand_to_string W64 src)
  | Add (width, dst, src) ->
     Printf.sprintf "  add %s, %s" (operand_to_string width dst) (operand_to_string width src)
  | Sub (width, dst, src) ->
     Printf.sprintf "  sub %s, %s" (operand_to_string width dst) (operand_to_string width src)
  | Imul (width, dst, src) ->
     Printf.sprintf "  imul %s, %s" (operand_to_string width dst) (operand_to_string width src)
  | Or (width, dst, src) ->
     Printf.sprintf "  or %s, %s" (operand_to_string width dst) (operand_to_string width src)
  | And (width, dst, src) ->
     Printf.sprintf "  and %s, %s" (operand_to_string width dst) (operand_to_string width src)
  | Cdq -> "  cdq"
  | Cqo -> "  cqo"
  | Div (width, src) -> "  div " ^ operand_to_string width src
  | Idiv (width, src) -> "  idiv " ^ operand_to_string width src
  | Xor (width, dst, src) ->
     Printf.sprintf "  xor %s, %s" (register_to_string width dst) (register_to_string width src)
  | Cmp (width, lhs, rhs) ->
     Printf.sprintf "  cmp %s, %s" (operand_to_string width lhs) (operand_to_string width rhs)
  | Test (width, lhs, rhs) ->
     Printf.sprintf "  test %s, %s" (operand_to_string width lhs) (operand_to_string width rhs)
  | Set (condition, dst) ->
     Printf.sprintf "  set%s %s" (condition_to_string condition) (register_to_string W8 dst)
  | Jmp label -> "  jmp " ^ label
  | Jcc (condition, label) -> "  j" ^ condition_to_string condition ^ " " ^ label
  | Call name -> "  call " ^ name
  | Call_indirect operand -> "  call " ^ operand_to_string W64 operand
  | Ret -> "  ret"

let prologue frame =
  Push Rbp
  :: Mov (W64, Reg Rbp, Reg Rsp)
  :: List.map (fun reg -> Push reg) frame.callee_saved
  @ if frame.stack_bytes = 0 then []
    else [Sub (W64, Reg Rsp, Imm (Int64.of_int frame.stack_bytes))]

let epilogue frame =
  (if frame.stack_bytes = 0 then []
   else [Add (W64, Reg Rsp, Imm (Int64.of_int frame.stack_bytes))])
  @ List.rev_map (fun reg -> Pop reg) frame.callee_saved
  @ [Pop Rbp; Ret]

(* let emit_function ~public ~name ~frame body = *)
(*   let linkage = if public then [".globl " ^ name] else [] in *)
(*   let instructions = Label name :: prologue frame @ body @ epilogue frame in *)
(*   String.concat "\n" *)
(*     (".intel_syntax noprefix" :: ".text" :: "" :: linkage @ *)
(*        List.map instruction_to_string instructions @ *)
(*          [""; ".section .note.GNU-stack,\"\",@progbits"]) *)

type layout =
  { slots : (string, int) Hashtbl.t
  ; types : (string, Type.t) Hashtbl.t
  ; param_indices : (int, int) Hashtbl.t
  ; mutable next_slot : int
  }

let slot_key = function
  | Tac.Temp id -> "temp:" ^ string_of_int id
  | Tac.Local sym | Tac.Param sym -> "symbol:" ^ string_of_int sym.id
  | Tac.Void | Tac.Proc _ | Tac.I32 _ | Tac.String _ ->
     invalid_arg "x86 operand has no stack slot"

let reserve_slot layout operand ty =
  match operand with
  | Tac.Temp _ | Tac.Local _ | Tac.Param _ ->
     let key = slot_key operand in
     if not @@ Hashtbl.mem layout.slots key then begin
        let slots = (Type.size_bytes ty + 7) / 8 in
        layout.next_slot <- layout.next_slot + slots;
        Hashtbl.add layout.slots key (-8 * layout.next_slot);
         Hashtbl.add layout.types key ty
       end
  | Tac.Void | Tac.Proc _ | Tac.I32 _ | Tac.String _ -> ()

let type_of_operand layout = function
  | Tac.Temp _ | Tac.Local _ | Tac.Param _ as operand ->
     Hashtbl.find layout.types (slot_key operand)
  | Tac.Void -> Type.Void
  | Tac.Proc sym -> sym.ty
  | Tac.I32 _ -> Type.I32
  | Tac.String _ -> Type.Ptr Type.U8

let reserve_instruction layout = function
  | Tac.Binop {dst; ty; result_ty; lhs; rhs; _} ->
     reserve_slot layout (Tac.Temp dst) result_ty;
     reserve_slot layout lhs ty;
     reserve_slot layout rhs ty
  | Tac.Load {dst; ty; src} ->
     reserve_slot layout (Tac.Temp dst) ty;
     reserve_slot layout src ty
  | Tac.Store {ty; src; dst} ->
     reserve_slot layout src ty;
     reserve_slot layout dst ty
  | Tac.Call {dst; ty; callee; args} ->
     Option.iter (fun dst -> reserve_slot layout (Tac.Temp dst) ty) dst;
     reserve_slot layout callee ty;
     List.iter
       (fun arg -> reserve_slot layout arg (type_of_operand layout arg))
       args
  | Tac.Array {dst; element_type; elements} ->
     reserve_slot layout dst (Type.Array (element_type, List.length elements));
     List.iter (fun element -> reserve_slot layout element element_type) elements
  | Tac.Index_load {dst; element_type; src; idx} ->
     reserve_slot layout (Tac.Temp dst) element_type;
     reserve_slot layout src (type_of_operand layout src);
     reserve_slot layout idx Type.I32
  | Tac.Index_store {element_type; src; idx; dst} ->
     reserve_slot layout src element_type;
     reserve_slot layout idx Type.I32;
     reserve_slot layout dst (type_of_operand layout dst)

let make_layout (proc : Tac.proc) =
  let layout =
    { slots = Hashtbl.create 16
    ; types = Hashtbl.create 16
    ; param_indices = Hashtbl.create 16
    ; next_slot = 0
    }
  in
  List.iteri
    (fun index (sym : Symbol.t) ->
      Hashtbl.add layout.param_indices sym.id index;
      reserve_slot layout (Tac.Local sym) sym.ty)
    proc.params;
  List.iter
    (fun block ->
      List.iter (reserve_instruction layout) block.Tac.instrs;
      match block.terminator with
      | Tac.Ret (Some operand) ->
         reserve_slot layout operand (type_of_operand layout operand)
      | Tac.Ret None | Tac.Jmp _ -> ()
      | Tac.Branch {cond; _} ->
         reserve_slot layout cond (type_of_operand layout cond))
    proc.blocks;
  layout

let stack_operand layout operand =
  Mem {base = Rbp; offset = Hashtbl.find layout.slots (slot_key operand)}

let string_label id = ".Lstr" ^ string_of_int id

let proc_label (proc : Tac.proc) label =
  Printf.sprintf ".L%d_%d" proc.sym.id label

let is_unsigned = function
  | Type.U8 | Type.U32 | Type.U64 -> true
  | Type.I32 | Type.I64 | Type.Ptr _ -> false
  | Type.Array _ -> assert false
  | Type.Void | Type.Undefined | Type.Proc _ ->
     invalid_arg "x86 integer signedness is undefined for this type"

let load_value layout reg operand ty =
  let width = width_of_type ty in
  match operand with
  | Tac.I32 value -> [Mov (width, Reg reg, Imm (Int64.of_int value))]
  | Tac.Temp _ | Tac.Local _ -> [Mov (width, Reg reg, stack_operand layout operand)]
  | Tac.Param sym ->
     let index = Hashtbl.find layout.param_indices sym.id in
     (match argument_location index with
      | Argument_register src -> [Mov (width, Reg reg, Reg src)]
      | Incoming_stack offset ->
         [Mov (width, Reg reg, Mem {base = Rbp; offset})])
  | Tac.String id -> [Lea (reg, Rip_relative (string_label id))]
  | Tac.Proc sym -> [Lea (reg, Rip_relative sym.name)]
  | Tac.Void -> invalid_arg "cannot materialize void"

let store_value layout operand ty reg =
  match operand with
  | Tac.Temp _ | Tac.Local _ ->
     [Mov (width_of_type ty, stack_operand layout operand, Reg reg)]
  | Tac.Param _ | Tac.Void | Tac.Proc _ | Tac.I32 _ | Tac.String _ ->
     invalid_arg "x86 store destination is not a mutable slot"

let array_offset layout operand =
  match operand with
  | Tac.Temp _ | Tac.Local _ -> Hashtbl.find layout.slots (slot_key operand)
  | Tac.Param _ | Tac.Void | Tac.Proc _ | Tac.I32 _ | Tac.String _ ->
     invalid_arg "x86 array operand is not a stack slot"

let element_address layout array index element_type =
  let offset = array_offset layout array in
  let element_size = Type.size_bytes element_type in
  load_value layout Rcx index Type.I32
  @ [ Lea (Rdx, Mem {base = Rbp; offset})
    ; Imul (W64, Reg Rcx, Imm (Int64.of_int element_size))
    ; Add (W64, Reg Rdx, Reg Rcx)
    ]

let emit_binop layout = function
  | Tac.Binop {dst; ty; result_ty; op; lhs; rhs} ->
     let width = width_of_type ty in
     let setup = load_value layout Rax lhs ty @ load_value layout Rcx rhs ty in
     let operation, result =
       match op with
       | Tac.Add -> [Add (width, Reg Rax, Reg Rcx)], Rax
       | Tac.Sub -> [Sub (width, Reg Rax, Reg Rcx)], Rax
       | Tac.Mul -> [Imul (width, Reg Rax, Reg Rcx)], Rax
       | Tac.Or -> [Or (width, Reg Rax, Reg Rcx)], Rax
       | Tac.And -> [And (width, Reg Rax, Reg Rcx)], Rax
       | Tac.Xor -> [Xor (width, Rax, Rcx)], Rax
       | Tac.Less | Tac.Greater | Tac.Less_equal | Tac.Greater_equal
         | Tac.Equal | Tac.Not_equal ->
          let condition =
            match op, is_unsigned ty with
            | Tac.Less, false -> Less
            | Tac.Less, true -> Below
            | Tac.Greater, false -> Greater
            | Tac.Greater, true -> Above
            | Tac.Less_equal, false -> Less_equal
            | Tac.Less_equal, true -> Below_equal
            | Tac.Greater_equal, false -> Greater_equal
            | Tac.Greater_equal, true -> Above_equal
            | Tac.Equal, _ -> Equal
            | Tac.Not_equal, _ -> Not_equal
            | _ -> assert false
          in
          [Xor (W32, Rdx, Rdx); Cmp (width, Reg Rax, Reg Rcx); Set (condition, Rdx)], Rdx
       | Tac.Div | Tac.Mod ->
          let extend = match width, is_unsigned ty with
            | W32, false -> [Cdq]
            | W64, false -> [Cqo]
            | W32, true | W64, true -> [Xor (W32, Rdx, Rdx)]
            | W8, _ -> invalid_arg "x86 division of u8 is not implemented"
          in
          let divide = if is_unsigned ty then Div (width, Reg Rcx) else Idiv (width, Reg Rcx) in
          extend @ [divide], (match op with Tac.Div -> Rax | Tac.Mod -> Rdx | _ -> assert false)
     in
     setup @ operation @ store_value layout (Tac.Temp dst) result_ty result
  | _ -> assert false

let emit_call layout = function
  | Tac.Call {dst; ty; callee; args} ->
     let register_args, stack_args =
       List.mapi (fun index arg -> index, arg) args
       |> List.partition (fun (index, _) -> index < 6)
     in
     let stack_bytes = 8 * List.length stack_args in
     let padding = if stack_bytes mod 16 = 0 then 0 else 8 in
     let reserve_padding =
       if padding = 0 then [] else [Sub (W64, Reg Rsp, Imm (Int64.of_int padding))]
     in
     let push_args =
       List.rev_map
         (fun (_, arg) ->
           let arg_ty = type_of_operand layout arg in
           load_value layout Rax arg arg_ty @ [Push Rax])
         stack_args
       |> List.flatten
     in
     let load_register_args =
       List.map
         (fun (index, arg) ->
           match integer_argument_register index with
           | Some reg -> load_value layout reg arg (type_of_operand layout arg)
           | None -> assert false)
         register_args
       |> List.flatten
     in
     let is_variadic = match callee with
       | Tac.Proc {ty = Type.Proc {variadic; _}; _} -> variadic
       | _ -> false
     in
     let call = match callee with
       | Tac.Proc sym -> [Call sym.name]
       | _ -> load_value layout Rax callee (type_of_operand layout callee) @ [Call_indirect (Reg Rax)]
     in
     let cleanup =
       if stack_bytes + padding = 0 then []
       else [Add (W64, Reg Rsp, Imm (Int64.of_int (stack_bytes + padding)))]
     in
     reserve_padding @ push_args @ load_register_args @
       (if is_variadic then [Xor (W32, Rax, Rax)] else []) @
         call @ cleanup @
           (match dst with
            | None -> []
            | Some dst -> store_value layout (Tac.Temp dst) ty Rax)
  | _ -> assert false

let emit_array layout = function
  | Tac.Array {dst; element_type; elements} ->
     let offset = array_offset layout dst in
     elements
     |> List.mapi
          (fun index element ->
            load_value layout Rax element element_type
            @ [Mov (width_of_type element_type,
                    Mem {base = Rbp; offset = offset + (index * Type.size_bytes element_type)},
                    Reg Rax)])
     |> List.flatten
  | Tac.Index_load {dst; element_type; src; idx} ->
     element_address layout src idx element_type
     @ [Mov (width_of_type element_type, Reg Rax, Mem {base = Rdx; offset = 0})]
     @ store_value layout (Tac.Temp dst) element_type Rax
  | Tac.Index_store {element_type; src; idx; dst} ->
     element_address layout dst idx element_type
     @ load_value layout Rax src element_type
     @ [Mov (width_of_type element_type, Mem {base = Rdx; offset = 0}, Reg Rax)]
  | _ -> assert false

let emit_instruction layout = function
  | Tac.Binop binop -> emit_binop layout (Tac.Binop binop)
  | Tac.Load {dst; ty; src} ->
     load_value layout Rax src ty @ store_value layout (Tac.Temp dst) ty Rax
  | Tac.Store {ty; src; dst} ->
     load_value layout Rax src ty @ store_value layout dst ty Rax
  | Tac.Call call -> emit_call layout (Tac.Call call)
  | Tac.Array array -> emit_array layout (Tac.Array array)
  | Tac.Index_load index_load -> emit_array layout (Tac.Index_load index_load)
  | Tac.Index_store index_store -> emit_array layout (Tac.Index_store index_store)

let emit_terminator layout proc epilogue = function
  | Tac.Ret None -> [Jmp epilogue]
  | Tac.Ret (Some value) ->
     let ty = type_of_operand layout value in
     load_value layout return_register value ty @ [Jmp epilogue]
  | Tac.Jmp label -> [Jmp (proc_label proc label)]
  | Tac.Branch {cond; if_true; if_false} ->
     let ty = type_of_operand layout cond in
     load_value layout Rax cond ty @
       [ Test (width_of_type ty, Reg Rax, Reg Rax)
       ; Jcc (Not_equal, proc_label proc if_true)
       ; Jmp (proc_label proc if_false)
       ]

let emit_proc proc =
  if Tac.is_extern proc.Tac.linkage then []
  else
    let layout = make_layout proc in
    let frame = make_frame ~local_bytes:(8 * layout.next_slot) ~callee_saved:[] in
    let epilogue_label = Printf.sprintf ".L%d_epilogue" proc.sym.id in
    let linkage = match proc.linkage with
      | Tac.Export -> [".globl " ^ proc.sym.name]
      | Tac.Internal -> []
      | Tac.Extern | Tac.Extern_Export -> assert false
    in
    let blocks =
      List.map
        (fun (block : Tac.block) ->
          Label (proc_label proc block.label)
          :: List.concat_map (emit_instruction layout) block.instrs
          @ emit_terminator layout proc epilogue_label block.terminator)
        proc.blocks
      |> List.flatten
    in
    linkage @ List.map instruction_to_string
                (Label proc.sym.name :: prologue frame @ blocks @
                   Label epilogue_label :: epilogue frame)

let generate (program : Tac.program) =
  let text = List.concat_map emit_proc program.procs in
  let strings =
    List.concat_map
      (fun ({id; value} : Tac.string_literal) ->
        [string_label id ^ ":"; "  .asciz \"" ^ String.escaped value ^ "\""])
      program.strings
  in
  String.concat "\n"
    (".intel_syntax noprefix" :: ".text" :: "" :: text @
       (if strings = [] then [] else "" :: ".section .rodata" :: strings) @
         [""; ".section .note.GNU-stack,\"\",@progbits"])
