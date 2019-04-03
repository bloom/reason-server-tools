open Belt;
let first = (decoder: Json.Decode.decoder('a), json: Js.Json.t): 'a => {
  open Json.Decode;
  let decoded = json |> list(decoder);
  switch (decoded->List.get(0)) {
  | Some(item) => item
  | None =>
    raise(
      Json.Decode.DecodeError(
        "Expected at least one element in list but got" ++ Obj.magic(json),
      ),
    )
  };
};

// Meant specifically for query strings. Allows the user to pass in
// booleans like this: ?foo | ?foo=1 | ?foo=true | ?foo=false | ?foo=0 etc...
let boolFromQuery = json => {
  switch (Json.Decode.string(json)) {
  | "" => true
  | "1" => true
  | "true" => true
  | "yes" => true
  | "0" => false
  | "false" => false
  | _ => false
  };
};

// Meant specifically for query strings. Allows the user to pass in
// strings like so: ?excludeJournals=c1g132g,c73aw93,d902ufa
let stringArrayFromQuery = json => {
  Json.Decode.string(json) |> Js.String.split(",");
};

let base64 = (json: Js.Json.t): Node.Buffer.t => {
  json |> Json.Decode.string |> Node.Buffer.fromStringWithEncoding(_, `base64);
};

let safe = (~decoder, ~onFail: exn => unit, json) =>
  switch (decoder(json)) {
  | exception e =>
    onFail(e);
    Result.Error();
  | value => Result.Ok(value)
  };

let logFailure = (~message, ~safeDecoder, json) =>
  switch (safeDecoder(json)) {
  | Result.Ok(v) => Result.Ok(v)
  | Result.Error(errMsg) =>
    Log.error2(message, errMsg);
    Result.Error();
  };