start
  = _ d:declaration * { return d }

declaration
  = c:configuration _ { return c }
  / f: function _     { return f }

configuration
  = i:ident _ "=" _ e:expression { return ["GSET", i, e] }

function
  = i:ident _ b:block { return ["DEF", i, b] }

block
  = "{" _ s:(statement *) _ "}" { return s }

ident
  = letters:([a-z][a-z0-9]*) { return letters[0] + letters[1].join("") }

tident
  = i:ident ":" t:expression { return [i, t] }
  / i:ident                  { return [i] }

integer
  = digits:[0-9]+ { return parseInt(digits.join(""), 10) }

statement
  = expression

expression
  = integer
  / call
  / tident _ "=" _ expression
  / ident _ "+=" _ expression
  / ident _ "-=" _ expression
  / ident

mutop
  = "+="
  / "-="
  / "*="
  / "/="
  / "%="

mutation
  = i:ident _ o:mutop _ e:expression { return [o, i, e] }

call
  = i:ident _ "(" _ a:args? _ ")" { return ["CALL", i, a || []] }

args
  = e:expression _ "," _ a:args { return [e].concat(a) }
  / e:expression                { return [e] }

// optional whitespace
_ = [ \t\r\n]* { return }

// mandatory whitespace
__ = [ \t\r\n]+ { return }
