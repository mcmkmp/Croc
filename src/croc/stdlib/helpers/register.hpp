#ifndef CROC_STDLIB_HELPERS_REGISTER_HPP
#define CROC_STDLIB_HELPERS_REGISTER_HPP

#include "croc/api.h"
#include "croc/types/base.hpp"

namespace croc
{
#define DModule CROC_DOC_MODULE
#define DFunc CROC_DOC_FUNC
#define DClass CROC_DOC_CLASS
#define DNs CROC_DOC_NS
#define DField CROC_DOC_FIELD
#define DFieldV CROC_DOC_FIELDV
#define DVar CROC_DOC_VAR
#define DVarV CROC_DOC_VARV
#define DBase CROC_DOC_BASE
#define DParamAny CROC_DOC_PARAMANY
#define DParamAnyD CROC_DOC_PARAMANYD
#define DParam CROC_DOC_PARAM
#define DParamD CROC_DOC_PARAMD
#define DVararg CROC_DOC_VARARG

#define DBeginList(name) const StdlibRegister name[] = {{
#define DListSep() },{
#define DEndList() },{nullptr, nullptr, 0, nullptr}};

#define _DListItem(name) {name##_info, &name},
#define _DListEnd {{nullptr, nullptr, 0}, nullptr}

	constexpr const char* Docstr(const char* s)
	{
#ifdef CROC_BUILTIN_DOCS
		return s;
#else
		return s - s; // teehee sneaky way to return nullptr AND use s
#endif
	}

	struct StdlibRegister
	{
		const char* docs;
		const char* name;
		word maxParams;
		CrocNativeFunc func;
	};

	struct _StdlibRegisterInfo
	{
		const char* docs;
		const char* name;
		word maxParams;
	};

	struct _StdlibRegister
	{
		_StdlibRegisterInfo info;
		CrocNativeFunc func;
	};

	void registerGlobals(CrocThread* t, const StdlibRegister* funcs);
	void registerFields(CrocThread* t, const StdlibRegister* funcs);
	void registerMethods(CrocThread* t, const StdlibRegister* funcs);
	void registerGlobalUV(CrocThread* t, const StdlibRegister* func);
	void registerFieldUV(CrocThread* t, const StdlibRegister* func);
	void registerMethodUV(CrocThread* t, const StdlibRegister* func);
	void registerGlobal(CrocThread* t, const StdlibRegister& func, uword numUVs);
	void registerField(CrocThread* t, const StdlibRegister& func, uword numUVs);
	void registerMethod(CrocThread* t, const StdlibRegister& func, uword numUVs);
#ifdef CROC_BUILTIN_DOCS
	void docGlobals(CrocDoc* d, const StdlibRegister* funcs);
	void docFields(CrocDoc* d, const StdlibRegister* funcs);
	void docGlobalUV(CrocDoc* d, const StdlibRegister* func);
	void docFieldUV(CrocDoc* d, const StdlibRegister* func);
	void docGlobal(CrocDoc* d, const StdlibRegister& func);
	void docField(CrocDoc* d, const StdlibRegister& func);
#endif

	void _registerGlobals(CrocThread* t, const _StdlibRegister* funcs);
	void _registerFields(CrocThread* t, const _StdlibRegister* funcs);
	void _registerMethods(CrocThread* t, const _StdlibRegister* funcs);
	void _registerGlobalUV(CrocThread* t, const _StdlibRegister* func);
	void _registerFieldUV(CrocThread* t, const _StdlibRegister* func);
	void _registerMethodUV(CrocThread* t, const _StdlibRegister* func);
	void _registerGlobal(CrocThread* t, const _StdlibRegister& func, uword numUVs);
	void _registerField(CrocThread* t, const _StdlibRegister& func, uword numUVs);
	void _registerMethod(CrocThread* t, const _StdlibRegister& func, uword numUVs);
#ifdef CROC_BUILTIN_DOCS
	void _docGlobals(CrocDoc* d, const _StdlibRegister* funcs);
	void _docFields(CrocDoc* d, const _StdlibRegister* funcs);
	void _docGlobalUV(CrocDoc* d, const _StdlibRegister* func);
	void _docFieldUV(CrocDoc* d, const _StdlibRegister* func);
	void _docGlobal(CrocDoc* d, const _StdlibRegister& func);
	void _docField(CrocDoc* d, const _StdlibRegister& func);
#endif

}

#endif