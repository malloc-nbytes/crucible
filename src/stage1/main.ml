let usage () =
  prerr_endline "usage: cruc <input.cr> [-o <output>] [-obj <file.o>]... [-asm] [-tac]";
  exit 1

let parse_args args =
  let rec aux input output objects keep_assembly emit_tac = function
    | [] ->
       (match input with
        | Some input -> input, output, List.rev objects, keep_assembly, emit_tac
        | None -> usage ())
    | "-o" :: output :: rest ->
       (match output with
        | "" -> usage ()
        | _ -> aux input (Some output) objects keep_assembly emit_tac rest)
    | "-o" :: [] -> usage ()
    | ("-obj" | "--object") :: object_file :: rest ->
       aux input output (object_file :: objects) keep_assembly emit_tac rest
    | ("-obj" | "--object") :: [] -> usage ()
    | "-asm" :: rest -> aux input output objects true emit_tac rest
    | "-tac" :: rest -> aux input output objects keep_assembly true rest
    | argument :: rest ->
       (match input with
        | None -> aux (Some argument) output objects keep_assembly emit_tac rest
        | Some _ -> usage ())
  in
  aux None None [] false false args

let run_linker assembly output objects =
  let command =
    String.concat " "
      (["cc"; "-o"; Filename.quote output; Filename.quote assembly]
       @ List.map Filename.quote objects)
  in
  match Sys.command command with
  | 0 -> ()
  | status ->
     failwith @@ Printf.sprintf "system linker failed with exit status %d" status

let write_output path contents =
  let channel = open_out path in
  try
    output_string channel contents;
    output_char channel '\n';
    close_out channel
  with error ->
    close_out_noerr channel;
    raise error

let compile input output objects keep_assembly emit_tac =
  let src = Io.load_file input in
  let ts = Lexer.lex input src in
  let stmts = Parser.parse ts in
  let stmts = Resolver.analyze stmts in
  let program = Lower.lower stmts in
  if emit_tac then write_output (output ^ ".tac") (Tac.program_to_string program);
  let assembly =
    if keep_assembly then output ^ ".s" else Filename.temp_file "cruc-" ".s"
  in
  try
    write_output assembly (X86.generate program);
    run_linker assembly output objects;
    if not keep_assembly then Sys.remove assembly
  with error ->
    if not keep_assembly then (try Sys.remove assembly with Sys_error _ -> ());
    raise error

let () =
  let input, output, objects, keep_assembly, emit_tac =
    parse_args (Array.to_list Sys.argv |> List.tl)
  in
  let output = match output with
    | Some output -> output
    | None -> Config.g_config.outfile
  in
  compile input output objects keep_assembly emit_tac

