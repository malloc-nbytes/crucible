exception Expect of Location.t * string * string
exception Out_Of_Tokens
exception Invalid_Type of Location.t * string
exception Invalid_Primary_Expression of Location.t
exception Illegal_Statement of Location.t
exception Incompatible_Types of Location.t * Type.t * Type.t
exception Incompatible_Binop_Types of Location.t * Type.t * Token.kind * Type.t
exception Invalid_Call_Target of Location.t * Type.t
exception Invalid_Argument_Count of Location.t * int * int
exception Identifier_Not_Defined of Location.t * string
exception Identifier_Already_Defined of Location.t * string
exception Invalid_Assignment_Target of Location.t
exception Empty_Array_Literal of Location.t
exception Invalid_Index_Target of Location.t * Type.t
exception Invalid_Index_Type of Location.t * Type.t
