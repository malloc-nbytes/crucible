type node =
  { loc : Location.t
  ; ty : Type.t
  }

type t =
  | Integer of integer
  | Identifier of identifier
  | String of string_
  | Binary of binary

and integer =
  { node : node
  ; i : Token.t
  }

and identifier =
  { node : node
  ; id : Token.t
  ; sym : Symbol.t option
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

let get_location = function
  | Integer    e -> e.node.loc
  | Identifier e -> e.node.loc
  | String     e -> e.node.loc
  | Binary     e -> e.node.loc

