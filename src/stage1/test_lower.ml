let lower src =
  src
  |> Lexer.lex "assignment.cr"
  |> Parser.parse
  |> Resolver.analyze
  |> Lower.lower
  |> Tac.program_to_string

let expect_equal expected actual =
  if actual <> expected then
    failwith @@
      Printf.sprintf "expected:\n%s\n\ngot:\n%s" expected actual

let contains needle haystack =
  let needle_len = String.length needle in
  let rec aux i =
    i + needle_len <= String.length haystack &&
    (String.sub haystack i needle_len = needle || aux (i + 1))
  in
  aux 0

let expect_contains needle haystack =
  if not @@ contains needle haystack then
    failwith @@ Printf.sprintf "expected output to contain:\n%s\n\ngot:\n%s" needle haystack

let () =
  expect_equal
    "export proc main(): i32 {\nL0:\n  store i32 9, %local1\n  %0 = load i32 %local1\n  %1 = add i32 %0, 2\n  store i32 %1, %local1\n  %2 = load i32 %local1\n  ret %2\n}"
    (lower
       "export proc main(): i32 {\n\
       \  let x: i32 = 9;\n\
       \  x += 2;\n\
       \  return x;\n\
       }\n")

let () =
  let output =
    lower
      "export proc main(): i32 {\n\
      \  let x: i32 = 9;\n\
      \  x = 8;\n\
      \  x += 1;\n\
      \  x -= 1;\n\
      \  x *= 2;\n\
      \  x /= 2;\n\
      \  x %= 2;\n\
      \  x |= 1;\n\
      \  x &= 1;\n\
      \  x ^= 1;\n\
      \  return x;\n\
      }\n"
  in
  List.iter
    (fun needle -> expect_contains needle output)
    [ "store i32 8, %local1"
    ; "= add i32"
    ; "= sub i32"
    ; "= mul i32"
    ; "= div i32"
    ; "= mod i32"
    ; "= or i32"
    ; "= and i32"
    ; "= xor i32"
    ]

let () =
  expect_equal
    "export proc main(): i32 {\nL0:\n  store i32 2, %local1\n  jmp L1\nL1:\n  %0 = load i32 %local1\n  branch %0, L2, L3\nL2:\n  %1 = load i32 %local1\n  %2 = sub i32 %1, 1\n  store i32 %2, %local1\n  jmp L1\nL3:\n  ret 0\n}"
    (lower
       "export proc main(): i32 {\n\
       \  let n: i32 = 2;\n\
       \  while n {\n\
       \    n -= 1;\n\
       \  }\n\
       \  return 0;\n\
       }\n")

let () =
  let output =
    lower
      "export proc main(): i32 {\n\
       \  let lt: i32 = 2 < 3;\n\
       \  let gt: i32 = 3 > 2;\n\
       \  let le: i32 = 2 <= 2;\n\
       \  let ge: i32 = 2 >= 2;\n\
       \  let eq: i32 = 2 == 2;\n\
       \  let ne: i32 = 2 != 3;\n\
       \  return lt + gt + le + ge + eq + ne;\n\
       }\n"
  in
  List.iter
    (fun needle -> expect_contains needle output)
    [ "= lt i32"
    ; "= gt i32"
    ; "= le i32"
    ; "= ge i32"
    ; "= eq i32"
    ; "= ne i32"
    ]

let () =
  expect_equal
    "export proc main(): i32 {\nL0:\n  array i32 {1, 2, 3}, %local1\n  %0 = index_load i32 %local1, 1\n  ret %0\n}"
    (lower
       "export proc main(): i32 {\n\
       \  let values: i32[3] = {1, 2, 3};\n\
       \  return values[1];\n\
       }\n")

let () =
  expect_equal
    "export proc main(): i32 {\nL0:\n  struct Pair {left = 4, right = 5}, %local1\n  ret 0\n}"
    (lower "struct Pair { left: i32, right: i32, } export proc main(): i32 { let pair: Pair = Pair { .right = 5, .left = 4, }; return 0; }")

let () =
  let output =
    lower
      "struct Point { x: i32, y: i32, } export proc main(): i32 { let point: Point = Point { .x = 3, .y = 4, }; let ptr: Point* = &point; point.x += 2; ptr.y += point.x; return point.x + ptr.y; }"
  in
  List.iter (fun needle -> expect_contains needle output)
    [ "field_address i32 %local1, x"
    ; "field_address i32 %4, y"
    ; "deref_store i32"
    ]
