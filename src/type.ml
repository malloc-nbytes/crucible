type t =
  | Undefined
  | Void
  | U8
  | I32
  | Ptr of t

let is_primitive = function
  | "void" | "u8" | "i32" -> true
  | _ -> false
