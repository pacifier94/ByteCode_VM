#include <iostream>
#include <vector>
#include <fstream>
#include <cstdint>

using namespace std;

class VM {
    vector<uint8_t> code;
    vector<int32_t> operandStack;
    vector<uint32_t> callStack;
    int32_t memory[1024];
    uint32_t pc = 0;
    bool running = true;

    // Stack safety
    bool ensureStack(size_t n) {
        if (operandStack.size() < n) {
            cerr << "Runtime Error: Stack underflow at PC " << pc - 1 << endl;
            running = false;
            return false;
        }
        return true;
    }

    // Jump safety
    bool ensureAddr(uint32_t addr) {
        if (addr >= code.size()) {
            cerr << "Runtime Error: Invalid jump address " << addr << endl;
            running = false;
            return false;
        }
        return true;
    }

public:
    VM(const vector<uint8_t>& bytecode) : code(bytecode) {
        reset();
    }

    void reset() {
        operandStack.clear();
        callStack.clear();
        pc = 0;
        running = true;
        for (int &m : memory) m = 0;
    }

    // Safe 32-bit fetch
    int32_t fetchInt32() {
        if (pc + 4 > code.size()) {
            cerr << "Runtime Error: Bytecode read out of bounds\n";
            running = false;
            return 0;
        }
        int32_t val =
            (code[pc] << 24) |
            (code[pc + 1] << 16) |
            (code[pc + 2] << 8) |
            (code[pc + 3]);
        pc += 4;
        return val;
    }

    void step() {
        if (pc >= code.size()) {
            cerr << "Runtime Error: PC out of bounds\n";
            running = false;
            return;
        }

        cout << "PC=" << pc << " Stack=[";
        for(auto v: operandStack) cout << v << " ";
        cout << "]" << endl;

        uint8_t opcode = code[pc++];

        switch (opcode) {
            case 0x01: operandStack.push_back(fetchInt32()); break; // PUSH
            case 0x02: if (ensureStack(1)) operandStack.pop_back(); break; // POP
            case 0x03: if (ensureStack(1)) operandStack.push_back(operandStack.back()); break; // DUP

            case 0x10: // ADD
                if (ensureStack(2)) {
                    int32_t b = operandStack.back(); operandStack.pop_back();
                    int32_t a = operandStack.back(); operandStack.pop_back();
                    operandStack.push_back(a + b);
                }
                break;

            case 0x11: // SUB
                if (ensureStack(2)) {
                    int32_t b = operandStack.back(); operandStack.pop_back();
                    int32_t a = operandStack.back(); operandStack.pop_back();
                    operandStack.push_back(a - b);
                }
                break;

            case 0x12: // MUL
                if (ensureStack(2)) {
                    int32_t b = operandStack.back(); operandStack.pop_back();
                    int32_t a = operandStack.back(); operandStack.pop_back();
                    operandStack.push_back(a * b);
                }
                break;

            case 0x13: // DIV
                if (ensureStack(2)) {
                    int32_t b = operandStack.back(); operandStack.pop_back();
                    int32_t a = operandStack.back(); operandStack.pop_back();
                    if (b == 0) {
                        cerr << "Runtime Error: Division by zero\n";
                        running = false;
                        break;
                    }
                    operandStack.push_back(a / b);
                }
                break;

            case 0x14: // CMP
                if (ensureStack(2)) {
                    int32_t b = operandStack.back(); operandStack.pop_back();
                    int32_t a = operandStack.back(); operandStack.pop_back();
                    operandStack.push_back(a < b ? 1 : 0);
                }
                break;

            case 0x20: { // JMP
                uint32_t addr = fetchInt32();
                if (ensureAddr(addr)) pc = addr;
                break;
            }

            case 0x21: { // JZ
                uint32_t addr = fetchInt32();
                if (ensureStack(1)) {
                    int32_t v = operandStack.back();
                    operandStack.pop_back();
                    if (v == 0 && ensureAddr(addr)) pc = addr;
                }
                break;
            }

            case 0x22: { // JNZ
                uint32_t addr = fetchInt32();
                if (ensureStack(1)) {
                    int32_t v = operandStack.back();
                    operandStack.pop_back();
                    if (v != 0 && ensureAddr(addr)) pc = addr;
                }
                break;
            }

            case 0x30: { // STORE
                uint32_t idx = fetchInt32();
                if (!ensureStack(1)) break;
                if (idx >= 1024) {
                    cerr << "Runtime Error: Memory store out of bounds\n";
                    running = false;
                    break;
                }
                memory[idx] = operandStack.back();
                operandStack.pop_back();
                break;
            }

            case 0x31: { // LOAD
                uint32_t idx = fetchInt32();
                if (idx >= 1024) {
                    cerr << "Runtime Error: Memory load out of bounds\n";
                    running = false;
                    break;
                }
                operandStack.push_back(memory[idx]);
                break;
            }

            case 0x40: { // CALL
                uint32_t addr = fetchInt32();
                if (ensureAddr(addr)) {
                    callStack.push_back(pc);
                    pc = addr;
                }
                break;
            }

            case 0x41: { // RET
                if (callStack.empty()) {
                    cerr << "Runtime Error: Call stack underflow\n";
                    running = false;
                    break;
                }
                pc = callStack.back();
                callStack.pop_back();
                break;
            }

            case 0xFF: // HALT
                running = false;
                break;

            default:
                cerr << "Runtime Error: Invalid opcode " << (int)opcode << endl;
                running = false;
        }
    }

    void run() {
        while (running) step();
    }

    int32_t getResult() const {
        return operandStack.empty() ? 0 : operandStack.back();
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: ./vm program.bin\n";
        return 1;
    }

    ifstream file(argv[1], ios::binary);
    if (!file) {
        cerr << "Error: Could not open bytecode file\n";
        return 1;
    }

    vector<uint8_t> buffer((istreambuf_iterator<char>(file)),
                             istreambuf_iterator<char>());

    VM vm(buffer);
    vm.run();

    cout << "Final Result: " << vm.getResult() << endl;
    return 0;
}
