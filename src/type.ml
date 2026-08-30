type t =
  | Undefined
  | Void
  | U8
  | I32
  | Ptr of t

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
