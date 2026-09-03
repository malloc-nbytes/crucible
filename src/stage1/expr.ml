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
  | Unary of unary
  | Cast of cast
  | Index of index
  | Array of array_

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

and unary =
  { node : node
  ; op : Token.t
  ; rhs : t
  }

and cast =
  { node : node
  ; target : Type.t
  ; rhs : t
  }

and index =
  { node : node
  ; lhs : t
  ; idx : t
  }

and array_ =
  { node : node
  ; exprs : t list
  }

let get_location = function
  | Integer    e -> e.node.loc
  | Identifier e -> e.node.loc
  | String     e -> e.node.loc
  | Binary     e -> e.node.loc
  | Call       e -> e.node.loc
  | Unary      e -> e.node.loc
  | Cast       e -> e.node.loc
  | Index      e -> e.node.loc
  | Array      e -> e.node.loc

let get_type = function
  | Integer    e -> e.node.ty
  | Identifier e -> e.node.ty
  | String     e -> e.node.ty
  | Binary     e -> e.node.ty
  | Call       e -> e.node.ty
  | Unary      e -> e.node.ty
  | Cast       e -> e.node.ty
  | Index      e -> e.node.ty
  | Array      e -> e.node.ty
