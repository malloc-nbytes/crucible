let assert_equal expected actual =
  if expected <> actual then
    failwith @@
      Printf.sprintf "expected:\n%s\n\nactual:\n%s" expected actual

let lower source =
  source
  |> Lexer.lex "test.cruc"
  |> Parser.parse
  |> Resolver.analyze
  |> Lower.lower
  |> Tac.program_to_string

let test_procedure_linkage () =
  assert_equal
    "proc internal(): void {\nL0:\n  ret\n}\n\n\
     export proc exported(): void {\nL0:\n  ret\n}\n\n\
     extern proc imported(): i32;\n\n\
     extern export proc published_import(): i32;"
    (lower
       "proc internal(): void {}\n\
        export proc exported(): void {}\n\
        extern proc imported(): i32;\n\
        extern export proc published_import(): i32;")

let () = test_procedure_linkage ()
