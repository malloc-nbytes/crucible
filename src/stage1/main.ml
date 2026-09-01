let usage () =
  prerr_endline "usage: cruc <input.cr> [-o <output>]";
  exit 1

let parse_args args =
  let rec aux input output = function
    | [] ->
       (match input with
        | Some input -> input, output
        | None -> usage ())
    | "-o" :: output :: rest ->
       (match output with
        | "" -> usage ()
        | _ -> aux input (Some output) rest)
    | "-o" :: [] -> usage ()
    | argument :: rest ->
       (match input with
        | None -> aux (Some argument) output rest
        | Some _ -> usage ())
  in
  aux None None args

let run_linker assembly output =
  let command =
    "cc -o " ^ Filename.quote output ^ " " ^ Filename.quote assembly
  in
  match Sys.command command with
  | 0 -> ()
  | status ->
     failwith @@ Printf.sprintf "system linker failed with exit status %d" status

let compile input output =
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
    run_linker assembly output;
    Sys.remove assembly
  with error ->
    (try Sys.remove assembly with Sys_error _ -> ());
    raise error

let () =
  let input, output = parse_args (Array.to_list Sys.argv |> List.tl) in
  let output = match output with
    | Some output -> output
    | None -> Config.g_config.outfile
  in
  compile input output

