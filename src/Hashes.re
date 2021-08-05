module Hash = {
  type hash;
  [@bs.module "crypto"] external createHash: string => hash = "";
  [@bs.send] external update: (hash, Node.Buffer.t) => unit = "";
  /* updateWithString: The string is assumed to be a UTF-8 string */
  [@bs.send] external updateWithString: (hash, string) => unit = "update";
  [@bs.send] external digest: (hash, string) => string = "";
};

module LegacyMd5 = {
  let md5OfString: string => string = [%bs.raw
    {|
    function (passedInString) {
      const long = require('long');
      const crypto = require('crypto');

      const bigNumber = long.fromString(passedInString);
      const byteArray = Int32Array.from([bigNumber.low, bigNumber.high]);
      return crypto.createHash('md5').update(byteArray).digest('hex').slice(0, 4);
    }
    |}
  ];
};

let sha1 = (b: Node.Buffer.t): string => {
  open Hash;
  let hash = createHash("sha1");
  hash->update(b) |> ignore;
  hash->digest("hex");
};

let sha256 = (s: string): string => {
  open Hash;
  let hash = createHash("sha256");
  hash->updateWithString(s) |> ignore;
  hash->digest("hex");
};

let md5 = (b: Node.Buffer.t): string => {
  open Hash;
  let hash = createHash("md5");
  hash->update(b) |> ignore;
  hash->digest("hex");
};

let md5OfString = (b: string): string => {
  open Hash;
  let hash = createHash("md5");
  hash->updateWithString(b) |> ignore;
  hash->digest("hex");
};

[@bs.module "crypto"] external randomBytes: int => Node.buffer = "";

[@bs.send.pipe: Node.buffer] external toString: string => string = "";

let randomId = () => randomBytes(16) |> toString("hex") |> String.uppercase_ascii;
