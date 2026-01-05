PUSH 4      ; [Addr 0-4]  Opcode 0x01 + 4-byte '4'
CALL 11     ; [Addr 5-9]  Opcode 0x40 + 4-byte '11'
HALT        ; [Addr 10]   Opcode 0xFF (PC returns here after function)

; --- Square Function Starts at Addr 11 ---
DUP         ; [Addr 11]   Opcode 0x03
MUL         ; [Addr 12]   Opcode 0x12
RET         ; [Addr 13]   Opcode 0x41