type t =
  { r : int
  ; c : int
  ; path : string
  }

let to_string l =
  Printf.sprintf "%s:%d:%d" l.path l.r l.c
