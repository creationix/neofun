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

console.log("\nSTACK CODE:");
var code = tree.compile();
p(code);
