PUSH 50
PUSH 20
CMP         ; Stack has 1 (True/Greater)
JZ EQUAL    ; If 0, jump to EQUAL. It's 1, so we continue.

; This is the "NOT EQUAL" branch
PUSH 50     ; Let's push 50 so we see it as the result
HALT

EQUAL:
PUSH 20     ; This part won't run in this case
HALT