let expect_equal expected actual =
  if actual <> expected then
    failwith @@
      Printf.sprintf "expected:\n%s\n\ngot:\n%s" expected actual

let expect_int expected actual =
  if actual <> expected then
    failwith @@ Printf.sprintf "expected %d, got %d" expected actual

let () =
  let frame = X86.make_frame ~local_bytes:0 ~callee_saved:[] in
  let output =
    X86.emit_function
      ~public:true
      ~name:"main"
      ~frame
      [X86.Xor (X86.W32, X86.Rax, X86.Rax)]
  in
  expect_equal
    ".intel_syntax noprefix\n.text\n\n.globl main\nmain:\n  push rbp\n  mov rbp, rsp\n  xor eax, eax\n  pop rbp\n  ret\n\n.section .note.GNU-stack,\"\",@progbits"
    output;
  let channel = open_out "test_x86.s" in
  output_string channel output;
  output_char channel '\n';
  close_out channel

let () =
  let frame = X86.make_frame ~local_bytes:24 ~callee_saved:[X86.Rbx] in
  expect_int
    40
    (X86.frame_size frame)
