{
  function list(head, tail, index) {
    return [head].concat(tail.map(entry => entry[index]));
  }
}

start
  = _ program:Program? _ { return program }

Program
  = head:Declaration tail:(__ Declaration)* { return list(head, tail, 1) }

Declaration
  = Configuration
  / Function

Configuration
  = i:ident _ "=" _ e:expression { return ["CONF", i, e] }

Function
  = i:ident _ b:block { return ["FUNC", i].concat(b) }

block
  = "{" _ c:code? _ "}" { return c || [] }

code
  = head:statement tail:(__ statement)* { return list(head, tail, 1) }

ident
  = letters:([a-z][a-z0-9]*) { return letters[0] + letters[1].join("") }

statement
  = "if" _ e:expression _ b:block
    elves:( _ "elf" _ expression _ block)*
    last:( _ "else" _ block)? {
      var list = ["IF", e, b];
      if (elves) {
        elves.forEach(function (elf) {
          list.push(elf[3], elf[5]);
        });
      }
      if (last) {
        list.push(1, last[3])
      }
      return list;
    }
  / expression


expression = e1

e1
  = i:ident t:(":" length)? _ "=" _ v:e1 {
      return t ? ["ASSIGN", i, v, t[1]] : ["ASSIGN", i, v]
    }
  / i:ident _ "+=" _ v:e1 {
      return ["INCR", i, v]
    }
  / i:ident _ "-=" _ v:e1 {
      return ["DECR", i, v]
    }
  / e2

e2
  = left:e3 _ "or" _ right:e2 {
      return ["OR", left, right]
    }
  / left:e3 _ "xor" _ right:e2 {
      return ["XOR", left, right]
    }
  / e3

e3
  = left:e4 _ "and" _ right:e3 {
      return ["AND", left, right]
    }
  / e4

e4
  = left:e5 _ "==" _ right:e4 {
      return ["EQ", left, right]
    }
  / left:e5 _ "!=" _ right:e4 {
      return ["NEQ", left, right]
    }
  / e5

e5
  = left:e6 _ "<" _ right:e5 {
      return ["LT", left, right]
    }
  / left:e6 _ "<=" _ right:e5 {
      return ["LTE", left, right]
    }
  / left:e6 _ ">" _ right:e5 {
      return ["GT", left, right]
    }
  / left:e6 _ ">=" _ right:e5 {
      return ["GTE", left, right]
    }
  / e6

e6
  = left:e7 _ "+" _ right:e6 {
      return ["ADD", left, right]
    }
  / left:e7 _ "-" _ right:e6 {
      return ["SUB", left, right]
    }
  / e7

e7
  = left:e8 _ "*" _ right:e7 {
      return ["MUL", left, right]
    }
  / left:e8 _ "/" _ right:e7 {
      return ["DIV", left, right]
    }
  / left:e8 _ "%" _ right:e7 {
      return ["MOD", left, right]
    }
  / e8

e8
  = "-" _ right:e9 { return ["NEG", right] }
  / "not" _ right:e9 { return ["NOT"], right }
  / e9

e9
  = "(" _ e:e1 _ ")" { return e }
  / Integer
  / Call
  / Variable

Integer
  = digits:([-+]?[0-9]+) {
      return parseInt((digits[0]||"") + digits[1].join(""), 10)
    }

length
  = Variable
  / Integer

Variable
  = i:ident { return i }

Call
  = i:ident _ "(" _ a:args? _ ")" { return ["CALL", i].concat(a || []) }

args
  = head:expression tail:(_ "," _ expression)* {
      return list(head, tail, 3)
    }

// optional whitespace
_ = [ \t\r\n]* { return }

// mandatory whitespace
__ = [ \t\r\n]+ { return }
