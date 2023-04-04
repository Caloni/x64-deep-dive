/* This code is for x64 debugging tests.

It follows the track from the article X64 Deep Dive 
https://codemachine.com/articles/x64_deep_dive.html
and recreate some source code to show how Visual Studio is
generating the x64 code.

@author Wanderley Caloni <wanderley.caloni@gmail.com>
@date 2023-04
*/

/*
int FastCall(int a, int b, int c, int d) {
mov         dword ptr [a],r9d
mov         dword ptr [rsp+18h],r8d # RBP is no longer used as frame pointer.
mov         dword ptr [rsp+10h],edx
mov         dword ptr [rsp+8],ecx
push        rdi
sub         rsp,10h
	int e = a + b + c + d;
mov         eax,dword ptr [b]
mov         ecx,dword ptr [a]
add         ecx,eax
mov         eax,ecx
add         eax,dword ptr [c]
add         eax,dword ptr [d]
mov         dword ptr [rsp],eax
	return e;
mov         eax,dword ptr [rsp]
}
add         rsp,10h # RBP is no longer used as frame pointer.
pop         rdi
ret
*/
int FastCall(int a, int b, int c, int d) {
	int e = a + b + c + d;
	return e;
}


/*
"Fastcall registers are used to pass parameters to functions. Fastcall is the
default calling convention on X64 where in the first 4 parameters are passed via
the registers RCX, RDX, R8, R9." - X64 Deep Dive

int TestFastCall() {
push        rdi
sub         rsp,30h # RBP is no longer used as frame pointer.
	int res = FastCallTest(1, 2, 3, 4);
mov         r9d,4
mov         r8d,3
mov         edx,2
mov         ecx,1
call        FastCall
mov         dword ptr [res],eax
	return res;
mov         eax,dword ptr [res]
}
add         rsp,30h # RBP is no longer used as frame pointer.
pop         rdi
ret
*/
int TestFastCall() {
	int res = FastCall(1, 2, 3, 4);
	return res;
}


int TailCall1(int a) {
	return a;
}


int TailCall2(int b) {
	return b;
}


int TailCall3(int a, int b) {
	return 1;
}


/*
"X64 compiler can optimize the last call made from a function by replacing it
with a jump to the callee. This avoids the overhead of setting up the stack
frame for the callee." - x64 Deep Dive

# TailCall3
	return a + b;
lea         eax,[rcx+rdx]
}
ret

# TestTailCallElimination
	TailCall1(1);
	TailCall2(2);
	return TailCall3(1, 2);
mov         edx,2
lea         ecx,[rdx-1]
jmp         TailCall3

*/
int TestTailCallElimination() {
	TailCall1(1);
	TailCall2(2);
	return TailCall3(1, 2);
}


/*
"Unlike the X86 CPU where the EBP register is used to access parameters and
local variables on the stack, X64 functions do not make use of the RBP register
for this purpose i.e. do not use the EBP register as a
frame pointer." - x64 Deep Dive

# Win32 (x86)
int TestFramePointerOmission() {
00601590  push        ebp
00601591  mov         ebp,esp
	return 1;
00601593  mov         eax,1
}
00601598  pop         ebp
00601599  ret

# Win62 (x64)
int TestFramePointerOmission() {
00007FF7D6E215E0  push        rdi
	return 1;
00007FF7D6E215E2  mov         eax,1
}
00007FF7D6E215E7  pop         rdi
00007FF7D6E215E8  ret
*/
int TestFramePointerOmission() {
	return 1;
}


int RSPIsTheSameCall1(int p1) {
	return p1;
}


int RSPIsTheSameCall4(int p1, int p2, int p3, int p4) {
	return p1 + p2 + p3 + p4;
}


int RSPIsTheSameCall8(int p1, int p2, int p3, int p4, int p5, int p6, int p7, int p8) {
	return p1 + p2 + p3 + p4 + p5 + p6 + p7 + p8;
}


/*
Since RSP is used to reference both parameters and local variables in x64,
the side effect and feature of x64 function is that RSP does not change
thru all its body, changing only in prolog (begin) and epilog (end) parts
of the function.

# Win32
int RSPIsTheSame(int p1, int p2, int p3, int p4, int p5, int p6, int p7, int p8) {
# prolog-begin
push        ebp
mov         ebp,esp
	RSPIsTheSameCall1(p1);
mov         eax,dword ptr [p1]
push        eax # RSP--
call        RSPIsTheSameCall1
add         esp,4 # RSP++
# prolog-end
	RSPIsTheSameCall4(p1, p2, p3, p4);
mov         ecx,dword ptr [p4]
push        ecx # RSP--
mov         edx,dword ptr [p3]
push        edx # RSP--
mov         eax,dword ptr [p2]
...
call        RSPIsTheSameCall4
add         esp,10h # RSP++
	RSPIsTheSameCall8(p1, p2, p3, p4, p5, p6, p7, p8);
mov         edx,dword ptr [p8]
push        edx # RSP--
...
call        RSPIsTheSameCall8
add         esp,20h # RSP++
	return 1;
mov         eax,1
}
# epilog-begin
pop         ebp
# epilog-end
ret

# Win64
int RSPIsTheSame(int p1, int p2, int p3, int p4, int p5, int p6, int p7, int p8) {
# prolog-begin
mov         dword ptr [rsp+20h],r9d
mov         dword ptr [rsp+18h],r8d
mov         dword ptr [rsp+10h],edx
mov         dword ptr [rsp+8],ecx
push        rdi
sub         rsp,40h # RSP last change
# prolog-end
	RSPIsTheSameCall1(p1);
mov         ecx,dword ptr [p1]
call        RSPIsTheSameCall1
	RSPIsTheSameCall4(p1, p2, p3, p4);
mov         r9d,dword ptr [p4]
...
call        RSPIsTheSameCall4
	RSPIsTheSameCall8(p1, p2, p3, p4, p5, p6, p7, p8);
mov         eax,dword ptr [p8]
mov         dword ptr [rsp+38h],eax
...
call        RSPIsTheSameCall8
	return 1;
mov         eax,1
}
# epilog-begin
add         rsp,40h # RSP restore
pop         rdi
# epilog-end
ret
*/
int RSPIsTheSame(int p1, int p2, int p3, int p4, int p5, int p6, int p7, int p8) {
	RSPIsTheSameCall1(p1);
	RSPIsTheSameCall4(p1, p2, p3, p4);
	RSPIsTheSameCall8(p1, p2, p3, p4, p5, p6, p7, p8);
	return 1;
}


int TestRSPIsTheSame() {
    return RSPIsTheSame(1, 2, 3, 4, 5, 6, 7, 8);
}


/*
# Win64
	return p1 + 1 + p2 + 1 + p3 + 1 + p4;
lea         eax,[rdx+3]
add         eax,ecx
add         eax,r8d
add         eax,r9d
# Win32 is similar, but x64 is more elegant =)
*/
int HomingSpace3(int p1, int p2, int p3, int p4) {
	return p1 + 1 + p2 + 1 + p3 + 1 + p4;
}


/*
# Win32
int HomingSpace2(int p1, int p2, int p3, int p4) {
push        ebp
mov         ebp,esp
	return HomingSpace3(p1 + 1, p2 + 1, p3 + 1, p4 + 1);
mov         eax,dword ptr [p4]
inc         eax
mov         dword ptr [p4],eax
inc         dword ptr [p3] # same logic: original params lost...
inc         dword ptr [p2]
inc         dword ptr [p1]
}
pop         ebp # ... and saving stack frame
	return HomingSpace3(p1 + 1, p2 + 1, p3 + 1, p4 + 1);
jmp         HomingSpace3 # ret right to HomingSpace

# Win64
	return HomingSpace3(p1 + 1, p2 + 1, p3 + 1, p4 + 1);
inc         r9d # original params lost
inc         r8d
inc         edx
inc         ecx
jmp         HomingSpace3 # ret right to HomingSpace
*/
int HomingSpace2(int p1, int p2, int p3, int p4) {
	return HomingSpace3(p1 + 1, p2 + 1, p3 + 1, p4 + 1);
}


/*
# Win32 Release
int HomingSpace1(int p1, int p2, int p3, int p4) {
push        ebp
mov         ebp,esp
	return HomingSpace2(p1 + 1, p2 + 1, p3 + 1, p4 + 1);
mov         eax,dword ptr [p4]
inc         eax
mov         dword ptr [p4],eax # original parameter lost
inc         dword ptr [p3] # original parameter lost
inc         dword ptr [p2] # original parameter lost
inc         dword ptr [p1] # original parameter lost
}
pop         ebp # stack frame recycled
	return HomingSpace2(p1 + 1, p2 + 1, p3 + 1, p4 + 1);
jmp         HomingSpace2 # ret to HomingSpace

# Win32 Debug
	return HomingSpace2(p1 + 1, p2 + 1, p3 + 1, p4 + 1);
mov         eax,dword ptr [p4]
add         eax,1
push        eax # original params saved in the stack
...
call        HomingSpace2

# Win64 Release
	return HomingSpace2(p1 + 1, p2 + 1, p3 + 1, p4 + 1);
inc         r9d # original parameter lost
inc         r8d # original parameter lost
inc         edx # original parameter lost
inc         ecx # original parameter lost
jmp         HomingSpace2 # no stack at all; ret right to HomingSpace

# Win64 Debug
int HomingSpace1(int p1, int p2, int p3, int p4) {
mov         dword ptr [rsp+20h],r9d # homing space in action
mov         dword ptr [rsp+18h],r8d
mov         dword ptr [rsp+10h],edx
mov         dword ptr [rsp+8],ecx
push        rdi
sub         rsp,30h
	return HomingSpace2(p1 + 1, p2 + 1, p3 + 1, p4 + 1);
mov         eax,dword ptr [p4]
inc         eax
mov         ecx,dword ptr [p3]
inc         ecx # educated, but need to overwrite params
...
call        HomingSpace2
*/
int HomingSpace1(int p1, int p2, int p3, int p4) {
	return HomingSpace2(p1 + 1, p2 + 1, p3 + 1, p4 + 1);
}


/*
"(...) homing space and is used to store parameter values if either the function
accesses the parameters by address instead of by value or if the function is
compiled with the /homeparams flag. The minimum size of this homing space is
0x20 bytes or four 64-bit slots, even if the function takes less than 4
parameters. When the homing space is not used to store parameter values, the
compiler uses it to save non-volatile registers." - x64 Deep Dive

However, even in x86 those arguments can be lost if there is some optimization
for parameters that are not used anymore, which is the case in this test. Note
the recycle of positions in the stack and the economy of stack frames to
call the nesting functions above.

# Win32
	int ret = HomingSpace1(1, 2, 3, 4);
push        4
push        3
push        2
push        1
call        HomingSpace1

# Win64
	int ret = HomingSpace1(1, 2, 3, 4);
mov         edx,2
lea         r9d,[rdx+2]
lea         r8d,[rdx+1]
lea         ecx,[rdx-1]
call        HomingSpace1
*/
int TestHomingSpace() {
    int ret = HomingSpace1(1, 2, 3, 4);
	return ret == 21 ? 1 : 0;
}


/*
"The register based parameter homing space exists only for non-leaf 
functions." - x64 Deep Dive

# Win64 Debug
int HomingSpaceLeaf(int p1, int p2, int p3, int p4) {
mov         dword ptr [p3],r9d # even begin leaf function, params are saved
mov         dword ptr [p2],r8d
mov         dword ptr [p1],edx
mov         dword ptr [rsp+8],ecx
push        rdi

# Win64 Release
	return p1 + p2 + p3 + p4;
lea         eax,[rcx+rdx] # params not saved (neither has stack frame)
add         eax,r8d
add         eax,r9d
}
*/
int HomingSpaceLeaf(int p1, int p2, int p3, int p4) {
	return p1 + p2 + p3 + p4;
}


/*
"The register based parameter homing space exists only for non-leaf 
functions." - x64 Deep Dive

# Win64 Debug
int HomingSpaceNonLeaf(int p1, int p2, int p3, int p4) {
mov         dword ptr [rsp+20h],r9d # homing space saving params
mov         dword ptr [rsp+18h],r8d
mov         dword ptr [rsp+10h],edx
mov         dword ptr [rsp+8],ecx
push        rdi
sub         rsp,20h

# Win64 Release
int TestHomingSpaceNonLeaf() {
sub         rsp,28h
	int ret = HomingSpaceNonLeaf(1, 2, 3, 4);
mov         edx,2 # even begin non-leaf function, params are not saved
lea         r9d,[rdx+2]
lea         r8d,[rdx+1]
lea         ecx,[rdx-1]
call        HomingSpaceNonLeaf
*/
int HomingSpaceNonLeaf(int p1, int p2, int p3, int p4) {
	return HomingSpaceLeaf(p1, p2, p3, p4);
}


int TestHomingSpaceNonLeaf() {
    int ret = HomingSpaceNonLeaf(1, 2, 3, 4);
	return ret == 10 ? 1 : 0;
}


/*
# Win64 Debug
			p5 = i + j + (i % 2 ? p1 : p2) + (j % 2 ? p3 : p4);
mov         eax,dword ptr [rsp+0Ch]
cdq
and         eax,1
xor         eax,edx
sub         eax,edx
test        eax,eax
je          ChildSPF3+0BBh
mov         rax,qword ptr [p1]
mov         eax,dword ptr [rax]
mov         dword ptr [rsp+14h],eax # reference child-sp
jmp         ChildSPF3+0C6h
mov         rax,qword ptr [p2]
mov         eax,dword ptr [rax]
mov         dword ptr [rsp+14h],eax # reference child-sp
mov         eax,dword ptr [rsp+10h] # reference child-sp
cdq
and         eax,1
xor         eax,edx
sub         eax,edx
test        eax,eax
je          ChildSPF3+0E3h
mov         rax,qword ptr [p3]
mov         eax,dword ptr [rax]
mov         dword ptr [rsp+18h],eax # reference child-sp
jmp         ChildSPF3+0EEh
mov         rax,qword ptr [p4]
mov         eax,dword ptr [rax]
mov         dword ptr [rsp+18h],eax # reference child-sp
mov         eax,dword ptr [rsp+10h] # reference child-sp
mov         ecx,dword ptr [rsp+0Ch] # reference child-sp
add         ecx,eax
mov         eax,ecx
add         eax,dword ptr [rsp+14h] # reference child-sp
add         eax,dword ptr [rsp+18h] # reference child-sp
mov         rcx,qword ptr [p5]
mov         dword ptr [rcx],eax
*/
int ChildSPF3(int& p1, int& p2, int& p3, int& p4, int& p5, int& p6, int& p7, int& p8) {
	int ret = 0;
	int lv1 = p1 + p2 + p3 + p4;
	int lv2 = p5 + p6 + p7 + p8;

	for (int i = 0; i < 50; ++i) {
		for (int j = 0; j < 60; ++j) {
			p5 = i + j + (i % 2 ? p1 : p2) + (j % 2 ? p3 : p4);
			p6 = i + j + (i % 2 ? p5 : p6) + (j % 2 ? p7 : p8);
			ret += lv1 + lv2 + p5 + p6;
		}
	}

	return ret;
}


int ChildSPF2(int& p1, int& p2, int& p3, int& p4, int& p5, int& p6, int& p7, int& p8) {
	int ret = 0;

	for (int i = 0; i < 30; ++i) {
		for (int j = 0; j < 40; ++j) {
			p7 = i + j + (i % 2 ? p1 : p2) + (j % 2 ? p3 : p4);
			p8 = i + j + (i % 2 ? p5 : p6) + (j % 2 ? p7 : p8);
		}
	}

	int lv1 = ChildSPF3(p1, p2, p3, p4, p5, p6, p7, p8);
	int lv2 = ChildSPF3(p8, p7, p6, p5, p4, p3, p2, p1);
    ret += lv1 + lv2 + p1 + p2 + p7 + p8;


	return ret;
}


int ChildSPF1(int& p1, int& p2, int& p3, int& p4, int& p5, int& p6, int& p7, int& p8) {
	int ret = 0;

	for (int i = 0; i < 10; ++i) {
		for (int j = 0; j < 20; ++j) {
			p7 = i + j + (i % 2 ? p1 : p2) + (j % 2 ? p3 : p4);
			p8 = i + j + (i % 2 ? p5 : p6) + (j % 2 ? p7 : p8);
		}
	}

	int lv1 = ChildSPF3(p1, p2, p3, p4, p5, p6, p7, p8);
	int lv2 = ChildSPF3(p8, p7, p6, p5, p4, p3, p2, p1);
    ret += lv1 + lv2 + p1 + p2 + p7 + p8;


	return ret;
}


/*
"The value of the Child-SP register displayed by the debugger's "k" command
represents the address at which the stack pointer (RSP) points to, as the point
where the function displayed in that frame, has finished executing its prolog.
The next item that would be pushed on the stack would be the return address of
the function as it invokes its callees. Since X64 functions do not modify the
value of RSP after the function prolog, any stack accesses performed by the rest
of the function are done relative to this position of the stack pointer. This
includes access to stack based parameters and local variables." - x64 Deep Dive

*/
int TestChildSP() {
	int p1 = 1, p2 = 2, p3 = 3, p4 = 4, p5 = 5, p6 = 6, p7 = 7, p8 = 8;
	int ret = ChildSPF1(p1, p2, p3, p4, p5, p6, p7, p8)
		+ ChildSPF2(p1, p2, p3, p4, p5, p6, p7, p8)
		+ ChildSPF3(p1, p2, p3, p4, p5, p6, p7, p8);
	return ret == 158942311 ? 1 : 0;
}


/*
3. Parameters are saved from the registers into memory.

# Win64 Release
	g_ParameterRetrieval_p7_retrieval = &p7;
mov         r10,qword ptr [p7]
mov         r11,rcx
	return p1 + p2 + p3 + p4 + p5 + p6 + p7 + p8;
mov         rcx,qword ptr [p8]
mov         qword ptr [g_ParameterRetrieval_p7_retrieval],r10 # save nonvolatile register
*/
int* g_ParameterRetrieval_p7_retrieval;
int ParameterRetrieval(int& p1, int& p2, int& p3, int& p4, int& p5, int& p6, int& p7, int& p8) {
	g_ParameterRetrieval_p7_retrieval = &p7;
	return p1 + p2 + p3 + p4 + p5 + p6 + p7 + p8;
}

/*
2. Parameters are loaded from non-volatile registers and those registers
are saved by the callee.

	int oldP7 = *g_ParameterRetrieval_p7_retrieval;
	return p1 + p2 + p3 + p4 + p5 + p6 + oldP7 + p8;
mov         eax,dword ptr [rdx]
mov         r10,rcx
add         eax,dword ptr [r8]
add         eax,dword ptr [r9]
mov         rdx,qword ptr [p5]
mov         rcx,qword ptr [g_ParameterRetrieval_p7_retrieval]
*/
int ParameterRetrieval2(int& p1, int& p2, int& p3, int& p4, int& p5, int& p6, int& p7, int& p8) {
	int oldP7 = *g_ParameterRetrieval_p7_retrieval;
	return p1 + p2 + p3 + p4 + p5 + p6 + oldP7 + p8;
}

/*
4. Parameters are saved into non-volatile registers and those registers
are saved by the callee.

# Win64 Release
int ParameterRetrieval3(int& p1, int& p2, int& p3, int& p4, int& p5, int& p6, int& p7, int& p8) {
mov         qword ptr [rsp+8],rbx
mov         qword ptr [rsp+10h],rsi
mov         qword ptr [rsp+18h],rdi
mov         qword ptr [rsp+20h],r14
push        r15 # saves what will be p7
	int oldP7 = p7;
	for (int i = 0; i < 7; ++i) {
		p7 += p1;
mov         r10d,dword ptr [rcx]
mov         r14,rcx
mov         r15,qword ptr [p7] # going to use p7
mov         rbx,rdx
mov         rdi,r9
mov         esi,dword ptr [r15] # Parameters are saved into non-volatile registers...
add         r10d,esi
...
	}
	int ret = p1 + p2 + p3 + p4 + p5 + p6 + p7 + p8;
...
	p7 = oldP7;
	return ret;
}
...
mov         dword ptr [r15],esi # ... and those registers are saved by the callee.
mov         rsi,qword ptr [rsp+18h]
pop         r15
ret
*/
int ParameterRetrieval3(int& p1, int& p2, int& p3, int& p4, int& p5, int& p6, int& p7, int& p8) {
	int oldP7 = p7;
	for (int i = 0; i < 7; ++i) {
		p7 += p1;
	}
	int ret = p1 + p2 + p3 + p4 + p5 + p6 + p7 + p8;
	p7 = oldP7;
	return ret;
}


/*
"(...) as execution progresses within the function body, the contents of the
parameter registers change and the initial parameter value gets overwritten. So,
to determine the value of these register based parameters at any point during
function execution, one needs to find out - where is the value of the parameter
being read from and where is the value of the parameter being written to?
Answers to these questions can be found by performing a sequence of steps in the
debugger which can be grouped as follows: Determine if the parameters are loaded
into the registers from memory. If so, the memory location can be examined to
determine the parameter values. Determine if the parameters are loaded from
non-volatile registers and if those registers are saved by the callee. If so,
the saved non-volatile register values can be examined to determine the
parameter values. Determine if the parameters are saved from the registers into
memory. If so, the memory location can be examined to determine the parameter
values. Determine if the parameters are saved into non-volatile registers and if
those registers are saved by the callee. If so, the saved non-volatile register
values can be examined to determine the parameter values." - x64 Deep Dive

# Win64 Release
1. Parameters are loaded into the registers from memory.
mov         dword ptr [rbp+30h],5 # p5 = 5
mov         dword ptr [rbp+28h],6 # p6 = 6
mov         dword ptr [rbp+20h],7 # p7 = 7
mov         dword ptr [rbp+18h],8 # p8 = 8
call        ChildSPF1
*/
int TestParameterRetrieval() {
	int p1 = 1, p2 = 2, p3 = 3, p4 = 4, p5 = 5, p6 = 6, p7 = 7, p8 = 8;
	int ret = ParameterRetrieval(p1, p2, p3, p4, p5, p6, p7, p8)
		+ ParameterRetrieval2(p1, p2, p3, p4, p5, p6, p7, p8)
		+ ParameterRetrieval3(p1, p2, p3, p4, p5, p6, p7, p8);
	return ret == 158942311 ? 1 : 0;
}


int main()
{
	int ret = 0;
	ret += TestFastCall();
	ret += TestTailCallElimination();
	ret += TestFramePointerOmission();
	ret += TestRSPIsTheSame();
	ret += TestHomingSpace();
	ret += TestHomingSpaceNonLeaf();
	ret += TestChildSP();
	ret += TestParameterRetrieval();
	return ret;
}

