#include <iostream>
#include <vector>
#include <fstream>
#include <cstdint>
#include <chrono>

using namespace std;
using namespace std::chrono;

class VM {
public:
    vector<uint8_t> code;
    vector<int32_t> operandStack;
    vector<uint32_t> callStack;
    int32_t memory[1024]; 
    uint32_t pc = 0;
    bool running = true;
    bool debug = false;  // Debug flag to print extra information

    VM(vector<uint8_t> bytecode) : code(bytecode) {
        reset();
    }

    void reset() {
        operandStack.clear();
        callStack.clear();
        for(int i = 0; i < 1024; i++) memory[i] = 0;
        pc = 0;
        running = true;
    }

    int32_t fetchInt32() {
        if (pc + 4 > code.size()) return 0; // Safety check
        int32_t val = (static_cast<int32_t>(code[pc]) << 24) |
                      (static_cast<int32_t>(code[pc+1]) << 16) |
                      (static_cast<int32_t>(code[pc+2]) << 8) |
                      (static_cast<int32_t>(code[pc+3]));
        pc += 4;
        return val;
    }

    int32_t safe_pop() {
        if (operandStack.empty()) {
            cerr << "Runtime Error: Stack Underflow" << endl;
            running = false;
            return 0;
        }
        int32_t val = operandStack.back();
        operandStack.pop_back();
        return val;
    }

    void step() {
        uint8_t opcode = code[pc++];

        if (debug) {
            cout << "Executing PC: " << pc - 1 << " Opcode: " << (int)opcode << endl;
        }

        switch (opcode) {
            case 0x01: operandStack.push_back(fetchInt32()); break;   // PUSH
            case 0x02: safe_pop(); break;                               // POP
            case 0x03: { // DUP
    if (!operandStack.empty()) {
        operandStack.push_back(operandStack.back());
    } else {
        cerr << "Runtime Error: DUP on empty stack" << endl;
        running = false;
    }
    break;
}
            case 0x10: {                                                // ADD
                int32_t b = safe_pop(); 
                int32_t a = safe_pop(); 
                operandStack.push_back(a + b);
                break;
            }
            case 0x11: {                                                // SUB
                int32_t b = safe_pop(); 
                int32_t a = safe_pop(); 
                operandStack.push_back(a - b);
                break;
            }
            case 0x12: {                                                // MUL
                int32_t b = safe_pop(); 
                int32_t a = safe_pop(); 
                operandStack.push_back(a * b);
                break;
            }
            case 0x13: { // DIV
    int32_t b = safe_pop();
    int32_t a = safe_pop();
    if (b == 0) {
        cerr << "Runtime Error: Division by Zero" << endl;
        running = false;
        break;
    }
    operandStack.push_back(a / b);
    break;
}
case 0x14: { // CMP (Compare)
    int32_t b = safe_pop(); // Top value
    int32_t a = safe_pop(); // Second value
    if (a < b) operandStack.push_back(-1);
    else if (a > b) operandStack.push_back(1);
    else operandStack.push_back(0);
    break;
}

            case 0x20: {                                                // JMP
                uint32_t addr = fetchInt32();
                if (addr < code.size()) pc = addr;
                else {
                    cerr << "Runtime Error: Invalid Jump Address" << endl;
                    running = false;
                }
                break;
            }
case 0x21: { // JZ (Jump if Zero)
    uint32_t addr = fetchInt32();
    if (safe_pop() == 0) pc = addr;  // Jump if the value popped is 0
    break;
}

            case 0x22: {                                                // JNZ
                uint32_t addr = fetchInt32();
                if (safe_pop() != 0) pc = addr;
                break;
            }
            case 0x30: {                                                // STORE
                uint32_t idx = fetchInt32();
                if (idx < 1024) {
                    if (!operandStack.empty()) {
                        memory[idx] = safe_pop();
                    } else {
                        cerr << "Runtime Error: Stack Underflow during STORE" << endl;
                        running = false;
                    }
                } else {
                    cerr << "Runtime Error: Memory Access Out of Bounds (Store at " << idx << ")" << endl;
                    running = false;
                }
                break;
            }
            case 0x31: {                                                // LOAD
                uint32_t idx = fetchInt32();
                if (idx < 1024) {
                    operandStack.push_back(memory[idx]);
                } else {
                    cerr << "Runtime Error: Memory Access Out of Bounds (Load at " << idx << ")" << endl;
                    running = false;
                }
                break;
            }
            case 0x40: {                                                // CALL
                uint32_t addr = fetchInt32();
                if (addr < code.size()) {
                    callStack.push_back(pc);
                    pc = addr;
                } else {
                    cerr << "Runtime Error: Invalid CALL Address" << endl;
                    running = false;
                }
                break;
            }
            case 0x41: {                                                // RET
                if (!callStack.empty()) {
                    pc = callStack.back();
                    callStack.pop_back();
                } else {
                    cerr << "Runtime Error: RET without CALL" << endl;
                    running = false;
                }
                break;
            }
            case 0xFF: running = false; break;                           // HALT
            default: {
                cerr << "Runtime Error: Unknown Opcode " << (int)opcode << endl;
                running = false;
                break;
            }
        }
    }

    void run() {
        while (running && pc < code.size()) {
            step();
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: ./vm <file.bin> [iterations]" << endl;
        return 1;
    }

    ifstream file(argv[1], ios::binary);
    if (!file) { cerr << "Could not open file." << endl; return 1; }
    vector<uint8_t> buffer((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    
    VM vm(buffer);

    if (argc == 3) {
        // BENCHMARK MODE
        int iterations = stoi(argv[2]);
        cout << "Benchmarking " << iterations << " iterations..." << endl;
        
        auto start = high_resolution_clock::now();
        for (int i = 0; i < iterations; i++) {
            vm.reset();
            vm.run();
        }
        auto end = high_resolution_clock::now();
        
        duration<double> diff = end - start;
        cout << "--- Results ---" << endl;
        cout << "Total time: " << diff.count() << " s" << endl;
        cout << "Avg time:   " << (diff.count() / iterations) * 1e6 << " microseconds" << endl;
    } else {
        // NORMAL MODE
        vm.run();
        if (!vm.operandStack.empty()) {
            cout << "Final Result: " << vm.safe_pop() << endl;
        }
    }

    return 0;
}
