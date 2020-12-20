local op = {}

-- Comparison operators
function op.eq(a, b) return a == b end
function op.neq(a, b) return a ~= b end
function op.lt(a, b) return a < b end
function op.gt(a, b) return a > b end
function op.leq(a, b) return a <= b end
function op.geq(a, b) return a >= b end

-- Arithmetic operators
function op.add(a, b) return a + b end
function op.sub(a, b) return a - b end
function op.mul(a, b) return a * b end
function op.mod(a, b) return a % b end
function op.pow(a, b) return a ^ b end
function op.div(a, b) return a / b end
function op.idiv(a, b) return a // b end
function op.band(a, b) return a & b end
function op.bor(a, b) return a | b end
function op.bxor(a, b) return a ~ b end
function op.shl(a, b) return a << b end
function op.shr(a, b) return a >> b end

function op.unm(a) return -a end
function op.bnot(a) return ~a end

-- Other operators
function op.concat(a, b) return a .. b end
function op.index(a, b) return a[b] end
function op.call(a, b) return a(b) end

function op.length(a) return #a end

-- Aliases
op.len = op.length
op.ge = op.geq
op.le = op.leq

-- Symbols
op["=="] = op.eq
op["~="] = op.neq
op["<"] = op.lt
op[">"] = op.gt
op["<="] = op.leq
op[">="] = op.geq

op["+"] = op.add
op["-"] = op.sub
op["*"] = op.mul
op["%"] = op.mod
op["^"] = op.pow
op["/"] = op.div
op["//"] = op.idiv
op["&"] = op.band
op["|"] = op.bor
op["~"] = op.bxor
op["<<"] = op.shl
op[">>"] = op.shr

-- already taken by their binary variants
-- op["-"] = op.unm
-- op["~"] = op.bnot

op[".."] = op.concat
op["[]"] = op.index
op["()"] = op.call

op["#"] = op.length

return op