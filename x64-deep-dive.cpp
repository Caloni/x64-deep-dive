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
call        RSPIsTheSameCall1 (07FF791BC1262h)
	RSPIsTheSameCall4(p1, p2, p3, p4);
mov         r9d,dword ptr [p4]
...
call        RSPIsTheSameCall4 (07FF791BC1271h)
	RSPIsTheSameCall8(p1, p2, p3, p4, p5, p6, p7, p8);
mov         eax,dword ptr [p8]
mov         dword ptr [rsp+38h],eax
...
call        RSPIsTheSameCall8 (07FF791BC1276h)
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


int main()
{
	int ret = 0;
	ret += TestFastCall();
	ret += TestTailCallElimination();
	ret += TestFramePointerOmission();
	ret += TestRSPIsTheSame();
	return ret;
}

