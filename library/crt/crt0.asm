; program entry point
;
; This file is part of HoleOS.
;
; HoleOS is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
;
; HoleOS is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with HoleOS.  If not, see <https://www.gnu.org/licenses/>.

global _start
extern main,proc_exit

_start:
	push ebp
	mov ebp,esp
	mov edx,[ebp+16]			;char* envp[]
	mov esi,[ebp+12]			;char* argv[]
	mov edi,[ebp+8]				;int argc
	push edx
	push esi
	push edi
	call main					;int main()
	add esp,12
	call proc_exit					;exit
	jmp $
