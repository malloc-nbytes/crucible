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

let rec check t t' =
  if t <> t' then
    false
  else
    match t, t' with
    | Ptr p, Ptr p' -> check p p'
    | _ -> true

let rec size_bytes = function
  | U8 -> 1
  | I32 | U32 -> 4
  | I64 | U64 | Ptr _ -> 8
  | Array (element_type, length) -> size_bytes element_type * length
  | Void | Undefined | Proc _ ->
     invalid_arg "type has no runtime size"

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

