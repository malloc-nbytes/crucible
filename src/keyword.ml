type t =
  | Proc
  | Let
  | If
  | Else
  | Export
  | Return

let to_string = function
  | Proc -> "proc"
  | Let -> "let"
  | If -> "if"
  | Else -> "else"
  | Export -> "export"
  | Return -> "return"

let of_string = function
  | "proc" -> Some Proc
  | "let" -> Some Let
  | "if" -> Some If
  | "else" -> Some Else
  | "export" -> Some Export
  | "return" -> Some Return
  | _ -> None
