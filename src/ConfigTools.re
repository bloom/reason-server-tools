/**
 * # ConfigTools
 *
 * We've found that a useful convention is to have a module named
 * "Config.re", where all values settable by ENV vars are collected
 * into one place.
 *
 * We've provided `fromEnv` as a tool to easily fetch a string from
 * the environment, and use a fallback value if the string isn't set.
 *
 * We've also provided a few built-in values that are generally needed
 * in server development.
 *
 * @all
 */;

let fromEnv = (key: string, default: string) => {
  let env = Node.Process.process##env;
  switch (env->(Js.Dict.get(key))) {
  | Some(v) => v
  | None => default
  };
};

let envName = fromEnv("NODE_ENV", "development");
let isProduction = envName == "production";
let isDev = envName == "dev" || envName == "development";
let logLevel = fromEnv("LOG_LEVEL", isProduction ? "info" : "silly");
let port = fromEnv("PORT", "3000") |> int_of_string;