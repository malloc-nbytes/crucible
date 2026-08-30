type node =
  { loc : Location.t
  ; ty : Type.t
  }

type t =
  | Integer of integer
  | String of string_
  | Binary of binary

and integer =
  { node : node
  ; i : Token.t
  }

and string_ =
  { node : node
  ; s : Token.t
  }

and binary =
  { node : node
  ; lhs : t
  ; op : Token.t
  ; rhs : t
  }
