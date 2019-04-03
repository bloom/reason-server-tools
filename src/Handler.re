open Pom;

type middleware('context, 'result) =
  (Express.Request.t, Express.Response.t, 'context) =>
  pomWithError('result, Express.complete);

type handler('context) =
  (Express.Request.t, Express.Response.t, 'context) => pom(Express.complete);

let pomTo500 = (~res, pom: pom('a)): pomWithError('a, Express.complete) => {
  pom
  <!> (
    _ => Express.Response.(res |> sendStatus(StatusCode.InternalServerError))
  );
};

let _onError500 =
    (a: handler('context)): middleware('context, Express.complete) =>
  (req, res, ctx) =>
    a(req, res, ctx)
    <!> (
      _ =>
        Express.Response.(res |> sendStatus(StatusCode.InternalServerError))
    );

exception UncaughtPromiseInHandler(Js.Promise.error);
let _handleMiddleware =
    (handler: middleware(unit, Express.complete)): Express.Middleware.t =>
  Express.PromiseMiddleware.from((_, req, res) =>
    handler(req, res, ())->mergeErr->toJsPromise
    |> Js.Promise.catch(err => {
         Log.error2(
           "Unexpected promise exception thrown. This shouldn't happen.",
           err,
         );
         Express.Response.(res |> sendStatus(StatusCode.InternalServerError))
         ->Js.Promise.resolve;
         /* next(err->UncaughtPromiseInHandler->Express.Next.error, res)
            ->Js.Promise.resolve; */
       })
  );

let andThen =
    (m: middleware('a, 'b), m2: middleware('b, 'c)): middleware('a, 'c) =>
  (req, res, a) => m(req, res, a)->flatMap(m2(req, res));

type _callableExpressMiddleware =
  (Express.Request.t, Express.Response.t, option(exn) => unit) => unit;
external _callExpressMiddleware:
  Express.Middleware.t => _callableExpressMiddleware =
  "%identity";
let middlewareFromExpress = (em: Express.Middleware.t): middleware('a, 'a) =>
  (req, res, a) => {
    let (p, resolve, reject) = Pom.make();
    let next =
      fun
      | Some(err) => {
          Log.error2("Failure from wrapped express middleware", err);
          reject(
            Express.Response.(
              res |> sendStatus(StatusCode.InternalServerError)
            ),
          );
        }
      | None => resolve(a);
    _callExpressMiddleware(em, req, res, next);
    p;
  };

let andThenHandle =
    (middleware: middleware(unit, 'a), handler: handler('a))
    : Express.Middleware.t =>
  middleware->andThen(handler->_onError500)->_handleMiddleware;

let handle = (handler: handler(unit)): Express.Middleware.t =>
  handler->_onError500->_handleMiddleware;