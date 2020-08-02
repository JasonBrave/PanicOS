; program entry point (with global constructors)
;
; This file is part of PanicOS.
;
; PanicOS is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
;
; PanicOS is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with PanicOS.  If not, see <https://www.gnu.org/licenses/>.

global _start
extern main,exit,_DYNAMIC
extern __preinit_array_start,__preinit_array_end
extern __init_array_start,__init_array_end

_start:
	push ebp
	mov ebp,esp
	mov ebx,[ebp+20]			;interp
	test ebx,ebx
	jz .runmain
.runinterp:
	sub esp,8
	;esp+4 dynamic section of dynamic linker
	;esp entry point of dynamic linker
	mov edx,esp					;use edx to store old esp
	push edx					;entry point of interp
	add edx,4
	push edx					;dynamic section of interp
	push ebx					;interp
	push 0
	mov eax,28
	int 64
	add esp,16
	test eax,eax
	jz .loaderr
	mov ebx,[esp]
	push _DYNAMIC				;dynamic
	add ebx,0x40000000
	call ebx					;dl_main
	add esp,4
;call global constructors
.call_preinit:
	mov ebx,__preinit_array_start
.call_preinit_cont:
	cmp ebx,__preinit_array_end
	je .call_init
	call [ebx]
	add ebx,4
	jmp .call_preinit_cont
.call_init:
	mov ebx,__init_array_start
.call_init_cont:
	cmp ebx,__init_array_end
	je .runmain
	call [ebx]
	add ebx,4
	jmp .call_init_cont
;call main function
.runmain:
	mov edx,[ebp+16]			;char* envp[]
	mov esi,[ebp+12]			;char* argv[]
	mov edi,[ebp+8]				;int argc
	push edx
	push esi
	push edi
	call main					;int main()
	add esp,12
	push eax					;int status
	call exit					;exit
	jmp $
.loaderr:
	push -1
	push 0
	mov eax,7					;proc_exit
	int 64
	jmp $
