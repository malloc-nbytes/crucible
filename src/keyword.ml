type t =
  | Proc
  | Let
  | If
  | Else

let to_string = function
  | Proc -> "proc"
  | Let -> "let"
  | If -> "if"
  | Else -> "else"

let of_string = function
  | "proc" -> Some Proc
  | "let" -> Some Let
  | "if" -> Some If
  | "else" -> Some Else
  | _ -> None
