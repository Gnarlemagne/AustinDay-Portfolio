.intel_syntax noprefix
.section .text

//PROGRAMMING LANGUAGES SPRING 2018 - AUSTIN E. DAY
//  Implementing a simple C++ class with constructor, destructor, class functions and operator functions in assembly
//  to demonstrate understanding of how C++ classes compile with things such as name mangling and assembly logic




//rdi = this
//[rdi+0] = mString
//[rdi+8] = mStringLength

//destructor
.global _ZN8mystringD1Ev
_ZN8mystringD1Ev:
	push	rdi
	mov		rdi, [rdi+0]
	call	free
	pop		rdi

	//store null pointer and length 0
	mov		QWORD PTR [rdi+0], 0
	mov		DWORD PTR [rdi+8], 0

	mov		rax, rdi
	ret


//empty constructor
.global _ZN8mystringC1Ev
_ZN8mystringC1Ev:
	
	//rax = malloc(1)

	push	rdi
	mov		rdi, 0
	call	malloc
	pop		rdi

	//mString = malloc(1)
	//*mString = '\0'
	mov		QWORD PTR [rdi+0], rax
	mov		r10, [rdi+0]
	mov		BYTE PTR [r10], 0
	mov		DWORD PTR [rdi+8], 0

	mov		rax, rdi
	ret


//const char* constructor
.global _ZN8mystringC1EPKc
_ZN8mystringC1EPKc:
	//rsi = src

	// mString = strdup(src)
	push	rdi
	push	rsi
	mov		rdi, rsi
	call	strdup
	pop		rsi
	pop		rdi

	//rax = pointer to new allocated cstring

	mov		[rdi+0], rax

	//mStringLength = strlen(src)

	push	rdi
	mov		rdi, rsi
	call	strlen
	pop		rdi

	//eax = length of string

	mov		DWORD PTR [rdi+8], eax

	mov		rax, rdi

	ret


//const mystring& constructor
.global _ZN8mystringC1ERKS_
_ZN8mystringC1ERKS_:
	//rsi = ptr to mystring rhs
	//[rsi+0] = rhs.mString
	//[rsi+8] = rhs.mStringLength

	//mString = strdup(rhs.mString)

	push	rdi
	push	rsi

	mov		rdi, [rsi+0]
	call	strdup
	pop		rsi
	pop		rdi

	//rax = pointer to duplicated string
	mov		[rdi+0], rax
	
	//mStringLength = rhs.mStringLength
	mov		r10d, [rsi+8]
	mov		DWORD PTR [rdi+8], r10d

	mov		rax, rdi
	ret


//const char* move operator
.global _ZN8mystringaSEPKc
_ZN8mystringaSEPKc:
	
	//rsi = ptr to char *src
	
	//free mString
	push	rdi
	push	rsi
	mov		rdi, [rdi+0]
	call	free
	pop		rsi
	pop 	rdi
	
	//mString = strdup(src)

	push	rdi
	push	rsi
	mov		rdi, rsi
	call	strdup
	pop		rsi
	pop		rdi

	//rax = pointer to new allocated cstring

	mov		[rdi+0], rax

	//mStringLength = strlen(src)

	push	rdi
	mov		rdi, rsi
	call	strlen
	pop		rdi

	//eax = length of string

	mov		DWORD PTR [rdi+8], eax

	mov		rax, rdi
	ret


//const mystring &&rhs rvalue move op
.global _ZN8mystringaSEOKS_
_ZN8mystringaSEOKS_:
	
	//rsi = ptr to temporary mystring rhs

	mov		r9, [rdi+0]
	

	//copy rhs.mString pointer to mString
	mov		r10, [rsi+0]
	mov		[rdi+0], r10

	//copy rhs.mStringLength to this
	mov		r10d, [rsi+8]
	mov		[rdi+8], r10d

	//swap rhs pointer so it frees old mString
	mov		[rsi+0], r9

	mov		rax, rdi
	ret


//length() -- return length
.global _ZNK8mystring6lengthEv
_ZNK8mystring6lengthEv:
	
	mov		eax, [rdi+8]
	ret


//find const mystring&
.global _ZNK8mystring4findERKS_
_ZNK8mystring4findERKS_:

	//r10 = strstr(mString, needle.mString); -- returns pointer to start of substring

	push	rdi
	push	rsi

	mov		rdi, [rdi+0]
	mov		rsi, [rsi+0]
	call	strstr
	
	pop		rsi
	pop		rdi

	mov		r10, rax
	// r10 now holds an address to the substring in mString, or 0 if not found
	// return -1 if r10 = 0
	cmp		r10, 0
	je		1f

	// if here, substring found
	// index of substring = r10 - mString

	sub		r10, [rdi+0]
	mov		eax, r10d
	ret

1:
	//if here, substring not found
	mov		eax, -1
	ret


//find const char* needle
.global _ZNK8mystring4findEPKc
_ZNK8mystring4findEPKc:

	//r10 = strstr(mString, needle)
	
	push	rdi
	push	rsi

	mov		rdi, [rdi+0]
	call	strstr
	
	pop		rsi
	pop		rdi

	mov		r10, rax
	// r10 now holds an address to the substring in mString, or 0 if not found
	// return -1 if r10 = 0
	cmp		r10, 0
	je		1f

	// if here, substring found
	// index of substring = r10 - mString

	sub		r10, [rdi+0]
	mov		eax, r10d
	ret

1:
	//if here, substring not found
	mov		eax, -1
	ret

	

//c_str() -- return mString
.global _ZNK8mystring5c_strEv
_ZNK8mystring5c_strEv:
	mov		rax, [rdi+0]
	ret


//ooh friend!
.global _ZlsRSoRK8mystring
_ZlsRSoRK8mystring:
	//rdi = ostream &s
	//rsi = const mystring &rhs


	//make rsi = rhs.c_str()

	push	rdi
	mov		rsi, [rsi+0]

	call	_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc@plt

	pop		rdi
	mov		rax, rdi
	ret
	

