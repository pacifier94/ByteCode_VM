; Calculate Fibonacci(10)
; F(0)=0, F(1)=1, F(n) = F(n-1) + F(n-2)

PUSH 10     ; The Nth number we want
STORE 0     ; Mem[0] = N
PUSH 0      ; F(n-2)
STORE 1     ; Mem[1] = 0
PUSH 1      ; F(n-1)
STORE 2     ; Mem[2] = 1

LOOP:
LOAD 0      ; Check if N == 0
PUSH 1
SUB
DUP
STORE 0     ; N = N - 1
JZ END      ; If N was 1, we are done

LOAD 1      ; a = Mem[1]
LOAD 2      ; b = Mem[2]
ADD         ; result = a + b
STORE 3     ; temp_result = result

LOAD 2      ; Shift values: Mem[1] = Mem[2]
STORE 1
LOAD 3      ; Shift values: Mem[2] = temp_result
STORE 2

JMP LOOP

END:
LOAD 2      ; Push final Fibonacci result to stack
HALT