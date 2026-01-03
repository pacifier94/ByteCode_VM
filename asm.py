import struct
import sys

# Opcode table
OPCODES = {
    'PUSH': 0x01, 'POP': 0x02, 'DUP': 0x03, 'HALT': 0xFF,
    'ADD': 0x10, 'SUB': 0x11, 'MUL': 0x12, 'DIV': 0x13, 'CMP': 0x14,
    'JMP': 0x20, 'JZ': 0x21, 'JNZ': 0x22,
    'STORE': 0x30, 'LOAD': 0x31, 'CALL': 0x40, 'RET': 0x41
}

def assemble(input_file, output_file):
    labels = {}
    instructions = []

    # --- Pass 1: Collect labels and addresses ---
    current_addr = 0
    with open(input_file, 'r') as f:
        for line_no, line in enumerate(f, 1):
            line = line.split(';')[0].strip()
            if not line:
                continue

            if line.endswith(':'):
                label = line[:-1].strip()
                if label in labels:
                    print(f"Error: Duplicate label '{label}' at line {line_no}")
                    return
                labels[label] = current_addr
            else:
                instructions.append((line_no, line))
                parts = line.split()
                current_addr += 1 # opcode
                if len(parts) > 1: current_addr += 4 # 32-bit argument

    # --- Pass 2: Generate bytecode ---
    bytecode = bytearray()
    for line_no, line in instructions:
        parts = line.split()
        mnemonic = parts[0].upper()

        if mnemonic not in OPCODES:
            print(f"Error: Unknown instruction '{mnemonic}' at line {line_no}")
            return

        # Opcode
        bytecode.append(OPCODES[mnemonic])

        # Argument (optional)
        if len(parts) > 1:
            arg = parts[1].rstrip(',')  # remove trailing commas
            try:
                # Label or integer
                if arg in labels:
                    val = labels[arg]
                else:
                    val = int(arg)
            except ValueError:
                print(f"Error: Invalid argument '{arg}' at line {line_no}")
                return
            # Pack as signed 32-bit integer (matches VM fetchInt32)
            bytecode.extend(struct.pack('>i', val))

    # Write output
    with open(output_file, 'wb') as f:
        f.write(bytecode)
    print(f"Assembled successfully. Labels found: {list(labels.keys())}")

# --- Main ---
if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python asm.py input.asm [output.bin]")
        sys.exit(1)
    output_file = sys.argv[2] if len(sys.argv) > 2 else "program.bin"
    assemble(sys.argv[1], output_file)
