#include <iostream>
#include <vector>
#include <fstream>
#include <cstdint>

using namespace std;

class VM {
    vector<uint8_t> code;
    vector<int32_t> stack;
    vector<uint32_t> callStack;
    int32_t memory[1024];
    uint32_t pc = 0;
    bool running = true;

    // Helper to safely pop values
    int32_t pop() {
        if (stack.empty()) {
            cerr << "Runtime Error: Stack Underflow at PC " << (pc - 1) << endl;
            running = false;
            return 0;
        }
        int32_t val = stack.back();
        stack.pop_back();
        return val;
    }

public:
    VM(vector<uint8_t> b) : code(b) { 
        for(int i = 0; i < 1024; i++) memory[i] = 0; 
    }

    int32_t fetch32() {
        if (pc + 4 > code.size()) {
            cerr << "Runtime Error: Unexpected End of Bytecode" << endl;
            running = false;
            return 0;
        }
        uint32_t v = (uint32_t)code[pc] << 24 | (uint32_t)code[pc+1] << 16 | 
                     (uint32_t)code[pc+2] << 8 | (uint32_t)code[pc+3];
        pc += 4;
        return (int32_t)v;
    }

    void run() {
        while (running && pc < code.size()) {
            uint8_t op = code[pc++];
            switch (op) {
                case 0x01: // PUSH
                    stack.push_back(fetch32()); 
                    break;
                
                case 0x02: // POP
                    pop(); 
                    break;
                
                case 0x03: // DUP
                    if(!stack.empty()) stack.push_back(stack.back());
                    else { cerr << "Error: DUP on empty stack" << endl; running = false; }
                    break;

                case 0x10: { // ADD
                    int32_t b = pop(); int32_t a = pop();
                    stack.push_back(a + b); break; 
                }

                case 0x11: { // SUB
                    int32_t b = pop(); int32_t a = pop();
                    stack.push_back(a - b); break; 
                }

                case 0x12: { // MUL
                    int32_t b = pop(); int32_t a = pop();
                    stack.push_back(a * b); break; 
                }

                case 0x13: { // DIV
                    int32_t b = pop(); int32_t a = pop();
                    if (b == 0) {
                        cerr << "Runtime Error: Division by Zero" << endl;
                        running = false;
                    } else {
                        stack.push_back(a / b);
                    }
                    break;
                }

                case 0x14: { // CMP (Compare)
    int32_t b = pop(); // Top value
    int32_t a = pop(); // Second value
    if (a < b) stack.push_back(-1);
    else if (a > b) stack.push_back(1);
    else stack.push_back(0);
    break;
}

case 0x20: { // JMP (Unconditional Jump)
    uint32_t dest = fetch32();
    pc = dest;
    break;
}

case 0x22: { // JNZ (Jump if Not Zero)
    uint32_t dest = fetch32();
    if (pop() != 0) pc = dest;
    break;
}



                case 0x21: { // JZ (Jump if Zero)
                    uint32_t dest = fetch32();
                    if (pop() == 0) pc = dest;
                    break;
                }

                case 0x30: { // STORE
                    uint32_t addr = (uint32_t)fetch32();
                    int32_t val = pop();
                    if (addr < 1024) memory[addr] = val;
                    else { cerr << "Error: Memory Out of Bounds (Store)" << endl; running = false; }
                    break;
                }

                case 0x31: { // LOAD
                    uint32_t addr = (uint32_t)fetch32();
                    if (addr < 1024) stack.push_back(memory[addr]);
                    else { cerr << "Error: Memory Out of Bounds (Load)" << endl; running = false; }
                    break;
                }

                case 0x40: { // CALL
                    uint32_t dest = fetch32();
                    callStack.push_back(pc);
                    pc = dest;
                    break;
                }

                case 0x41: { // RET
                    if (!callStack.empty()) {
                        pc = callStack.back();
                        callStack.pop_back();
                    } else {
                        cerr << "Error: RET without CALL" << endl;
                        running = false;
                    }
                    break;
                }

                case 0x50: // PRINT
                    cout << "VM PRINT: " << pop() << endl;
                    break;

                case 0xFF: // HALT
                    running = false;
                    break;

                default:
                    cerr << "Unknown Opcode: " << hex << (int)op << endl;
                    running = false;
                    break;
            }
        }
        if (!stack.empty()) cout << "Final Result: " << stack.back() << endl;
    }
};

int main(int argc, char** argv) {
    if (argc < 2) return 1;
    ifstream f(argv[1], ios::binary);
    if (!f) return 1;
    vector<uint8_t> b((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());
    VM vm(b);
    vm.run();
    return 0;
}
