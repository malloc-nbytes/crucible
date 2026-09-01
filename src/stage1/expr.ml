type node =
  { loc : Location.t
  ; ty : Type.t
  }

type t =
  | Integer of integer
  | Identifier of identifier
  | String of string_
  | Binary of binary
  | Call of call

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

and call =
  { node : node
  ; lhs : t
  ; args : t list
  }

let get_location = function
  | Integer    e -> e.node.loc
  | Identifier e -> e.node.loc
  | String     e -> e.node.loc
  | Binary     e -> e.node.loc
  | Call       e -> e.node.loc

let get_type = function
  | Integer    e -> e.node.ty
  | Identifier e -> e.node.ty
  | String     e -> e.node.ty
  | Binary     e -> e.node.ty
  | Call       e -> e.node.ty
