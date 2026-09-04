type kind =
  | Identifier
  | Keyword of Keyword.t
  | Type
  | Integer_Literal
  | String_Literal
  | L_Paren
  | R_Paren
  | Colon
  | Semicolon
  | Equals
  | L_Curly
  | R_Curly
  | L_Sqr
  | R_Sqr
  | Greaterthan
  | Lessthan
  | Bang
  | Bang_Equals
  | Double_Equals
  | Greaterthan_Equals
  | Lessthan_Equals
  | Pipe
  | Pipe_Equals
  | Double_Pipe
  | Ampersand
  | Ampersand_Equals
  | Double_Ampersand
  | Plus
  | Plus_Equals
  | Minus
  | Minus_Equals
  | Asterisk
  | Asterisk_Equals
  | Forward_Slash
  | Forward_Slash_Equals
  | Percent
  | Percent_Equals
  | Uptick
  | Uptick_Equals
  | Comma
  | Triple_Period
  | Dot

type t =
  { lx : string
  ; k : kind
  ; loc : Location.t
  }

let kind_to_string = function
  | Identifier -> "Identifier"
  | Keyword kw -> "Keyword(" ^ Keyword.to_string kw ^ ")"
  | Type -> "Type"
  | Integer_Literal -> "Integer_Literal"
  | String_Literal -> "String_Literal"
  | L_Paren -> "L_Paren"
  | R_Paren -> "R_Paren"
  | Colon -> "Colon"
  | Semicolon -> "Semicolon"
  | Equals -> "Equals"
  | L_Curly -> "L_Curly"
  | R_Curly -> "R_Curly"
  | L_Sqr -> "L_Sqr"
  | R_Sqr -> "R_Sqr"
  | Greaterthan -> "Greaterthan"
  | Lessthan -> "Lessthan"
  | Bang -> "Bang"
  | Bang_Equals -> "Bang_Equals"
  | Double_Equals -> "Double_Equals"
  | Greaterthan_Equals -> "Greaterthan_Equals"
  | Lessthan_Equals -> "Lessthan_Equals"
  | Pipe -> "Pipe"
  | Pipe_Equals -> "Pipe_Equals"
  | Double_Pipe -> "Double_Pipe"
  | Ampersand -> "Ampersand"
  | Ampersand_Equals -> "Ampersand_Equals"
  | Double_Ampersand -> "Double_Ampersand"
  | Plus -> "Plus"
  | Plus_Equals -> "Plus_Equals"
  | Minus -> "Minus"
  | Minus_Equals -> "Minus_Equals"
  | Asterisk -> "Asterisk"
  | Asterisk_Equals -> "Asterisk_Equals"
  | Forward_Slash -> "Forward_Slash"
  | Forward_Slash_Equals -> "Forward_Slash_Equals"
  | Percent -> "Percent"
  | Percent_Equals -> "Percent_Equals"
  | Uptick -> "Uptick"
  | Uptick_Equals -> "Uptick_Equals"
  | Comma -> "Comma"
  | Triple_Period -> "Triple_Period"
  | Dot -> "Dot"


let to_string t =
  Printf.sprintf
    "<lx = `%s', k = `%s', loc = %s>"
    t.lx
    (kind_to_string t.k)
    (Location.to_string t.loc)
