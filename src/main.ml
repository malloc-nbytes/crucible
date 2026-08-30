let () =
  let path = "1.cruc" in
  let src = Io.load_file path in
  let ts = Lexer.lex path src in
  let stmts = Parser.parse ts in
  let stmts = Resolver.analyze stmts in
  let tac = Lower.lower stmts in
  print_endline @@ Tac.program_to_string tac
