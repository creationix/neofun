var parse = require('./bright-parser').parse;
var code = require('fs').readFileSync("bugs.bright", "utf8");
var inspect = require('util').inspect;
try {
  var tree = parse(code);
}
catch (err) {
  console.log(err);
}
function p(value) {
  console.log(inspect(value, {depth:null,colors:true}));
}

console.log("\nPARSE TREE:");
p(tree);

var names = {};
function setType(name, type) {
  var old = names[name];
  if (old) {
    if (old === type) return;
    if (old === "NATIVE" && type === "FUNC") {
      names[name] = type;
      return;
    }
    if (old === "FUNC" && type === "NATIVE") return;
    throw new Error("Name conflict for '" + name + "' as both '" +
                    old + "' and '" + type + "'");
  }
  names[name] = type;
}
tree.forEach(nameWalk);
function nameWalk(node) {
  if (!Array.isArray(node)) return;
  var type = node[0];
  if (Array.isArray(type)) {
    return nameWalk(type);
  }
  if (type === "CONF" || type === "FUNC") {
    setType(node[1], type);
    return node.slice(2).forEach(nameWalk);
  }
  if (type === "ASSIGN" ||
      type === "INCRMOD" || type === "DECRMOD" ||
      type === "INCR" || type === "DECR") {
    setType(node[1], "VAR");
    return node.slice(2).forEach(nameWalk);
  }
  if (type === "LOOP" && node.length >= 4) {
    setType(node[3], "VAR");
    return nameWalk(node[2]);
  }
  if (type === "CALL") {
    setType(node[1], "NATIVE");
    return node.slice(2).forEach(nameWalk);
  }
  return node.slice(1).forEach(nameWalk);
}

console.log("\nNAMES:");
p(names);

var indexes = {};
var types = {
  conf: [],
  func: [],
  native: [],
  var: [],
};

Object.keys(names).forEach(function (name) {
  var type = names[name];
  var list = types[type.toLowerCase()];
  indexes[name] = list.length;
  list.push(name);
});

console.log("\nINDEXES:");
p(indexes);
console.log("\nTYPES:");
p(types);


var code = {};
tree.forEach(function (decl) {
  var type = decl[0];
  var name = decl[1];
  if (type === "CONF") {
    code[name] = decl[2];
  }
  else if (type === "FUNC") {
    code[name] = stackify(decl.slice(2));
  }
  else {
    throw new Error("Unexpected type: " + type);
  }
});

function stackify(tree) {
  var code = [];
  p(tree);
}
