open Token

let isalpha = function
  | 'a' .. 'z' | 'A' .. 'Z' -> true
  | _ -> false

let isdigit = function
  | '0' .. '9' -> true
  | _ -> false

let isalnum c = isalpha c || isdigit c

let consume_while s pred =
  let rec aux acc = function
    | [] -> List.rev acc, []
    | (hd :: tl) as lst when not @@ pred hd -> List.rev acc, lst
    | hd :: tl -> aux (hd :: acc) tl
  in
  let res, tl = aux [] s in
  res |> List.to_seq |> String.of_seq, tl

let rec print = function
  | [] -> ()
  | hd :: tl ->
     let _ = print_endline @@ Token.to_string hd in
     print tl

let lex path src =
  let rec aux r c = function
    | [] -> []
    | (' ' | '\t') :: tl -> aux r (c+1) tl
    | '\n' :: tl -> aux (r+1) 1 tl
    | '.' :: '.' :: '.' :: tl -> {lx = "..."; k = Triple_Period; loc = {r; c; path}} :: aux r (c+1) tl
    | '.' :: tl -> {lx = "."; k = Dot; loc = {r; c; path}} :: aux r (c+1) tl
    | '!' :: '=' :: tl -> {lx = "!="; k = Bang_Equals;          loc = {r; c; path}} :: aux r (c+1) tl
    | '=' :: '=' :: tl -> {lx = "=="; k = Double_Equals;        loc = {r; c; path}} :: aux r (c+1) tl
    | '>' :: '=' :: tl -> {lx = ">="; k = Greaterthan_Equals;   loc = {r; c; path}} :: aux r (c+1) tl
    | '<' :: '=' :: tl -> {lx = "<="; k = Lessthan_Equals;      loc = {r; c; path}} :: aux r (c+1) tl
    | '|' :: '=' :: tl -> {lx = "|="; k = Pipe_Equals;          loc = {r; c; path}} :: aux r (c+1) tl
    | '|' :: '|' :: tl -> {lx = "||"; k = Double_Pipe;          loc = {r; c; path}} :: aux r (c+1) tl
    | '&' :: '=' :: tl -> {lx = "&="; k = Ampersand_Equals;     loc = {r; c; path}} :: aux r (c+1) tl
    | '&' :: '&' :: tl -> {lx = "&&"; k = Double_Ampersand;     loc = {r; c; path}} :: aux r (c+1) tl
    | '+' :: '=' :: tl -> {lx = "+="; k = Plus_Equals;          loc = {r; c; path}} :: aux r (c+1) tl
    | '-' :: '=' :: tl -> {lx = "-="; k = Minus_Equals;         loc = {r; c; path}} :: aux r (c+1) tl
    | '*' :: '=' :: tl -> {lx = "*="; k = Asterisk_Equals;      loc = {r; c; path}} :: aux r (c+1) tl
    | '/' :: '=' :: tl -> {lx = "/="; k = Forward_Slash_Equals; loc = {r; c; path}} :: aux r (c+1) tl
    | '%' :: '=' :: tl -> {lx = "%="; k = Percent_Equals;       loc = {r; c; path}} :: aux r (c+1) tl
    | '^' :: '=' :: tl -> {lx = "^="; k = Uptick_Equals;        loc = {r; c; path}} :: aux r (c+1) tl
    | ',' :: tl        -> {lx = ",";  k = Comma;                loc = {r; c; path}} :: aux r (c+1) tl
    | '^' :: tl        -> {lx = "^";  k = Uptick;               loc = {r; c; path}} :: aux r (c+1) tl
    | '%' :: tl        -> {lx = "%";  k = Percent;              loc = {r; c; path}} :: aux r (c+1) tl
    | '/' :: tl        -> {lx = "/";  k = Forward_Slash;        loc = {r; c; path}} :: aux r (c+1) tl
    | '*' :: tl        -> {lx = "*";  k = Asterisk;             loc = {r; c; path}} :: aux r (c+1) tl
    | '-' :: tl        -> {lx = "-";  k = Minus;                loc = {r; c; path}} :: aux r (c+1) tl
    | '+' :: tl        -> {lx = "+";  k = Plus;                 loc = {r; c; path}} :: aux r (c+1) tl
    | '&' :: tl        -> {lx = "&";  k = Ampersand;            loc = {r; c; path}} :: aux r (c+1) tl
    | '|' :: tl        -> {lx = "|";  k = Pipe;                 loc = {r; c; path}} :: aux r (c+1) tl
    | '(' :: tl        -> {lx = "(";  k = L_Paren;              loc = {r; c; path}} :: aux r (c+1) tl
    | ')' :: tl        -> {lx = ")";  k = R_Paren;              loc = {r; c; path}} :: aux r (c+1) tl
    | ':' :: tl        -> {lx = ":";  k = Colon;                loc = {r; c; path}} :: aux r (c+1) tl
    | ';' :: tl        -> {lx = ";";  k = Semicolon;            loc = {r; c; path}} :: aux r (c+1) tl
    | '=' :: tl        -> {lx = "=";  k = Equals;               loc = {r; c; path}} :: aux r (c+1) tl
    | '{' :: tl        -> {lx = "{";  k = L_Curly;              loc = {r; c; path}} :: aux r (c+1) tl
    | '}' :: tl        -> {lx = "}";  k = R_Curly;              loc = {r; c; path}} :: aux r (c+1) tl
    | '[' :: tl        -> {lx = "[";  k = L_Sqr;                loc = {r; c; path}} :: aux r (c+1) tl
    | ']' :: tl        -> {lx = "]";  k = R_Sqr;                loc = {r; c; path}} :: aux r (c+1) tl
    | '>' :: tl        -> {lx = ">";  k = Greaterthan;          loc = {r; c; path}} :: aux r (c+1) tl
    | '<' :: tl        -> {lx = "<";  k = Lessthan;             loc = {r; c; path}} :: aux r (c+1) tl
    | '!' :: tl        -> {lx = "!";  k = Bang;                 loc = {r; c; path}} :: aux r (c+1) tl
    | (hd :: tl) as lst when isalpha hd || hd = '_' ->
       let lx, tl = consume_while lst (fun c -> isalnum c || c = '_') in
       let k = match Keyword.of_string lx with
         | Some kw -> Token.Keyword kw
         | None -> match Type.is_primitive lx with
                   | true -> Type
                   | false -> Identifier in
       {lx; k; loc = {r; c; path}} :: aux r (c + String.length lx) tl
    | (hd :: tl) as lst when isdigit hd ->
       let lx, tl = consume_while lst isdigit in
       {lx; k = Integer_Literal; loc = {r; c; path}} :: aux r (c + String.length lx) tl
    | '"' :: tl ->
       let lx, tl = consume_while tl (fun c -> c <> '"') in
       {lx; k = String_Literal; loc = {r; c; path}} :: aux r (c + (String.length lx) + 2) (List.tl tl)
    | _ -> assert false
  in aux 1 1 (String.to_seq src |> List.of_seq)
