type t =
  | Proc
  | Let
  | If
  | Else
  | While
  | Export
  | Extern
  | Return

let to_string = function
  | Proc -> "proc"
  | Let -> "let"
  | If -> "if"
  | Else -> "else"
  | While -> "while"
  | Export -> "export"
  | Extern -> "extern"
  | Return -> "return"

let of_string = function
  | "proc" -> Some Proc
  | "let" -> Some Let
  | "if" -> Some If
  | "else" -> Some Else
  | "while" -> Some While
  | "export" -> Some Export
  | "extern" -> Some Extern
  | "return" -> Some Return
  | _ -> None
