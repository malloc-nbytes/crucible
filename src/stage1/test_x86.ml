let expect_equal expected actual =
  if actual <> expected then
    failwith @@
      Printf.sprintf "expected:\n%s\n\ngot:\n%s" expected actual

let expect_int expected actual =
  if actual <> expected then
    failwith @@ Printf.sprintf "expected %d, got %d" expected actual

let contains needle haystack =
  let needle_len = String.length needle in
  let rec aux index =
    index + needle_len <= String.length haystack &&
    (String.sub haystack index needle_len = needle || aux (index + 1))
  in
  aux 0

let expect_contains needle haystack =
  if not @@ contains needle haystack then
    failwith @@ Printf.sprintf "expected output to contain %S" needle

let compile source =
  source
  |> Lexer.lex "codegen.cr"
  |> Parser.parse
  |> Resolver.analyze
  |> Lower.lower
  |> X86.generate

let () =
  let frame = X86.make_frame ~local_bytes:24 ~callee_saved:[X86.Rbx] in
  expect_int 40 (X86.frame_size frame)

let () =
  let output =
    compile
      "extern proc add(a: i32, b: i32): i32;\n\
       proc add64(a: i64, b: i64): i64 { return a + b; }\n\
       export proc main(): i32 {\n\
       \  let x: i32 = 20;\n\
       \  let y: i32 = 2;\n\
       \  let lt: i32 = 2 < 3;\n\
       \  let gt: i32 = 3 > 2;\n\
       \  let le: i32 = 2 <= 2;\n\
       \  let ge: i32 = 2 >= 2;\n\
       \  let eq: i32 = 2 == 2;\n\
       \  let ne: i32 = 2 != 3;\n\
       \  while y { y -= 1; }\n\
       \  y = 2;\n\
       \  x += y;\n\
       \  return add(lt + gt + le + ge + eq + ne, 18);\n\
       }\n"
  in
  List.iter
    (fun needle -> expect_contains needle output)
    [ ".globl main"
    ; "add64:"
    ; "mov qword ptr [rbp -"
    ; "add rax"
    ; "mov edi"
    ; "mov esi"
    ; "call add"
    ; "test eax, eax"
    ; "setl dl"
    ; "setg dl"
    ; "setle dl"
    ; "setge dl"
    ; "sete dl"
    ; "setne dl"
    ];
  let channel = open_out "test_x86.s" in
  output_string channel output;
  output_char channel '\n';
  close_out channel

let () =
  let output =
    compile
      "export proc main(): i32 {\n\
       \  let values: i32[3] = {1, 2, 3};\n\
       \  values[1] += 4;\n\
       \  return values[1];\n\
       }\n"
  in
  let channel = open_out "test_array.s" in
  output_string channel output;
  output_char channel '\n';
  close_out channel

let () =
  let output =
    compile
      "export proc main(): i32 {\n\
       \  let values: i32[3] = {4, 5, 6};\n\
       \  let pointer: i32* = &values[1];\n\
       \  *pointer += 7;\n\
       \  return *pointer + *values + pointer[1];\n\
       }\n"
  in
  let channel = open_out "test_pointer.s" in
  output_string channel output;
  output_char channel '\n';
  close_out channel
