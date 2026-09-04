type t =
  | Proc
  | Let
  | If
  | Else
  | While
  | Export
  | Extern
  | Return
  | Struct

let to_string = function
  | Proc -> "proc"
  | Let -> "let"
  | If -> "if"
  | Else -> "else"
  | While -> "while"
  | Export -> "export"
  | Extern -> "extern"
  | Return -> "return"
  | Struct -> "struct"

let of_string = function
  | "proc" -> Some Proc
  | "let" -> Some Let
  | "if" -> Some If
  | "else" -> Some Else
  | "while" -> Some While
  | "export" -> Some Export
  | "extern" -> Some Extern
  | "return" -> Some Return
  | "struct" -> Some Struct
  | _ -> None
