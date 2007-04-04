module benchmark.recursive;

// n = 11, 345.122 sec (Fib(38.0) output is off by 2, don't have double precision)

local function ack(m, n)
{
	if(m == 0)
		return n + 1;

	if(n == 0)
		return ack(m - 1, 1);

	return ack(m - 1, ack(m, n - 1));
}

local function fib(n)
{
	if(n < 2)
		return 1;

	return fib(n - 2) + fib(n - 1);
}

local function tak(x, y, z)
{
	if(y >= x)
		return z;

	return tak(tak(x - 1, y, z), tak(y - 1, z, x), tak(z - 1, x, y));
}

local args = [vararg];
local n = 11;

if(#args > 0)
{
	try
		n = toInt(args[0]);
	catch(e) {}
}

local time = os.microTime();

	writefln("Ack(3, %d): %d", n, ack(3, n));
	writefln("Fib(%.1f): %.1f", n + 27.0, fib(n + 27.0));
	
	--n;
	writefln("Tak(%d, %d, %d): %d", 3 * n, 2 * n, n, tak(3 * n, 2 * n, n));
	writefln("Fib(3): %d", fib(3));
	writefln("Tak(3.0, 2.0, 1.0): %.1f", tak(3.0, 2.0, 1.0));
	
time = os.microTime() - time;
writefln("Took ", time / 1000000.0, " sec");