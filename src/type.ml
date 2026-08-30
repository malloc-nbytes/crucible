type t =
  | Undefined
  | Void
  | U8
  | I32
  | Ptr of t
  | Proc of
      { rty : t
      ; ptys : t list
      }

let is_primitive = function
  | "void" | "u8" | "i32" -> true
  | _ -> false

let from = function
  | "void" -> Some Void
  | "u8" -> Some U8
  | "i32" -> Some I32
  | _ -> None

let rec to_string = function
  | Undefined -> "Undefined"
  | Void -> "Void"
  | U8 -> "U8"
  | I32 -> "I32"
  | Ptr t -> "Ptr(" ^ to_string t ^ ")"
  | Proc {rty; ptys} ->
     Printf.sprintf "%s Proc(%s)" (to_string rty)
       (List.map to_string ptys |> String.concat ", ")

let rec check t t' =
  if t <> t' then
    false
  else
    match t, t' with
    | Ptr p, Ptr p' -> check p p'
    | _ -> true

let binop_check t (op : Token.kind) t' =
  if not @@ check t t' then false
  else
    match op with
    | (Token.Plus     | Token.Minus |
       Token.Asterisk | Token.Forward_Slash) -> true
    | _ -> false

