open Belt;
type pomWithError('data, 'err) = Js.Promise.t(Result.t('data, 'err));
type pom('data) = pomWithError('data, unit);

module JsPromise = {
  let make = () => {
    let resolver = ref(ignore);
    let p =
      Js.Promise.make((~resolve, ~reject as _) =>
        resolver := (a => resolve(. a))
      );

    (p, resolver^);
  };

  let map = (p, mapper) =>
    p |> Js.Promise.then_(v => mapper(v)->Js.Promise.resolve);
  let flatMap = (p, mapper) => p |> Js.Promise.then_(v => mapper(v));
  let catch = (p, mapper) => p |> Js.Promise.catch(mapper);
  let resolve = Js.Promise.resolve;
  let reject = Js.Promise.reject;
  let wait = (p: Js.Promise.t('a), waiter: 'a => unit): Js.Promise.t(unit) =>
    p->map(v => waiter(v));
  let all = Js.Promise.all;
};

/* Constructors */
let make = () => {
  let (p, resolve) = JsPromise.make();
  let rreject = e => resolve(Result.Error(e));
  let rresolve = a => resolve(Result.Ok(a));
  (p, rresolve, rreject);
};

let makeWithCallback = () => {
  let (p, resolve) = JsPromise.make();
  let isFalsy: 'x => bool = [%bs.raw
    {js| function(a){return a ? true : false}|js}
  ];
  let callback = (err: 'e, result: 'a) =>
    /* The convention with Node or JS callbacks is to
       check an error like this:
       if (err) {
         ...
       } else {
         ...
       }

       Thus, some JS libraries may return false, null, 0, or something other than undefined
       when there was no error. We'll be loose with our test to account for this.
        */
    isFalsy(err)
      ? resolve(Result.Error(err)) : resolve(Result.Ok(result));
  (p, callback);
};

type nodeCallback('err, 'data) = ('err, 'data) => unit;
let fromCallback =
    (fn: nodeCallback('err, 'data) => unit): pomWithError('data, 'err) => {
  let (p, cb) = makeWithCallback();
  fn(cb);
  p;
};
let resolved = a => a->Result.Ok->JsPromise.resolve;
let rejected = a => a->Result.Error->JsPromise.resolve;

/* Operators */
let map =
    (p: pomWithError('a, 'err), mapper: 'a => 'b): pomWithError('b, 'err) =>
  p->JsPromise.map(
    fun
    | Result.Ok(a) => Result.Ok(mapper(a))
    | Result.Error(e) => Result.Error(e),
  );

let (<$>) = map;

let flatMap =
    (p: pomWithError('a, 'err), mapper: 'a => pomWithError('b, 'err))
    : pomWithError('b, 'err) =>
  p->JsPromise.flatMap(
    fun
    | Result.Ok(a) => mapper(a)
    | Result.Error(err) => err->rejected,
  );

let (>>=) = flatMap;

let wait =
    (p: pomWithError('data, 'err), waiter: Result.t('data, 'err) => unit)
    : pom(unit) =>
  JsPromise.wait(p, waiter)->JsPromise.map(_ => Result.Ok());

let try_ =
    (p: pomWithError('a, 'err), mapper: 'a => Result.t('b, 'err))
    : pomWithError('b, 'err) =>
  p->flatMap(v =>
    switch (mapper(v)) {
    | Result.Ok(vv) => resolved(vv)
    | Result.Error(e) => rejected(e)
    }
  );

let (<?>) = try_;

let tap =
    (p: pomWithError('data, 'err), tapper: 'data => unit)
    : pomWithError('data, 'err) =>
  p->map(d => {
    tapper(d);
    d;
  });

let (>->) = tap;

let mapErr =
    (p: pomWithError('data, 'err), mapper: 'err => 'err2)
    : pomWithError('data, 'err2) =>
  p->JsPromise.map(
    fun
    | Result.Ok(data) => Result.Ok(data)
    | Result.Error(err) => Result.Error(err->mapper),
  );

let (<!>) = mapErr;

let recover =
    (p: pomWithError('data, 'err), recoverer: 'err => 'data): pom('data) =>
  p->JsPromise.map(
    fun
    | Result.Ok(a) => Result.Ok(a)
    | Result.Error(e) => Result.Ok(recoverer(e)),
  );

let mergeErr = (p: pomWithError('a, 'a)): pom('a) => recover(p, a => a);

let flatMapFromError =
    (
      p: pomWithError('data, 'err),
      mapper: 'err => pomWithError('data, 'err),
    )
    : pomWithError('data, 'err) =>
  p->JsPromise.flatMap(
    fun
    | Result.Ok(a) => Result.Ok(a)->JsPromise.resolve
    | Result.Error(e) => mapper(e),
  );

let (!>=) = flatMapFromError;

let all = (ps: list(pom('a))): pom(list('a)) =>
  ps
  ->List.toArray
  ->JsPromise.all
  ->JsPromise.map(List.fromArray)
  ->JsPromise.map(resultList =>
      if (List.some(resultList, Result.isError)) {
        Result.Error();
      } else {
        List.map(resultList, Result.getExn)->Result.Ok;
      }
    );

let allArray = (ps: array(pom('a))): pom(array('a)) =>
  ps
  ->JsPromise.all
  ->JsPromise.map(resultArray =>
      if (Array.some(resultArray, Result.isError)) {
        Result.Error();
      } else {
        Array.map(resultArray, Result.getExn)->Result.Ok;
      }
    );

/* Compat */
let fromJsPromise: Js.Promise.t('a) => pomWithError('a, Js.Promise.error) =
  jsp =>
    jsp
    ->JsPromise.map(a => Result.Ok(a))
    ->JsPromise.catch(err => Result.Error(err)->JsPromise.resolve);

exception PomFailed;
exception PromiseRejected(Js.Promise.error);
let toJsPromise: pom('a) => Js.Promise.t('a) =
  pom =>
    pom
    ->JsPromise.flatMap(
        fun
        | Result.Ok(v) => JsPromise.resolve(v)
        | Result.Error () => JsPromise.reject(PomFailed),
      )
    ->JsPromise.catch(err => JsPromise.reject(PromiseRejected(err)));

/* ppx-let support */
let let_ = (>>=);
