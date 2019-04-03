let queryJson = (req: Express.Request.t): Js.Json.t =>
  Obj.magic(Express.Request.query(req));

