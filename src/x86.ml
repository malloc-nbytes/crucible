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
  | Cqo
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
  | Cqo -> "  cqo"
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

let emit_function ~public ~name ~frame body =
  let linkage = if public then [".globl " ^ name] else [] in
  let instructions = Label name :: prologue frame @ body @ epilogue frame in
  String.concat "\n"
    (".intel_syntax noprefix" :: ".text" :: "" :: linkage @
       List.map instruction_to_string instructions @
         [""; ".section .note.GNU-stack,\"\",@progbits"])
