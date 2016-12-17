#!/usr/bin/env node
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
    console.error(loc + err.message);
    process.exit(-1);
  }
  throw err;
}
function p(value) {
  console.error(inspect(value, {depth:null,colors:true}));
}

console.error("\nPARSE TREE:");
p(tree);

console.error("\nASSEMBLY CODE:");
var code = tree.compile();
p(code);

console.error("\nPACKAGE:");
var parts = tree.package();
p(parts);
process.stdout.write(Buffer.concat(parts));
