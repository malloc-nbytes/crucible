let usage () =
  prerr_endline "usage: cruc <input.cr> [-o <output>] [-obj <file.o>]...";
  exit 1

let parse_args args =
  let rec aux input output objects = function
    | [] ->
       (match input with
        | Some input -> input, output, List.rev objects
        | None -> usage ())
    | "-o" :: output :: rest ->
       (match output with
        | "" -> usage ()
        | _ -> aux input (Some output) objects rest)
    | "-o" :: [] -> usage ()
    | ("-obj" | "--object") :: object_file :: rest ->
       aux input output (object_file :: objects) rest
    | ("-obj" | "--object") :: [] -> usage ()
    | argument :: rest ->
       (match input with
        | None -> aux (Some argument) output objects rest
        | Some _ -> usage ())
  in
  aux None None [] args

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

let compile input output objects =
  let src = Io.load_file input in
  let ts = Lexer.lex input src in
  let stmts = Parser.parse ts in
  let stmts = Resolver.analyze stmts in
  let program = Lower.lower stmts in
  let assembly = Filename.temp_file "cruc-" ".s" in
  try
    let channel = open_out assembly in
    output_string channel (X86.generate program);
    output_char channel '\n';
    close_out channel;
    run_linker assembly output objects;
    Sys.remove assembly
  with error ->
    (try Sys.remove assembly with Sys_error _ -> ());
    raise error

let () =
  let input, output, objects = parse_args (Array.to_list Sys.argv |> List.tl) in
  let output = match output with
    | Some output -> output
    | None -> Config.g_config.outfile
  in
  compile input output objects

