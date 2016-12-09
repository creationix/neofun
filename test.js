var parse = require('./bright-parser').parse;
var code = require('fs').readFileSync("bugs.bright", "utf8");
var inspect = require('util').inspect;
function stripComments(code) {
  return code.replace(/\/\/[^\r\n]*/g, "");
}
code = stripComments(code);

var tree = parse(code);
console.log(inspect(tree, {depth:null,colors:true}));
