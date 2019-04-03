let isBefore = (a: Js.Date.t, b: Js.Date.t): bool => {
  a->Js.Date.getTime < b->Js.Date.getTime;
};