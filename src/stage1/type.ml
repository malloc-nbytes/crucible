type t =
  | Undefined
  | Void
  | U8
  | I32
  | I64
  | U32
  | U64
  | Ptr of t
  | Proc of
      { rty : t
      ; ptys : t list
      ; variadic : bool
      }
  | Array of t * int
  | Custom of string
  | Struct of struct_

and struct_field =
  { name : string
  ; ty : t
  ; offset : int
  }

and struct_ =
  { name : string
  ; fields : struct_field list
  ; size : int
  }

let is_primitive = function
  | "void" | "u8" | "i32" | "i64" | "u32" | "u64" -> true
  | _ -> false

let from = function
  | "void" -> Some Void
  | "u8" -> Some U8
  | "i32" -> Some I32
  | "i64" -> Some I64
  | "u32" -> Some U32
  | "u64" -> Some U64
  | _ -> None

let rec to_string = function
  | Undefined -> "undefined"
  | Void -> "void"
  | U8 -> "u8"
  | I32 -> "i32"
  | I64 -> "i64"
  | U32 -> "u32"
  | U64 -> "u64"
  | Ptr t -> "ptr(" ^ to_string t ^ ")"
  | Proc {rty; ptys} ->
     Printf.sprintf "%s proc(%s)" (to_string rty)
       (List.map to_string ptys |> String.concat ", ")
  | Array (inner, len) ->
     Printf.sprintf "array(%s, %d)" (to_string inner) len
  | Custom name -> name
  | Struct {name; _} -> name

let rec check t t' =
  if t <> t' then
    false
  else
    match t, t' with
    | Ptr p, Ptr p' -> check p p'
    | Struct {name; _}, Struct {name = name'; _} -> name = name'
    | _ -> true

let is_integer = function
  | U8 | I32 | I64 | U32 | U64 -> true
  | Undefined | Void | Ptr _ | Proc _ | Array _ | Custom _ | Struct _ -> false

let is_castable source target =
  match source, target with
  | source, target when is_integer source && is_integer target -> true
  | Ptr _, Ptr _ -> true
  | Array _, Ptr _ -> true
  | _ -> false

let rec size_bytes = function
  | U8 -> 1
  | I32 | U32 -> 4
  | I64 | U64 | Ptr _ -> 8
  | Array (element_type, length) -> size_bytes element_type * length
  | Struct {size; _} -> size
  | Void | Undefined | Proc _ | Custom _ ->
     invalid_arg "type has no runtime size"

let align_up value alignment =
  ((value + alignment - 1) / alignment) * alignment

let rec alignment = function
  | U8 -> 1
  | I32 | U32 -> 4
  | I64 | U64 | Ptr _ -> 8
  | Array (ty, _) -> alignment ty
  | Struct {size; _} -> min 8 size
  | Void | Undefined | Proc _ | Custom _ ->
     invalid_arg "type has no runtime alignment"

let make_struct name fields =
  let offset, fields, max_alignment =
    List.fold_left
      (fun (offset, fields, max_alignment) (field_name, field_ty) ->
        let field_alignment = alignment field_ty in
        let offset = align_up offset field_alignment in
        offset + size_bytes field_ty,
        {name = field_name; ty = field_ty; offset} :: fields,
        max max_alignment field_alignment)
      (0, [], 1)
      fields
  in
  Struct
    { name
    ; fields = List.rev fields
    ; size = align_up offset max_alignment
    }

let field_by_name name = function
  | Struct {fields; _} -> List.find_opt (fun field -> field.name = name) fields
  | _ -> None

let compound_assignment_binop = function
  | Token.Plus_Equals          -> Some Token.Plus
  | Token.Minus_Equals         -> Some Token.Minus
  | Token.Asterisk_Equals      -> Some Token.Asterisk
  | Token.Forward_Slash_Equals -> Some Token.Forward_Slash
  | Token.Percent_Equals       -> Some Token.Percent
  | Token.Pipe_Equals          -> Some Token.Pipe
  | Token.Ampersand_Equals     -> Some Token.Ampersand
  | Token.Uptick_Equals        -> Some Token.Uptick
  | _ -> None

let is_assignment = function
  | Token.Equals -> true
  | op -> Option.is_some @@ compound_assignment_binop op

let is_comparison = function
  | Token.Lessthan | Token.Greaterthan
  | Token.Lessthan_Equals | Token.Greaterthan_Equals
  | Token.Double_Equals | Token.Bang_Equals -> true
  | _ -> false

let binop_check t (op : Token.kind) t' =
  if not @@ check t t' then false
  else
    match op with
    | (Token.Plus     | Token.Minus |
       Token.Asterisk | Token.Forward_Slash | Token.Percent |
       Token.Pipe | Token.Ampersand | Token.Uptick |
       Token.Double_Equals | Token.Bang_Equals | Token.Equals
       | Token.Lessthan | Token.Greaterthan
       | Token.Lessthan_Equals | Token.Greaterthan_Equals) -> true
    | op when Option.is_some @@ compound_assignment_binop op -> true
    | _ -> false

