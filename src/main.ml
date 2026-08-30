let () =
  let path = "1.in" in
  let src = Io.load_file path in
  let ts = Lexer.lex path src in
  let _ = Parser.parse ts in
  ()
