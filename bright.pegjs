{
  var conf = {};
  var funcs = {};
  var code;
  var locals = [];
  var native = [];

  function varSlot(name) {
    if (name in conf || name in funcs) {
      throw new Error("name is taken: " + name);
    }
    var index = locals.indexOf(name);
    if (index >= 0) return index;
    index = locals.length;
    locals[index] = name;
    return index;
  }
  function fnSlot(name) {
    if (name in conf || name in funcs) {
      throw new Error("name is taken: " + name);
    }
    var index = native.indexOf(name);
    if (index >= 0) return index;
    index = native.length;
    native[index] = name;
    return index;
  }

  var OPS = [
    "SET0",  "SET1",  "SET2",  "SET3",  "SET4",  "SET5",  "SET6",  "SET7",
    "SET8",  "SET9",  "SET10", "SET11", "SET12", "SET13", "SET14", "SET15",
    "GET0",  "GET1",  "GET2",  "GET3",  "GET4",  "GET5",  "GET6",  "GET7",
    "GET8",  "GET9",  "GET10", "GET11", "GET12", "GET13", "GET14", "GET15",
    "CALL0",  "CALL1",  "CALL2",  "CALL3",  "CALL4",  "CALL5",  "CALL6",  "CALL7",
    "SETN", "GETN", "CALLN",
    "INCR", "DECR", "INCRMOD", "DECRMOD",
    "ADD", "SUB", "MUL", "DIV", "MOD", "NEG",
    "LT", "LTE", "GT", "GTE", "EQ", "NEQ",
    "AND", "OR", "XOR", "NOT",
    "IF", "THEN", "ELSE",
    "DO", "DOI", "LOOP",
    "GOSUB", "RETURN",
  ];

  var opindex = {};
  for (var i = 0, l = OPS.length; i < l; i++) {
    opindex[OPS[i]] = i + 128;
  }

  Conf.prototype.__proto__ =
  Func.prototype.__proto__ =
  Assign.prototype.__proto__ =
  Mut.prototype.__proto__ =
  Binop.prototype.__proto__ =
  Unop.prototype.__proto__ =
  Call.prototype.__proto__ =
  Variable.prototype.__proto__ =
  Integer.prototype.__proto__ =
  If.prototype.__proto__ =
  Loop.prototype.__proto__ =
  {
    compile: function () {
      throw new Error("TODO: compile " + this.constructor.name);
    }
  };
  function Program(code) {
    this.code = code;
  }
  Program.prototype.compile = function () {

    this.code.forEach(decl => {
      if (decl instanceof Conf) {
        conf[decl.name] = decl.value;
      }
      else if (decl instanceof Func) {
        funcs[decl.name] = true;
      }
      else {
        throw new Error("Unexpected declaration");
      }
    });

    this.code.forEach(decl => {
      if (decl instanceof Func) {
        code = [];
        decl.body.compile();
        funcs[decl.name] = code;
      }
    });
    return { conf, locals, native, funcs };
  };
  function Conf(name, value) {
    this.name = name;
    this.value = value;
  }
  function Block(statements) {
    this.statements = statements;
  }
  Block.prototype.compile = function () {
    this.statements.forEach(statement => {
      statement.compile();
    });
  }
  function Func(name, body) {
    this.name = name;
    this.body = body;
  }
  function Assign(name, value) {
    this.name = name;
    this.value = value;
  }
  Assign.prototype.compile = function () {
    this.value.compile();
    var index = varSlot(this.name);
    if (index < 15) {
      code.push("SET" + index);
    }
    else {
      code.push(index, "SETN");
    }
  };
  function Mut(name, op, incr, mod) {
    this.name = name;
    this.op = op;
    this.incr = incr;
    this.mod = mod;
  }
  Mut.prototype.compile = function () {
    var slot = varSlot(this.name);
    var prefix = this.op === "ADD" ? "INCR" : "DECR";
    this.incr.compile();
    if (this.mod) {
      this.mod.compile();
      code.push(slot, prefix + "MOD");
    }
    else {
      code.push(slot, prefix);
    }
  }
  function Binop(op, left, right) {
    this.op = op;
    this.left = left;
    this.right = right;
  }
  Binop.prototype.compile = function () {
    this.left.compile();
    this.right.compile();

    if (typeof code[code.length - 1] !== "number" ||
        typeof code[code.length - 2] !== "number") {
      code.push(this.op);
      return;
    }
    var right = code.pop();
    var left = code.pop();
    code.push(compute(this.op, left, right));
  };
  function compute(op, left, right) {
    switch (op) {
      case "ADD": return left + right;
      case "SUB": return left - right;
      case "MUL": return left * right;
      case "DIV": return (left / right) >>> 0;
      case "MOD": return left % right;
      case "NEG": return - right;
      case "LT": return left < right ? 1 : 0;
      case "LTE": return left <= right ? 1 : 0;
      case "GT": return left > right ? 1 : 0;
      case "GTE": return left >= right ? 1 : 0;
      case "EQ": return left === right ? 1 : 0;
      case "NEQ": return left !== right ? 1 : 0;
      case "AND": return left && right;
      case "OR": return left || right;
      case "XOR": return left ? right ? 0 : left : right ? right : 0;
      case "NOT": return right ? 0 : 1;
    }
  }
  function Unop(op, right) {
    this.op = op;
    this.right = right;
  }
  Unop.prototype.compile = function () {
    this.right.compile();
    if (typeof code[code.length - 1] !== "number") {
      code.push(this.op);
      return;
    }
    var right = code.pop();
    code.push(compute(this.op, null, right));
  }
  function Call(name, args) {
    this.name = name;
    this.args = args;
  }
  Call.prototype.compile = function () {
    if (this.name in funcs) {
      if (this.args.length) {
        throw new Error("user functions don't take args")
      }
      code.push(Object.keys(funcs).indexOf(this.name))
      code.push("GOSUB");
      return;
    }
    this.args.forEach(arg => {
      arg.compile();
    });
    code.push(fnSlot(this.name));
    var len = this.args.length;
    if (len < 8) {
      code.push("CALL" + len);
    }
    else {
      code.push(len, "CALLN");
    }
  };
  function Variable(name) {
    this.name = name;
  }
  Variable.prototype.compile = function () {
    if (this.name in conf) {
      conf[this.name].compile();
      return;
    }
    var slot = varSlot(this.name);
    if (slot < 16) {
      code.push("GET" + slot);
    }
    else {
      code.push(slot, "GETN");
    }
  };
  function Integer(value) {
    this.value = value;
  }
  Integer.prototype.compile = function () {
    code.push(this.value);
  };
  function If(pairs, last) {
    this.pairs = pairs;
    this.last = last;
  }
  If.prototype.compile = function () {
    var pairs = this.pairs;
    var last = this.last;
    var i = 0, l = pairs.length;
    function dump() {
      var pair = pairs[i];
      pair[0].compile();
      code.push("IF");
      pair[1].compile();
      if (++i < l) {
        code.push("ELSE");
        dump();
      }
      else if (last) {
        code.push("ELSE");
        last.compile();
      }
      code.push("THEN");
    }
    dump();
  }
  function Loop(name, count, body) {
    this.name = name;
    this.count = count;
    this.body = body;
  }
  Loop.prototype.compile = function () {
    this.count.compile();
    if (this.name) {
      code.push(varSlot(this.name), "DOI");
    }
    else {
      code.push("DO");
    }
    this.body.compile();
    code.push("LOOP");
  }

  function list(head, tail, index) {
    return [head].concat(tail.map(entry => entry[index]));
  }
}

start
  = _ program:Program? _ { return new Program(program || []) }

Program
  = head:Declaration tail:(__ Declaration)* { return list(head, tail, 1) }

Declaration
  = Configuration
  / Function

Configuration
  = i:ident _ "=" _ e:Integer { return new Conf(i, e); }

Function
  = i:ident _ b:block { return new Func(i, b); }

block
  = "{" _ c:code? _ "}" { return new Block(c || []) }

code
  = head:statement tail:(_ statement)* {
      return list(head, tail, 1)
    }

ident
  = letters:([a-z][a-z0-9]*) {
      return letters[0] + letters[1].join("");
    }


IdentifierPart = [a-zA-Z]

Keyword
  = IfToken
  / ElfToken
  / ElseToken
  / LoopToken

IfToken    = "if"      !IdentifierPart
ElfToken   = "elf"     !IdentifierPart
ElseToken  = "else"    !IdentifierPart
LoopToken  = "loop"    !IdentifierPart

statement
  = IfToken _ e:expression _ b:block
    el:( _ ElfToken _ expression _ block)*
    last:( _ ElseToken _ block)? {
      var elves = [[e, b]];
      if (el) {
        el.forEach(function (elf) {
          elves.push([elf[3], elf[5]]);
        });
      }
      return new If(elves, last && last[3]);
    }
  / LoopToken _ name:(ident ":")? count:Integer _ b:block {
      return new Loop(name && name[0], count, b);
    }
  / i:ident _ "=" _ v:expression {
      return new Assign(i, v);
    }
  / i:ident t:(":" length)? _ "+=" _ v:expression {
      return new Mut(i, "ADD", v, t && t[1]);
    }
  / i:ident t:(":" length)? _ "-=" _ v:expression {
      return new Mut(i, "SUB", v, t && t[1]);
    }
  / Call

expression
  = e2

e2
  = left:e3 _ "or" _ right:e2 {
      return new Binop("OR", left, right);
    }
  / left:e3 _ "xor" _ right:e2 {
      return new Binop("XOR", left, right);
    }
  / e3

e3
  = left:e4 _ "and" _ right:e3 {
      return new Binop("AND", left, right);
    }
  / e4

e4
  = left:e5 _ "==" _ right:e4 {
      return new Binop("EQ", left, right);
    }
  / left:e5 _ "!=" _ right:e4 {
      return new Binop("NEQ", left, right);
    }
  / e5

e5
  = left:e6 _ "<" _ right:e5 {
      return new Binop("LT", left, right);
    }
  / left:e6 _ "<=" _ right:e5 {
      return new Binop("LTE", left, right);
    }
  / left:e6 _ ">" _ right:e5 {
      return new Binop("GT", left, right);
    }
  / left:e6 _ ">=" _ right:e5 {
      return new Binop("GTE", left, right);
    }
  / e6

e6
  = left:e7 _ "+" _ right:e6 {
      return new Binop("ADD", left, right);
    }
  / left:e7 _ "-" _ right:e6 {
      return new Binop("SUB", left, right);
    }
  / e7

e7
  = left:e8 _ "*" _ right:e7 {
      return new Binop("MUL", left, right);
    }
  / left:e8 _ "/" _ right:e7 {
      return new Binop("DIV", left, right);
    }
  / left:e8 _ "%" _ right:e7 {
      return new Binop("MOD", left, right);
    }
  / e8

e8
  = "-" _ right:e9 { return new Unop("NEG", right) }
  / "not" _ right:e9 { return new Unop("NOT", right) }
  / e9

e9
  = "(" _ e:expression _ ")" { return e }
  / Integer
  / Call
  / Variable

Integer
  = digits:([-+]?[0-9]+) {
      return new Integer(parseInt((digits[0]||"") + digits[1].join(""), 10))
    }

length
  = Variable
  / Integer

Variable
  = i:ident { return new Variable(i) }

Call
  = i:ident _ "(" _ a:args? _ ")" {
      return new Call(i, a || []);
    }

args
  = head:expression tail:(_ "," _ expression)* {
      return list(head, tail, 3)
    }

// optional whitespace
_ = empty* { return }
// mandatory whitespace
__ = empty+ { return }

empty
  = [ \t\r\n]+
  / "//" [^\r\n]*
