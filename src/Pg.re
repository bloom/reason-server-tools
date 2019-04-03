type pool = {
  .
  "totalCount": int,
  "idleCount": int,
  "waitingCount": int,
};
type client;
[@bs.module "pg"] [@bs.new]
external makePool: {. "connectionString": string} => pool = "Pool";

type dbResult = {. "rows": Js.Json.t};

module Types = {
  type t;

  [@bs.val] external parseInt: string => int = "";
  [@bs.module "pg"] external types: t = "";
  [@bs.send] external setTypeParser: (t, int, string => 'a) => unit = "";
  [@bs.send]
  external setTypeParserD: (t, int, Js.Date.t => 'a) => unit = "setTypeParser";

  let parseDate = (a: Js.Date.t) => {
    Js.Date.toString(a);
  };

  types->setTypeParser(20, parseInt);
  types->setTypeParserD(1184, parseDate);
};

type values;
type queryPayload = {
  .
  "text": string,
  "values": values,
};

[@bs.send]
external _query: (pool, queryPayload) => Js.Promise.t(dbResult) = "query";

let query = (pool, payload) => _query(pool, payload)->Pom.fromJsPromise;

[@bs.send]
external onConnect: (pool, [@bs.as "connect"] _, client => unit) => unit =
  "on";

[@bs.send]
external onAcquire: (pool, [@bs.as "acquire"] _, client => unit) => unit =
  "on";

[@bs.send]
external onError:
  (pool, [@bs.as "error"] _, (Js.Exn.t, client) => unit) => unit =
  "on";

[@bs.send]
external onRemove: (pool, [@bs.as "remove"] _, client => unit) => unit = "on";

let poolStats = (pool: pool) =>
  "totalCount="
  ++ string_of_int(pool##totalCount)
  ++ ",idleCount="
  ++ string_of_int(pool##idleCount)
  ++ ",waitingCount="
  ++ string_of_int(pool##waitingCount)
  ++ ",";