/******************************************************************************
License:
Copyright (c) 2007 Jarrett Billingsley

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the
use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it freely,
subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
	claim that you wrote the original software. If you use this software in a
	product, an acknowledgment in the product documentation would be
	appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not
	be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.
******************************************************************************/

module minid.arraylib;

import minid.types;

import std.math;

class ArrayLib
{
	this(MDNamespace namespace)
	{
		iteratorClosure = new MDClosure(namespace, &iterator, "array.iterator");
		iteratorReverseClosure = new MDClosure(namespace, &iteratorReverse, "array.iteratorReverse");

		namespace.addList
		(
			"sort"d,     new MDClosure(namespace, &sort,     "array.sort"),
			"reverse"d,  new MDClosure(namespace, &reverse,  "array.reverse"),
			"dup"d,      new MDClosure(namespace, &dup,      "array.dup"),
			"length"d,   new MDClosure(namespace, &length,   "array.length"),
			"opApply"d,  new MDClosure(namespace, &opApply,  "array.opApply"),
			"expand"d,   new MDClosure(namespace, &expand,   "array.expand"),
			"toString"d, new MDClosure(namespace, &ToString, "array.toString"),
			"apply"d,    new MDClosure(namespace, &apply,    "array.apply"),
			"map"d,      new MDClosure(namespace, &map,      "array.map"),
			"reduce"d,   new MDClosure(namespace, &reduce,   "array.reduce"),
			"each"d,     new MDClosure(namespace, &each,     "array.each"),
			"filter"d,   new MDClosure(namespace, &filter,   "array.filter"),
			"find"d,     new MDClosure(namespace, &find,     "array.find"),
			"bsearch"d,  new MDClosure(namespace, &bsearch,  "array.bsearch")
		);

		MDGlobalState().setGlobal("array"d, MDNamespace.create
		(
			"array"d,    MDGlobalState().globals,
			"new"d,      new MDClosure(namespace, &newArray, "array.new"),
			"range"d,    new MDClosure(namespace, &range,    "array.range")
		));
	}

	int newArray(MDState s, uint numParams)
	{
		int length = s.getParam!(int)(0);
		
		if(length < 0)
			s.throwRuntimeException("Invalid length: ", length);
			
		if(numParams == 1)
			s.push(new MDArray(length));
		else
		{
			MDArray arr = new MDArray(length);
			arr[] = s.getParam(1u);
			s.push(arr);
		}

		return 1;
	}
	
	int range(MDState s, uint numParams)
	{
		int v1 = s.getParam!(int)(0);
		int v2;
		int step = 1;

		if(numParams == 1)
		{
			v2 = v1;
			v1 = 0;
		}
		else if(numParams == 2)
			v2 = s.getParam!(int)(1);
		else
		{
			v2 = s.getParam!(int)(1);
			step = s.getParam!(int)(2);
		}

		if(step <= 0)
			s.throwRuntimeException("Step may not be negative or 0");
		
		int range = abs(v2 - v1);
		int size = range / step;

		if((range % step) != 0)
			size++;

		MDArray ret = new MDArray(size);
		
		int val = v1;

		if(v2 < v1)
		{
			for(int i = 0; val > v2; i++, val -= step)
				*ret[i] = val;
		}
		else
		{
			for(int i = 0; val < v2; i++, val += step)
				*ret[i] = val;
		}

		s.push(ret);
		return 1;
	}

	int sort(MDState s, uint numParams)
	{
		MDArray arr = s.getContext!(MDArray);
		arr.sort();
		s.push(arr);

		return 1;
	}
	
	int reverse(MDState s, uint numParams)
	{
		MDArray arr = s.getContext!(MDArray);
		arr.reverse();
		s.push(arr);
		
		return 1;
	}
	
	int dup(MDState s, uint numParams)
	{
		s.push(s.getContext!(MDArray).dup);
		return 1;
	}
	
	int length(MDState s, uint numParams)
	{
		MDArray arr = s.getContext!(MDArray);
		int length = s.getParam!(int)(0);

		if(length < 0)
			s.throwRuntimeException("Invalid length: ", length);

		arr.length = length;

		s.push(arr);
		return 1;
	}

	int iterator(MDState s, uint numParams)
	{
		MDArray array = s.getContext!(MDArray);
		int index = s.getParam!(int)(0);

		index++;
		
		if(index >= array.length)
			return 0;
			
		s.push(index);
		s.push(array[index]);
		
		return 2;
	}

	int iteratorReverse(MDState s, uint numParams)
	{
		MDArray array = s.getContext!(MDArray);
		int index = s.getParam!(int)(0);
		
		index--;

		if(index < 0)
			return 0;
			
		s.push(index);
		s.push(array[index]);
		
		return 2;
	}
	
	MDClosure iteratorClosure;
	MDClosure iteratorReverseClosure;
	
	int opApply(MDState s, uint numParams)
	{
		MDArray array = s.getContext!(MDArray);

		if(s.isParam!("string")(0) && s.getParam!(MDString)(0) == "reverse"d)
		{
			s.push(iteratorReverseClosure);
			s.push(array);
			s.push(cast(int)array.length);
		}
		else
		{
			s.push(iteratorClosure);
			s.push(array);
			s.push(-1);
		}

		return 3;
	}
	
	int expand(MDState s, uint numParams)
	{
		MDArray array = s.getContext!(MDArray);
		
		for(int i = 0; i < array.length; i++)
			s.push(array[i]);
			
		return array.length;
	}
	
	int ToString(MDState s, uint numParams)
	{
		MDArray array = s.getContext!(MDArray);
		
		char[] str = "[";

		for(int i = 0; i < array.length; i++)
		{
			if(array[i].isString())
				str ~= '"' ~ array[i].toString() ~ '"';
			else
				str ~= array[i].toString();
			
			if(i < array.length - 1)
				str ~= ", ";
		}

		s.push(str ~ "]");
		
		return 1;
	}
	
	int apply(MDState s, uint numParams)
	{
		MDArray array = s.getContext!(MDArray);
		MDClosure func = s.getParam!(MDClosure)(0);
		MDValue arrayVal = array;

		foreach(i, v; array)
		{
			s.easyCall(func, 1, arrayVal, v);
			array[i] = s.pop();
		}

		s.push(array);
		return 1;
	}
	
	int map(MDState s, uint numParams)
	{
		MDArray array = s.getContext!(MDArray);
		MDClosure func = s.getParam!(MDClosure)(0);
		MDValue arrayVal = array;
		
		MDArray ret = new MDArray(array.length);

		foreach(i, v; array)
		{
			s.easyCall(func, 1, arrayVal, v);
			ret[i] = s.pop();
		}

		s.push(ret);
		return 1;
	}
	
	int reduce(MDState s, uint numParams)
	{
		MDArray array = s.getContext!(MDArray);
		MDClosure func = s.getParam!(MDClosure)(0);
		MDValue arrayVal = array;

		if(array.length == 0)
		{
			s.pushNull();
			return 1;
		}

		MDValue ret = array[0];
		
		for(int i = 1; i < array.length; i++)
		{
			s.easyCall(func, 1, arrayVal, ret, array[i]);
			ret = s.pop();
		}
		
		s.push(ret);
		return 1;
	}
	
	int each(MDState s, uint numParams)
	{
		MDArray array = s.getContext!(MDArray);
		MDClosure func = s.getParam!(MDClosure)(0);
		MDValue arrayVal = array;

		foreach(i, v; array)
		{
			s.easyCall(func, 1, arrayVal, i, v);

			MDValue ret = s.pop();
		
			if(ret.isBool() && ret.as!(bool)() == false)
				break;
		}

		s.push(array);
		return 1;
	}
	
	int filter(MDState s, uint numParams)
	{
		MDArray array = s.getContext!(MDArray);
		MDClosure func = s.getParam!(MDClosure)(0);
		MDValue arrayVal = array;
		
		MDArray retArray = new MDArray(array.length / 2);
		uint retIdx = 0;

		foreach(i, v; array)
		{
			s.easyCall(func, 1, arrayVal, i, v);

			if(s.pop!(bool)() == true)
			{
				if(retIdx >= retArray.length)
					retArray.length = retArray.length + 10;

				retArray[retIdx] = v;
				retIdx++;
			}
		}
		
		retArray.length = retIdx;
		s.push(retArray);
		return 1;
	}
	
	int find(MDState s, uint numParams)
	{
		MDArray array = s.getContext!(MDArray);
		MDValue val = s.getParam(0u);
		
		foreach(i, v; array)
		{
			if(val.type == v.type && val.compare(&v) == 0)
			{
				s.push(i);
				return 1;
			}
		}
		
		s.push(-1);
		return 1;
	}
	
	int bsearch(MDState s, uint numParams)
	{
		MDArray array = s.getContext!(MDArray);
		MDValue val = s.getParam(0u);

		uint lo = 0;
		uint hi = array.length - 1;
		uint mid = (lo + hi) >> 1;

		while((hi - lo) > 8)
		{
			int cmp = val.compare(array[mid]);
			
			if(cmp == 0)
			{
				s.push(mid);
				return 1;
			}
			else if(cmp < 0)
				hi = mid;
			else
				lo = mid;
				
			mid = (lo + hi) >> 1;
		}

		for(int i = lo; i <= hi; i++)
		{
			if(val.compare(array[i]) == 0)
			{
				s.push(i);
				return 1;
			}
		}

		s.push(-1);
		return 1;
	}
}

public void init()
{
	MDNamespace namespace = new MDNamespace("array"d, MDGlobalState().globals);
	new ArrayLib(namespace);
	MDGlobalState().setMetatable(MDValue.Type.Array, namespace);
}