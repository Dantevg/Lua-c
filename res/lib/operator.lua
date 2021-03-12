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
function op.div(a, b) return a / b end
function op.mod(a, b) return a % b end
function op.pow(a, b) return a ^ b end
function op.idiv(a, b) return a // b end
function op.band(a, b) return a & b end
function op.bor(a, b) return a | b end
function op.bxor(a, b) return a ~ b end
function op.shl(a, b) return a << b end
function op.shr(a, b) return a >> b end

function op.unm(a) return -a end
function op.bnot(a) return ~a end

-- Logic operators
function op.land(a, b) return a and b end
function op.lor(a, b) return a or b end

function op.lnot(a) return not a end

-- Other operators
function op.call(f, ...) return f(...) end
function op.id(...) return ... end

function op.newindex(t, k, v) t[k] = v end

function op.concat(a, b) return a .. b end
function op.index(t, k) return t[k] end

function op.length(a) return #a end

function op.nop() end

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

op["and"] = op.land
op["or"] = op.lor
op["not"] = op.lnot

op["()"] = op.call
op[""] = op.id
op[".."] = op.concat
op["[]"] = op.index
op["#"] = op.length

-- Classes
op.comparison = {
	[op.eq]  = true, ["=="] = true, eq  = true,
	[op.neq] = true, ["~="] = true, neq = true,
	[op.lt]  = true, ["<"]  = true, lt  = true,
	[op.gt]  = true, [">"]  = true, gt  = true,
	[op.leq] = true, ["<="] = true, leq = true, le = true,
	[op.geq] = true, [">="] = true, geq = true, ge = true,
}

op.arithmetic = {
	[op.add]  = true, ["+"]  = true, add  = true,
	[op.sub]  = true, ["-"]  = true, sub  = true,
	[op.mul]  = true, ["*"]  = true, mul  = true,
	[op.div]  = true, ["/"]  = true, div  = true,
	[op.mod]  = true, ["%"]  = true, mod  = true,
	[op.pow]  = true, ["^"]  = true, pow  = true,
	[op.unm]  = true, ["-"]  = true, unm  = true,
	[op.idiv] = true, ["//"] = true, idiv = true,
	[op.band] = true, ["&"]  = true, band = true,
	[op.bor]  = true, ["|"]  = true, bor  = true,
	[op.bxor] = true, ["~"]  = true, bxor = true,
	[op.bnot] = true, ["~"]  = true, bnot = true,
	[op.shl]  = true, ["<<"] = true, shl  = true,
	[op.shr]  = true, [">>"] = true, shr  = true,
}

op.logic = {
	[op.land] = true, ["and"] = true,
	[op.lor]  = true, ["or"]  = true,
	[op.lnot] = true, ["not"] = true,
}

-- Metamethods
op.mt = {
	__add      = op.add,
	__sub      = op.sub,
	__mul      = op.mul,
	__div      = op.div,
	__mod      = op.mod,
	__pow      = op.pow,
	__unm      = op.unm,
	__idiv     = op.idiv,
	__band     = op.band,
	__bor      = op.bor,
	__bxor     = op.bxor,
	__bnot     = op.bnot,
	__shl      = op.shl,
	__shr      = op.shr,
	__concat   = op.concat,
	__len      = op.len,
	__eq       = op.eq,
	__neq      = op.neq,
	__lt       = op.lt,
	__le       = op.le,
	__index    = op.index,
	__newindex = op.newindex,
	__call     = op.call,
}

return op