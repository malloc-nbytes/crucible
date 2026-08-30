let () =
  let path = "1.in" in
  let src = Io.load_file path in
  let l = Lexer.lex path src in
  Lexer.print l
