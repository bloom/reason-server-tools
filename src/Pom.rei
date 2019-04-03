/**
 * # Pom
 *
 * Poms are richer alternatives to promises that make error handling
 * required and explicit. They aren't much more than the combination
 * of JS promises and Reason Results. The value lies in the functions
 * provided for easy control flow.
 *
 * Hopefully more docs later!
 *
 * @all
 */;

type pomWithError('data, 'err);
type pom('data) = pomWithError('data, unit);
let make: unit => (pomWithError('a, 'b), 'a => unit, 'b => unit);
/*
 Working with node-style callbacks:

 The intention here is that, for JS APIs that don't offer promises, the developer can call the function, leaving off the last argument which, by convention, is the callback. That partially applied function
 can then be passed into `fromCallback` and a pom will be returned.

 The wrapped library's callback should express the error type as non-optional. The runtime of `fromCallback` will check to see if
 the error is falsy. If the error is falsy, then the promise
 will be fulfilled with the value of 'data. If the error is truthy,
 then the promise will be rejected with the value in 'err.

 so, for example, if I were wrapping Node's `fs`:

 The external definition:

 [@bs.module "fs"] external readFile: (string, {. "encoding": string}, (Js.Exn.t, Node.Buffer.t) => unit) => unit = "";

 And turning that into a pom:

 readFile("./foo", {"encoding": "utf-8"})
 ->Pom.fromCallback
 ->Pom.map(buffer => {
   -- The buffer will be available here.
 })

  */
type nodeCallback('err, 'data) = ('err, 'data) => unit;
let fromCallback:
  (nodeCallback('err, 'data) => unit) => pomWithError('data, 'err);
let resolved: 'a => pomWithError('a, 'b);
let rejected: 'a => pomWithError('b, 'a);
let map: (pomWithError('a, 'err), 'a => 'b) => pomWithError('b, 'err);
let (<$>): (pomWithError('a, 'b), 'a => 'c) => pomWithError('c, 'b);
let flatMap:
  (pomWithError('a, 'err), 'a => pomWithError('b, 'err)) =>
  pomWithError('b, 'err);
let (>>=):
  (pomWithError('a, 'b), 'a => pomWithError('c, 'b)) => pomWithError('c, 'b);
let wait:
  (pomWithError('data, 'err), Belt.Result.t('data, 'err) => unit) =>
  pom(unit);
let try_:
  (pomWithError('a, 'err), 'a => Belt.Result.t('b, 'err)) =>
  pomWithError('b, 'err);
let (<?>):
  (pomWithError('a, 'err), 'a => Belt.Result.t('b, 'err)) =>
  pomWithError('b, 'err);
let tap:
  (pomWithError('data, 'err), 'data => unit) => pomWithError('data, 'err);
let (>->):
  (pomWithError('data, 'err), 'data => unit) => pomWithError('data, 'err);
let mapErr:
  (pomWithError('data, 'err), 'err => 'err2) => pomWithError('data, 'err2);
let (<!>): (pomWithError('a, 'b), 'b => 'c) => pomWithError('a, 'c);
let recover: (pomWithError('data, 'err), 'err => 'data) => pom('data);
let mergeErr: pomWithError('a, 'a) => pom('a);
let flatMapFromError:
  (pomWithError('data, 'err), 'err => pomWithError('data, 'err)) =>
  pomWithError('data, 'err);
let (!>=):
  (pomWithError('data, 'err), 'err => pomWithError('data, 'err)) =>
  pomWithError('data, 'err);

let all: list(pom('a)) => pom(list('a));
let allArray: array(pom('a)) => pom(array('a));
let fromJsPromise: Js.Promise.t('a) => pomWithError('a, Js.Promise.error);
let toJsPromise: pom('a) => Js.Promise.t('a);
let let_:
  (pomWithError('a, 'b), 'a => pomWithError('c, 'b)) => pomWithError('c, 'b);