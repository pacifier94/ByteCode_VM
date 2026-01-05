#include <iostream>
#include <vector>
#include <fstream>
#include <cstdint>

using std::vector;
using std::uint8_t;
using std::uint32_t;
using std::int32_t;
using std::cerr;
using std::cout;
using std::endl;

enum Opcode : uint8_t {
    PUSH  = 0x01,
    POP   = 0x02,
    DUP   = 0x03,

    ADD   = 0x10,
    SUB   = 0x11,
    MUL   = 0x12,
    DIV   = 0x13,
    CMP   = 0x14,

    JMP   = 0x20,
    JZ    = 0x21,
    JNZ   = 0x22,

    STORE = 0x30,
    LOAD  = 0x31,

    CALL  = 0x40,
    RET   = 0x41,

    PRINT = 0x50,
    HALT  = 0xFF
};

class VM {
    static constexpr uint32_t MEM_SIZE = 1024;

    vector<uint8_t> code;
    vector<int32_t> stack;
    vector<uint32_t> callStack;
    int32_t memory[MEM_SIZE]{};

    uint32_t pc = 0;
    bool running = true;

    void error(const char* msg) {
        cerr << "Runtime Error: " << msg << " at PC " << pc << endl;
        running = false;
    }

    int32_t pop() {
        if (stack.empty()) {
            error("Stack underflow");
            return 0;
        }
        int32_t v = stack.back();
        stack.pop_back();
        return v;
    }

    int32_t fetch32() {
        if (pc + 4 > code.size()) {
            error("Unexpected end of bytecode");
            return 0;
        }

        uint32_t v =
            (code[pc] << 24) |
            (code[pc + 1] << 16) |
            (code[pc + 2] << 8) |
            (code[pc + 3]);

        pc += 4;
        return static_cast<int32_t>(v);
    }

public:
    explicit VM(vector<uint8_t> bytecode) : code(std::move(bytecode)) {}

    void run() {
        while (running && pc < code.size()) {
            Opcode op = static_cast<Opcode>(code[pc++]);

            switch (op) {
                case PUSH:
                    stack.push_back(fetch32());
                    break;

                case POP:
                    pop();
                    break;

                case DUP:
                    if (stack.empty()) error("DUP on empty stack");
                    else stack.push_back(stack.back());
                    break;

                case ADD: stack.push_back(pop() + pop()); break;
                case SUB: {
                    int32_t b = pop(), a = pop();
                    stack.push_back(a - b);
                    break;
                }
                case MUL: stack.push_back(pop() * pop()); break;

                case DIV: {
                    int32_t b = pop(), a = pop();
                    if (b == 0) error("Division by zero");
                    else stack.push_back(a / b);
                    break;
                }

                case CMP: {
                    int32_t b = pop(), a = pop();
                    stack.push_back((a > b) - (a < b));
                    break;
                }

                case JMP:
                    pc = fetch32();
                    break;

                case JZ: {
                    uint32_t dest = fetch32();
                    if (pop() == 0) pc = dest;
                    break;
                }

                case JNZ: {
                    uint32_t dest = fetch32();
                    if (pop() != 0) pc = dest;
                    break;
                }

                case STORE: {
                    uint32_t addr = fetch32();
                    if (addr >= MEM_SIZE) error("Memory store OOB");
                    else memory[addr] = pop();
                    break;
                }

                case LOAD: {
                    uint32_t addr = fetch32();
                    if (addr >= MEM_SIZE) error("Memory load OOB");
                    else stack.push_back(memory[addr]);
                    break;
                }

                case CALL:
                    callStack.push_back(pc);
                    pc = fetch32();
                    break;

                case RET:
                    if (callStack.empty()) error("RET without CALL");
                    else {
                        pc = callStack.back();
                        callStack.pop_back();
                    }
                    break;

                case PRINT:
                    cout << "VM PRINT: " << pop() << endl;
                    break;

                case HALT:
                    running = false;
                    break;

                default:
                    error("Unknown opcode");
                    break;
            }
        }

        if (!stack.empty())
            cout << "Final Result: " << stack.back() << endl;
    }
};

int main(int argc, char** argv) {
    if (argc < 2) return 1;

    std::ifstream f(argv[1], std::ios::binary);
    if (!f) return 1;

    vector<uint8_t> bytecode(
        (std::istreambuf_iterator<char>(f)),
        std::istreambuf_iterator<char>()
    );

    VM vm(bytecode);
    vm.run();
    return 0;
}
