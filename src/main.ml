let () =
  let path = "1.cr" in
  let src = Io.load_file path in
  let ts = Lexer.lex path src in
  let stmts = Parser.parse ts in
  let stmts = Resolver.analyze stmts in
  let program = Lower.lower stmts in
  print_endline @@ X86.generate program

