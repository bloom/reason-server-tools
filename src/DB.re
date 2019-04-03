open Pom;

// These operations are abstract in case you want to use them
// with multiple pools (multiple databases) in one project.
module Abstract = {
  let perform =
      (pool: Pg.pool, bricks: Bricks.builder)
      : Pom.pomWithError(Js.Json.t, Js.Promise.error) =>
    pool->Pg.query(bricks->Bricks.toPayload)->map(a => a##rows);

  let performRaw =
      (pool: Pg.pool, ~values=?, text)
      : Pom.pomWithError(Js.Json.t, Js.Promise.error) => {
    let fragment =
      switch (values) {
      | Some(v) => Bricks.sqlWithValues(~text, ~values=v)
      | None => Bricks.sql(text)
      };
    pool
    ->Pg.query({"text": fragment##str, "values": fragment##vals})
    ->map(a => a##rows);
  };
};

let pool = ref(None);
let connect = postgresUrl => {
  pool := Some(Pg.makePool({"connectionString": postgresUrl}));
};

exception DatabaseNotConnected;
let getPool = () => {
  switch (pool^) {
  | None =>
    Log.error(
      "Looks like the database isn't connected yet. Did you forget to call DB.connect?",
    );
    raise(DatabaseNotConnected);
  | Some(p) => p
  };
};

let perform = getPool()->Abstract.perform;

let performRaw =
    (~values=?, text): Pom.pomWithError(Js.Json.t, Js.Promise.error) => {
  Abstract.performRaw(getPool(), ~values?, text);
};