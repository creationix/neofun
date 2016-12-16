var parse = require('./bright-parser').parse;
var code = require('fs').readFileSync("bugs.bright", "utf8");
var inspect = require('util').inspect;
try {
  var tree = parse(code);
}
catch (err) {
  if (err.location && err.name === 'SyntaxError') {
    var start = err.location.start;
    var loc = "At line " + start.line + " column " + start.column + ":\n";
    err.message  = loc + err.message;
    err.stack = loc + err.stack;
  }
  throw err;
}
function p(value) {
  console.log(inspect(value, {depth:null,colors:true}));
}

console.log("\nPARSE TREE:");
p(tree);

var types = {};
var names = {
  conf: [],
  var: [],
  native: [],
  func: [],
};

function setType(name, type, fallback) {
  var old = types[name];
  if (old) {
    if (old === type || (fallback && fallback === old)) return;
    throw new Error("Name conflict for '" + name + "' as both '" +
                    old + "' and '" + type + "'");
  }
  types[name] = type;
  names[type].push(name);
}

function nameWalk(node) {
  if (!Array.isArray(node)) return;
  var type = node[0];
  if (Array.isArray(type)) {
    return nameWalk(type);
  }
  if (type === "ASSIGN" ||
      type === "INCRMOD" || type === "DECRMOD" ||
      type === "INCR" || type === "DECR") {
    setType(node[1], "var", "conf");
    return node.slice(2).forEach(nameWalk);
  }
  if (type === "LOOP" && node.length >= 4) {
    setType(node[3], "var", "conf");
    return nameWalk(node[2]);
  }
  if (type === "CALL") {
    setType(node[1], "native", "func");
    return node.slice(2).forEach(nameWalk);
  }
  return node.slice(1).forEach(nameWalk);
}

tree.forEach(function (decl) {
  setType(decl[1], decl[0].toLowerCase());
});

tree.forEach(function (decl) {
  decl.slice(2).forEach(nameWalk);
});

console.log("\nTYPES:");
p(types);
console.log("\nNAMES:");
p(names);

//
// var code = {};
// tree.forEach(function (decl) {
//   var type = decl[0];
//   var name = decl[1];
//   if (type === "GLOBAL") {
//     code[name] = decl[2];
//   }
//   else if (type === "FUNC") {
//     code[name] = stackify(decl.slice(2));
//   }
//   else {
//     throw new Error("Unexpected type: " + type);
//   }
// });
//
// function stackify(tree) {
//   var code = [];
//   p(tree);
// }
