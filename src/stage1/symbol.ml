type kind =
  | Local
  | Param
  | Proc

type t =
  { id : int
  ; name : string
  ; ty : Type.t
  ; kind : kind
  ; loc : Location.t
  }
