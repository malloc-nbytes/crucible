open Token

type t =
  { ts : Token.t list
  ; global : bool
  }

let expect k p =
  match p.ts with
  | [] -> raise Err.Out_Of_Tokens
  | {k = k'; _} as hd :: tl when k' = k -> hd, p
  | {k = k'; loc; _} :: _ ->
     raise @@
       Err.Expect (loc,
                   Token.kind_to_string k,
                   Token.kind_to_string k')

let expect' k p = snd @@ expect k p

let parse_expr p = assert false

let parse_type p =
  let base, p = expect Type p in
  let ty, p =
    let rec aux base p =
      match p.ts with
      | {k = Asterisk; _} :: tl ->
         aux (Type.Ptr base) p
      | _ -> base, p
    in aux (match Type.from base.lx with
            | Some t -> t
            | _ -> raise (Err.Invalid_Type (base.loc, base.lx)))
         p
  in
  ty, p

let rec parse_stmt_expr p =
  let e, p = parse_expr p in
  let p = expect' Semicolon p in
  Stmt.Expr {node = {loc = e.loc}; e}, p

and parse_stmt_if p =
  let loc, p =
    let hd, p = expect (Keyword If) p in
    hd.loc, p in
  let cond, p = parse_expr p in
  let then_, p = parse_stmt p in
  let else_, p =
    match p.ts with
    | {k = Keyword Else; _} :: tl ->
       let s, p = parse_stmt {p with ts = tl} in
       Some s, p
    | _ -> None, p
  in
  Stmt.If {node = {loc}; cond; then_; else_}, p

and parse_stmt_block p =
  let rec aux acc p =
    match p.ts with
    | {k = R_Curly; _} :: tl -> List.rev acc, p
    | _ ->
       let s, p = parse_stmt p in
       match p.ts with
       | {k = Comma; _} :: tl ->
          aux (s :: acc) {p with ts = tl}
       | _ ->
          let p = expect' R_Curly p in
          List.rev acc, p
  in
  let loc, p =
    let hd, p = expect L_Curly p in
    hd.loc, p in
  let stmts, p = aux [] p in
  Stmt.Block {node = {loc}; stmts}, p

and parse_stmt_let p =
  let loc, p =
    let hd, p = expect (Keyword Let) p in
    hd.loc, p in
  let id, p = expect Identifier p in
  let p = expect' Semicolon p in
  let ty, p = parse_type p in
  let p = expect' Equals p in
  let e, p = parse_expr p in
  let p = expect' Semicolon p in
  Stmt.Let {node = {loc}; id; ty; e; sym = None}, p

and parse_stmt_return p =
  let loc, p =
    let hd, p = expect (Keyword Return) p in
    hd.loc, p in
  let e, p = match p.ts with
    | {k = Semicolon; _} :: _ -> None, p
    | _ ->
       let e, p = parse_expr p in
       Some e, p
  in
  let p = expect' Semicolon p in
  Stmt.Return {node = {loc}; e}, p

and parse_stmt_proc p =
  let rec aux acc p =
    match p.ts with
    | {k = R_Paren; _} :: tl ->
       List.rev acc, {p with ts = tl}
    | _ ->
       let id, p = expect Identifier p in
       let p = expect' Colon p in
       let ty, p = parse_type p in
       let param = Stmt.{id; ty; sym = None} in
       let acc = param :: acc in
       match p.ts with
       | {k = Comma; _} :: tl ->
          aux acc {p with ts = tl}
       | _ ->
          let p = expect' R_Curly p in
          List.rev acc, p
  in
  let export, loc, p =
    match p.ts with
    | {k = Keyword Export; _} as hd :: tl ->
       true, hd.loc, {p with ts = tl}
    | _ ->
       let hd, p = expect (Keyword Proc) p in
       false, hd.loc, p
  in
  let id, p = expect Identifier p in
  let params, p = aux [] (expect' L_Paren p) in
  let p = expect' Colon p in
  let rty, p = parse_type p in
  let body, p = parse_stmt p in
  Stmt.Proc
    { node = {loc}
    ; export
    ; id
    ; params
    ; rty
    ; body
    ; sym = None
    },
  p

and parse_stmt p =
  match p.ts with
  | [] -> raise Err.Out_Of_Tokens
  | ({k = Keyword Export; _} :: {k = Keyword Proc; _} :: _)
    | ({k = Keyword Proc; _} :: _) -> parse_stmt_proc p
  | {k = Keyword Let; _} :: _ -> parse_stmt_let p
  | {k = Keyword If; _} :: _ -> parse_stmt_if p
  | {k = Keyword Return; _} :: _ -> parse_stmt_return p
  | {k = L_Curly; _} :: _ -> parse_stmt_block p
  | _ -> parse_stmt_expr p

let parse ts =
  try
    let rec aux p =
      match p.ts with
      | [] -> []
      | hd :: tl ->
         let s, p = parse_stmt p in
         s :: aux p
    in aux {ts; global = true}
  with
  | Err.Expect (l, e, g) ->
     let _ = Printf.eprintf
               "%s: expected `%s' but got `%s'\n"
               (Location.to_string l) e g in
     failwith "parser error"

  | Err.Out_Of_Tokens ->
     let _ = Printf.eprintf "out of tokens\n" in
     failwith "parser error"

  | Err.Invalid_Type (l, t) ->
      let _ = Printf.eprintf "%s: invalid type `%s'\n"
                (Location.to_string l) t in
      failwith "parser error"
