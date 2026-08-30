type 'v entry =
  { id : string
  ; value : 'v
  }

type 'v frame = 'v entry list

type 'v t = 'v frame list

let empty = [[]]

let rec get id = function
  | [] -> None
  | frame :: ps ->
     match List.find_opt (fun e -> e.id = id) frame with
     | Some e -> Some e.value
     | None -> get id ps

let contains id t = Option.is_some @@ get id t

let push t = [] :: t

let pop = function
  | _ :: (_ :: _ as ps) -> ps
  | t -> t

let add id value = function
  | [] -> [[{id; value}]]
  | frame :: ps ->
     ({id; value} :: List.filter (fun e -> e.id <> id) frame) :: ps

