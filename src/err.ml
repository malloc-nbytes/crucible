exception Expect of Location.t * string * string
exception Out_Of_Tokens
exception Invalid_Type of Location.t * string
exception Invalid_Primary_Expression of Location.t
exception Illegal_Statement of Location.t
exception Incompatible_Types of Location.t * Type.t * Type.t
exception Incompatible_Binop_Types of Location.t * Type.t * Token.kind * Type.t
