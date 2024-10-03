#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <iomanip>
#include <algorithm>

using namespace std;

// Memory and Register Constants
const int no_of_registers = 32;
const int DATA_SECTION_START = 0x10000;
const int memory_size = 400000;  

// Register and Memory Arrays
int64_t registers[no_of_registers];       // Register array
int memory[memory_size];           // Memory array for simulation

// Program Counter
int PC = 0;                         // Program Counter (PC)
vector<string> instructions;        // Stores the loaded instructions
vector<int> breakpoints;

// Register Name Map
unordered_map<string, int> regNameMap = {
    {"x0", 0}, {"x1", 1}, {"x2", 2}, {"x3", 3}, {"x4", 4}, {"x5", 5},
    {"x6", 6}, {"x7", 7}, {"x8", 8}, {"x9", 9}, {"x10", 10}, {"x11", 11},
    {"x12", 12}, {"x13", 13}, {"x14", 14}, {"x15", 15}, {"x16", 16}, {"x17", 17},
    {"x18", 18}, {"x19", 19}, {"x20", 20}, {"x21", 21}, {"x22", 22}, {"x23", 23},
    {"x24", 24}, {"x25", 25}, {"x26", 26}, {"x27", 27}, {"x28", 28}, {"x29", 29},
    {"x30", 30}, {"x31", 31},  {"zero", 0}, {"ra", 1}, {"sp", 2}, {"gp", 3}, {"tp", 4}, {"t0", 5}, {"t1", 6}, {"t2", 7},
    {"s0", 8}, {"s1", 9}, {"a0", 10}, {"a1", 11}, {"a2", 12}, {"a3", 13}, {"a4", 14}, {"a5", 15},
    {"a6", 16}, {"a7", 17}, {"s2", 18}, {"s3", 19}, {"s4", 20}, {"s5", 21}, {"s6", 22}, {"s7", 23},
    {"s8", 24}, {"s9", 25}, {"s10", 26}, {"s11", 27}, {"t3", 28}, {"t4", 29}, {"t5", 30}, {"t6", 31}
};
struct Label {
    string name;
    int address;
};

vector<Label> labelList;

void reset() {
    fill(begin(registers), end(registers), 0);
    fill(begin(memory), end(memory), 0);
    PC = 0;
}

void MapLabels(ifstream &inputFile, vector<Label> &labelList) {
    string line;
    int instructionIndex = 0;
    int address = DATA_SECTION_START;

    while (getline(inputFile, line)) {
        if (line.find(".data") != string::npos) {
            continue;
        }
        if (line.find(".text") != string::npos) {
            continue;
        }
        size_t labelPos = line.find(':');
        if (labelPos != string::npos) {
            string label = line.substr(0, labelPos);
            label = label.substr(0, label.find_last_not_of(" \t\n\r") + 1);  // Trim

            if (!label.empty()) {
                // Check if label already exists
                auto labelExists = find_if(labelList.begin(), labelList.end(), [&](const Label &l) {
                    return l.name == label;
                });

                if (labelExists != labelList.end()) {
                    cerr << "Error: Label '" << label << "' is repeated.\n";
                    exit(1);
                }

                labelList.push_back({label, instructionIndex});
            }

            // Handle the case where an instruction follows the label on the same line
            line = line.substr(labelPos + 1);  // Remove label part
        }

        // Now process the rest of the line (which is the instruction part)
        stringstream ss(line);
        string word;
        ss >> word;

        
        if (word == ".dword") {
            string value;
            while (ss >> value) {
                if(*(value.end()-1) == ',')
                {
                    value = value.substr(0,value.length()-1);
                }
                int value_int = stoi(value);
                for (int i = 0; i < 8; ++i) {
                    memory[address + i] = value_int & 0xFF;
                    value_int >>= 8;
                }
                address += 8;  // Move to the next 8-byte block
            }
        }
        if (word == ".word") {
            string value;
            while (ss >> value) {
                if(*(value.end()-1) == ',')
                {
                    value = value.substr(0,value.length()-1);
                }
                int value_int = stoi(value);
                for (int i = 0; i < 4; ++i) {
                    memory[address + i] = value_int & 0xFF;
                    value_int >>= 8;
                }
                address += 4;  // Move to the next 8-byte block
            }
        }
        if (word == ".half") {
            string value;
            while (ss >> value) {
                if(*(value.end()-1) == ',')
                {
                    value = value.substr(0,value.length()-1);
                }
                int value_int = stoi(value);
                for (int i = 0; i < 2; ++i) {
                    memory[address + i] = value_int & 0xFF;
                    value_int >>= 8;
                }
                address += 2;  // Move to the next 8-byte block
            }
        }
        if (word == ".byte") {
            string value;
            while (ss >> value) {
                if(*(value.end()-1) == ',')
                {
                    value = value.substr(0,value.length()-1);
                }
                int value_int = stoi(value);
                for (int i = 0; i < 1; ++i) {
                    memory[address + i] = value_int & 0xFF;
                    value_int >>= 8;
                }
                address += 1;  // Move to the next 8-byte block
            }
        }
        if (!word.empty()) {
            if( word == ".dword" || word == ".word" || word == ".half" || word == ".byte" ){
                continue;
            }
            instructions.push_back(line);
            instructionIndex++;
        }
    }

    inputFile.clear();
    inputFile.seekg(0, ios::beg);
}

Label FindLabel(const vector<Label> &labelList, const string &label) {
    for (const auto &lbl : labelList) {
        if (lbl.name == label) {
            return lbl;
        }
    }
}

bool loadInstructions(const string &filename) {
    ifstream infile(filename);
    if (!infile.is_open()) {
        cout << "Error: Could not open file " << filename << endl;
        return false;
    }

    reset();
    instructions.clear();
    labelList.clear();
    MapLabels(infile, labelList);
    
    infile.close();
    return true;
}

void printRegisters() {
    cout << "Registers:\n";
    for (int i = 0; i < no_of_registers; ++i) {
        cout << "x" << dec<< i << " = 0x" << hex << setw(16) << setfill('0') << registers[i] << "\n";
    }
    cout << dec;
}

void executeInstruction(const string &instruction) {
    stringstream ss(instruction);
    string opcode, rd, rs1, rs2, imm;
    ss >> opcode;
    bool isBranch = false;
    if (opcode == "add") {
        ss >> rd >> rs1 >> rs2;
        rd = rd.substr(0, rd.length() - 1);
        rs1 = rs1.substr(0, rs1.length() - 1);
        registers[regNameMap[rd]] = registers[regNameMap[rs1]] + registers[regNameMap[rs2]];
    }
    else if (opcode == "sub") {
        ss >> rd >> rs1 >> rs2;
        rd = rd.substr(0, rd.length() - 1);
        rs1 = rs1.substr(0, rs1.length() - 1);
        registers[regNameMap[rd]] = registers[regNameMap[rs1]] - registers[regNameMap[rs2]];
    }
    else if (opcode == "and") {
        ss >> rd >> rs1 >> rs2;
        rd = rd.substr(0, rd.length() - 1);
        rs1 = rs1.substr(0, rs1.length() - 1);
        registers[regNameMap[rd]] = registers[regNameMap[rs1]] & registers[regNameMap[rs2]];
    }
    else if (opcode == "or") {
        ss >> rd >> rs1 >> rs2;
        rd = rd.substr(0, rd.length() - 1);
        rs1 = rs1.substr(0, rs1.length() - 1);
        registers[regNameMap[rd]] = registers[regNameMap[rs1]] | registers[regNameMap[rs2]];
    }
    else if (opcode == "xor") {
        ss >> rd >> rs1 >> rs2;
        rd = rd.substr(0, rd.length() - 1);
        rs1 = rs1.substr(0, rs1.length() - 1);
        registers[regNameMap[rd]] = registers[regNameMap[rs1]] ^ registers[regNameMap[rs2]];
    }
    else if (opcode == "sll") {
        ss >> rd >> rs1 >> rs2;
        rd = rd.substr(0, rd.length() - 1);
        rs1 = rs1.substr(0, rs1.length() - 1);
        registers[regNameMap[rd]] = registers[regNameMap[rs1]] << (registers[regNameMap[rs2]] & 0x1F);
    }
    else if (opcode == "srl") {
        ss >> rd >> rs1 >> rs2;
        rd = rd.substr(0, rd.length() - 1);
        rs1 = rs1.substr(0, rs1.length() - 1);
        registers[regNameMap[rd]] = (unsigned int)registers[regNameMap[rs1]] >> (registers[regNameMap[rs2]] & 0x1F);
    }
    else if (opcode == "sra") {
        ss >> rd >> rs1 >> rs2;
        rd = rd.substr(0, rd.length() - 1);
        rs1 = rs1.substr(0, rs1.length() - 1);
        registers[regNameMap[rd]] = registers[regNameMap[rs1]] >> (registers[regNameMap[rs2]] & 0x1F);
    }
    else if (opcode == "slt") {
        ss >> rd >> rs1 >> rs2;
        rd = rd.substr(0, rd.length() - 1);
        rs1 = rs1.substr(0, rs1.length() - 1);
        registers[regNameMap[rd]] = (registers[regNameMap[rs1]] < registers[regNameMap[rs2]]) ? 1 : 0;
    }
    else if (opcode == "sltu") {
        ss >> rd >> rs1 >> rs2;
        rd = rd.substr(0, rd.length() - 1);
        rs1 = rs1.substr(0, rs1.length() - 1);
        registers[regNameMap[rd]] = ((unsigned int)registers[regNameMap[rs1]] < (unsigned int)registers[regNameMap[rs2]]) ? 1 : 0;
    }
    else if (opcode == "addi") {
        ss >> rd >> rs1 >> imm;
        rd = rd.substr(0, rd.length() - 1); 
        rs1 = rs1.substr(0, rs1.length() - 1);
        registers[regNameMap[rd]] = registers[regNameMap[rs1]] + stoi(imm);
    }
    else if (opcode == "andi") {
        ss >> rd >> rs1 >> imm;
        rd = rd.substr(0, rd.length() - 1); 
        rs1 = rs1.substr(0, rs1.length() - 1);
        registers[regNameMap[rd]] = registers[regNameMap[rs1]] & stoi(imm);
    }
    else if (opcode == "ori") {
        ss >> rd >> rs1 >> imm;
        rd = rd.substr(0, rd.length() - 1); 
        rs1 = rs1.substr(0, rs1.length() - 1);
        registers[regNameMap[rd]] = registers[regNameMap[rs1]] | stoi(imm);
    }
    else if (opcode == "xori") {
        ss >> rd >> rs1 >> imm;
        rd = rd.substr(0, rd.length() - 1); 
        rs1 = rs1.substr(0, rs1.length() - 1);
        registers[regNameMap[rd]] = registers[regNameMap[rs1]] ^ stoi(imm);
    }
    else if (opcode == "slli") {
        ss >> rd >> rs1 >> imm;
        rd = rd.substr(0, rd.length() - 1); 
        rs1 = rs1.substr(0, rs1.length() - 1);
        registers[regNameMap[rd]] = registers[regNameMap[rs1]] << (stoi(imm) & 0x1F);
    }
    else if (opcode == "srli") {
        ss >> rd >> rs1 >> imm;
        rd = rd.substr(0, rd.length() - 1); 
        rs1 = rs1.substr(0, rs1.length() - 1);
        registers[regNameMap[rd]] = (unsigned int)registers[regNameMap[rs1]] >> (stoi(imm) & 0x1F);
    }
    else if (opcode == "srai") {
        ss >> rd >> rs1 >> imm;
        rd = rd.substr(0, rd.length() - 1); 
        rs1 = rs1.substr(0, rs1.length() - 1);
        registers[regNameMap[rd]] = registers[regNameMap[rs1]] >> (stoi(imm) & 0x1F);
    }
   else if (opcode == "ld") {
    ss >> rd >> imm >> rs1;
    rd = rd.substr(0, rd.length() - 1);  // Remove trailing comma
    int bracket1 = imm.find('(');
    int bracket2 = imm.find(')');
    rs1 = imm.substr(bracket1 + 1, bracket2-bracket1-1);
    imm = imm.substr(0, bracket1 );

    int address = registers[regNameMap[rs1]] + stoi(imm);
    int64_t value = 0;

    // Load 8 bytes individually and construct 64-bit value
    for (int i = 0; i < 8; ++i) {
        value |= ((int64_t)(uint8_t)memory[address + i] << (8 * i));
    }
    registers[regNameMap[rd]] = value;
}

   else if (opcode == "lw") {
    ss >> rd >> imm >> rs1;
    rd = rd.substr(0, rd.length() - 1);  // Remove trailing comma
    int bracket1 = imm.find('(');
    int bracket2 = imm.find(')');
    rs1 = imm.substr(bracket1 + 1, bracket2-bracket1-1);
    imm = imm.substr(0, bracket1 );

    int address = registers[regNameMap[rs1]] + stoi(imm);
    int64_t value = 0;

    // Load 8 bytes individually and construct 64-bit value
    for (int i = 0; i < 4; ++i) {
        value |= ((int64_t)(uint8_t)memory[address + i] << (8 * i));
    }
    registers[regNameMap[rd]] = value;
}

    else if (opcode == "lh") {
    ss >> rd >> imm >> rs1;
    rd = rd.substr(0, rd.length() - 1);  // Remove trailing comma
    int bracket1 = imm.find('(');
    int bracket2 = imm.find(')');
    rs1 = imm.substr(bracket1 + 1, bracket2-bracket1-1);
    imm = imm.substr(0, bracket1 );

    int address = registers[regNameMap[rs1]] + stoi(imm);
    int64_t value = 0;

    // Load 8 bytes individually and construct 64-bit value
    for (int i = 0; i < 2; ++i) {
        value |= ((int64_t)(uint8_t)memory[address + i] << (8 * i));
    }
    registers[regNameMap[rd]] = value;
}

    else if (opcode == "lb") {
    ss >> rd >> imm >> rs1;
    rd = rd.substr(0, rd.length() - 1);  // Remove trailing comma
    int bracket1 = imm.find('(');
    int bracket2 = imm.find(')');
    rs1 = imm.substr(bracket1 + 1, bracket2-bracket1-1);
    imm = imm.substr(0, bracket1 );

    int address = registers[regNameMap[rs1]] + stoi(imm);
    int64_t value = 0;

    // Load 8 bytes individually and construct 64-bit value
    for (int i = 0; i < 1; ++i) {
        value |= ((int64_t)(uint8_t)memory[address + i] << (8 * i));
    }
    registers[regNameMap[rd]] = value;
}

   else if (opcode == "lwu") {
    ss >> rd >> imm >> rs1;
    rd = rd.substr(0, rd.length() - 1);  // Remove trailing comma
    int bracket1 = imm.find('(');
    int bracket2 = imm.find(')');
    rs1 = imm.substr(bracket1 + 1, bracket2-bracket1-1);
    imm = imm.substr(0, bracket1 );

    int address = registers[regNameMap[rs1]] + stoi(imm);
    uint64_t value = 0;

    // Load 8 bytes individually and construct 64-bit value
    for (int i = 0; i < 4; ++i) {
        value |= ((uint64_t)(uint8_t)memory[address + i] << (8 * i));
    }
    registers[regNameMap[rd]] = value;
}

    else if (opcode == "lhu") {
        ss >> rd >> imm >> rs1;
        rd = rd.substr(0, rd.length() - 1);  // Remove trailing comma
        imm = imm.substr(0, imm.length() - 1); // Remove trailing comma
        int address = registers[regNameMap[rs1]] + stoi(imm);
        uint16_t value = 0;
        for (int i = 0; i < 2; ++i) {
            value |= ((uint16_t)memory[address + i] << (8 * i));
        }
        registers[regNameMap[rd]] = value;
    }

    else if (opcode == "lbu") {
    ss >> rd >> imm >> rs1;
    rd = rd.substr(0, rd.length() - 1);  // Remove trailing comma
    int bracket1 = imm.find('(');
    int bracket2 = imm.find(')');
    rs1 = imm.substr(bracket1 + 1, bracket2-bracket1-1);
    imm = imm.substr(0, bracket1 );

    int address = registers[regNameMap[rs1]] + stoi(imm);
    uint64_t value = 0;

    // Load 8 bytes individually and construct 64-bit value
    for (int i = 0; i < 1; ++i) {
        value |= ((uint64_t)(uint8_t)memory[address + i] << (8 * i));
    }
    registers[regNameMap[rd]] = value;
}

 else if (opcode == "sd") {
    ss >> rs2 >> imm >> rs1;
    rs2 = rs2.substr(0, rs2.length() - 1);  // Remove comma
    int bracket1 = imm.find('(');
    int bracket2 = imm.find(')');
    rs1 = imm.substr(bracket1 + 1, bracket2-bracket1-1);
    imm = imm.substr(0, bracket1 );  // Remove comma

    int address = registers[regNameMap[rs1]] + stoi(imm);
    int64_t value = registers[regNameMap[rs2]];

    // Store 64-bit value into 8 separate bytes
    for (int i = 0; i < 8; ++i) {
        memory[address + i] = (uint8_t)((value >> (8 * i)) & 0xFF);
    }
}

    else if (opcode == "sw") {
    ss >> rs2 >> imm >> rs1;
    rs2 = rs2.substr(0, rs2.length() - 1);  // Remove comma
    int bracket1 = imm.find('(');
    int bracket2 = imm.find(')');
    rs1 = imm.substr(bracket1 + 1, bracket2-bracket1-1);
    imm = imm.substr(0, bracket1 );  // Remove comma

    int address = registers[regNameMap[rs1]] + stoi(imm);
    int64_t value = registers[regNameMap[rs2]];

    // Store 64-bit value into 8 separate bytes
    for (int i = 0; i < 4; ++i) {
        memory[address + i] = (uint8_t)((value >> (8 * i)) & 0xFF);
    }
}
    else if (opcode == "sh") {
    ss >> rs2 >> imm >> rs1;
    rs2 = rs2.substr(0, rs2.length() - 1);  // Remove comma
    int bracket1 = imm.find('(');
    int bracket2 = imm.find(')');
    rs1 = imm.substr(bracket1 + 1, bracket2-bracket1-1);
    imm = imm.substr(0, bracket1 );  // Remove comma

    int address = registers[regNameMap[rs1]] + stoi(imm);
    int64_t value = registers[regNameMap[rs2]];

    // Store 64-bit value into 8 separate bytes
    for (int i = 0; i < 2; ++i) {
        memory[address + i] = (uint8_t)((value >> (8 * i)) & 0xFF);
    }
}
    else if (opcode == "sb") {
    ss >> rs2 >> imm >> rs1;
    rs2 = rs2.substr(0, rs2.length() - 1);  // Remove comma
    int bracket1 = imm.find('(');
    int bracket2 = imm.find(')');
    rs1 = imm.substr(bracket1 + 1, bracket2-bracket1-1);
    imm = imm.substr(0, bracket1 );  // Remove comma

    int address = registers[regNameMap[rs1]] + stoi(imm);
    int64_t value = registers[regNameMap[rs2]];

    // Store 64-bit value into 8 separate bytes
    for (int i = 0; i < 1; ++i) {
        memory[address + i] = (uint8_t)((value >> (8 * i)) & 0xFF);
    }
}
    else if (opcode == "beq") {
        ss >> rs1 >> rs2 >> imm;
        rs1 = rs1.substr(0, rs1.length() - 1);
        rs2 = rs2.substr(0, rs2.length() - 1);
        
        int targetPC;
        bool isLabel = false;
        
        // Check if the third operand (imm/label) is a label
        for (const auto& lbl : labelList) {
            if (lbl.name == imm) {
                targetPC = lbl.address * 4; // Set the target address from the label
                isLabel = true;
                break;
            }
        }

        // If not a label, treat it as an immediate value (integer offset)
        if (!isLabel) {
            targetPC = PC + stoi(imm) * 4; // Immediate value specifies offset in instructions, so multiply by 4
        }

        // Perform the branch if the condition is met
        if (registers[regNameMap[rs1]] == registers[regNameMap[rs2]]) {
            PC = targetPC;
            return; // Return to avoid incrementing PC after branching
        }
    }
    else if (opcode == "bne") {
        ss >> rs1 >> rs2 >> imm;
        rs1 = rs1.substr(0, rs1.length() - 1);
        rs2 = rs2.substr(0, rs2.length() - 1);
        
        int targetPC;
        bool isLabel = false;
        
        // Check if the third operand (imm/label) is a label
        for (const auto& lbl : labelList) {
            if (lbl.name == imm) {
                targetPC = lbl.address * 4; // Set the target address from the label
                isLabel = true;
                break;
            }
        }

        // If not a label, treat it as an immediate value (integer offset)
        if (!isLabel) {
            targetPC = PC + stoi(imm) * 4; // Immediate value specifies offset in instructions, so multiply by 4
        }

        // Perform the branch if the condition is met
        if (registers[regNameMap[rs1]] != registers[regNameMap[rs2]]) {
            PC = targetPC;
            return; // Return to avoid incrementing PC after branching
        }
    }
    else if (opcode == "blt") {
        ss >> rs1 >> rs2 >> imm;
        rs1 = rs1.substr(0, rs1.length() - 1);
        rs2 = rs2.substr(0, rs2.length() - 1);
        
        int targetPC;
        bool isLabel = false;
        
        // Check if the third operand (imm/label) is a label
        for (const auto& lbl : labelList) {
            if (lbl.name == imm) {
                targetPC = lbl.address * 4; // Set the target address from the label
                isLabel = true;
                break;
            }
        }

        // If not a label, treat it as an immediate value (integer offset)
        if (!isLabel) {
            targetPC = PC + stoi(imm) * 4; // Immediate value specifies offset in instructions, so multiply by 4
        }

        // Perform the branch if the condition is met
        if (registers[regNameMap[rs1]] < registers[regNameMap[rs2]]) {
            PC = targetPC;
            return; // Return to avoid incrementing PC after branching
        }
    }

    else if (opcode == "bge") {
        ss >> rs1 >> rs2 >> imm;
        rs1 = rs1.substr(0, rs1.length() - 1);
        rs2 = rs2.substr(0, rs2.length() - 1);
        
        int targetPC;
        bool isLabel = false;
        
        // Check if the third operand (imm/label) is a label
        for (const auto& lbl : labelList) {
            if (lbl.name == imm) {
                targetPC = lbl.address * 4; // Set the target address from the label
                isLabel = true;
                break;
            }
        }

        // If not a label, treat it as an immediate value (integer offset)
        if (!isLabel) {
            targetPC = PC + stoi(imm) * 4; // Immediate value specifies offset in instructions, so multiply by 4
        }

        // Perform the branch if the condition is met
        if (registers[regNameMap[rs1]] >= registers[regNameMap[rs2]]) {
            PC = targetPC;
            return; // Return to avoid incrementing PC after branching
        }
    }
    else if (opcode == "bgeu") {
        ss >> rs1 >> rs2 >> imm;
        rs1 = rs1.substr(0, rs1.length() - 1);
        rs2 = rs2.substr(0, rs2.length() - 1);
        
        int targetPC;
        bool isLabel = false;
        
        // Check if the third operand (imm/label) is a label
        for (const auto& lbl : labelList) {
            if (lbl.name == imm) {
                targetPC = lbl.address * 4; // Set the target address from the label
                isLabel = true;
                break;
            }
        }

        // If not a label, treat it as an immediate value (integer offset)
        if (!isLabel) {
            targetPC = PC + stoi(imm) * 4; // Immediate value specifies offset in instructions, so multiply by 4
        }

        // Perform the branch if the condition is met
        if ((unsigned int)registers[regNameMap[rs1]] >= (unsigned int)registers[regNameMap[rs2]]) {
            PC = targetPC;
            return; // Return to avoid incrementing PC after branching
        }
    }
    else if (opcode == "bltu") {
        ss >> rs1 >> rs2 >> imm;
        rs1 = rs1.substr(0, rs1.length() - 1);
        rs2 = rs2.substr(0, rs2.length() - 1);
        
        int targetPC;
        bool isLabel = false;
        
        // Check if the third operand (imm/label) is a label
        for (const auto& lbl : labelList) {
            if (lbl.name == imm) {
                targetPC = lbl.address * 4; // Set the target address from the label
                isLabel = true;
                break;
            }
        }

        // If not a label, treat it as an immediate value (integer offset)
        if (!isLabel) {
            targetPC = PC + stoi(imm) * 4; // Immediate value specifies offset in instructions, so multiply by 4
        }

        // Perform the branch if the condition is met
        if ((unsigned int)registers[regNameMap[rs1]] < (unsigned int)registers[regNameMap[rs2]]) {
            PC = targetPC;
            return; // Return to avoid incrementing PC after branching
        }
    }
    else if (opcode == "jal") {
        ss >> rd >> imm;
        rd = rd.substr(0, rd.length() - 1);
        
        int targetPC;
        bool isLabel = false;

        for (const auto& lbl : labelList) {
            if (lbl.name == imm) {
                targetPC = lbl.address * 4; 
                isLabel = true;
                break;
            }
        }

        if (!isLabel) {
            targetPC = PC + stoi(imm) * 4; // Immediate value specifies offset in instructions, so multiply by 4
        }

        registers[regNameMap[rd]] = PC + 4;
        
        PC = targetPC;
        return; 
    }
    else if (opcode == "jalr") {
        ss >> rd >> rs1 >> imm;

        rd = rd.substr(0, rd.length() - 1);
        int bracket1 = rs1.find('(');
        int bracket2 = rs1.find(')');
        imm = rs1.substr(bracket1 + 1, bracket2-bracket1-1);
        rs1 = rs1.substr(0, bracket1);
        
        int targetPC = (registers[regNameMap[rs1]] + stoi(imm)) & ~1;

        if (regNameMap[rd] != 0) { 
            registers[regNameMap[rd]] = PC + 4;
        }

        PC = targetPC;
        return;
    }
    else if (opcode == "lui") {
        ss >> rd >> imm;
        rd = rd.substr(0, rd.length() - 1);
        registers[regNameMap[rd]] = stoi(imm, nullptr, 0) << 12; 
    }

    PC += 4; 
}

void runProgram() {
    while (PC / 4 < instructions.size()) {
        if (find(breakpoints.begin(), breakpoints.end(), PC) != breakpoints.end()) {
            cout << "Execution stopped at breakpoint\n";
            return;  // Exit the function to pause execution
        }
        cout << "Executed " << instructions[PC / 4] << " ; PC = 0x" << setw(8) << setfill('0') << hex << PC << "\n";
        executeInstruction(instructions[PC / 4]);
    }
    cout << dec; 
}

void printMemory(int addr, int count) {
    for (int i = 0; i < count; ++i) {
        if ((addr + i) < 0x50000) { 
            // Print each byte as a two-digit hexadecimal value
            cout << "Memory[0x" << setw(8) << setfill('0') << (addr + i) <<"] : 0x"
                 << setw(2) << setfill('0') << (memory[addr + i] & 0xFF) << "\n";
        } else {
            cout << "Error: Address out of bounds.\n";
            break;
        }
    }
    cout << dec;  // Reset number format to decimal
}

void stepProgram() {
    if (PC / 4 < instructions.size()) {
        cout << "Executed " << instructions[PC / 4] << " ; PC = 0x" << setw(8) << setfill('0') << hex << PC << "\n";
        executeInstruction(instructions[PC / 4]);
    } else {
        cout << "Nothing to step\n";
    }
    cout << dec;
}

int main() {
    string command;
    while (true) {
        getline(cin, command);
        stringstream ss(command);
        string cmd, filename;
        ss >> cmd;
        if (cmd == "load") {
            ss >> filename;
            if (!loadInstructions(filename)) {
                cout << "Failed to load file: " << filename << "\n";
            }
        }
        else if (cmd == "run") {
            runProgram();
        }
        else if (cmd == "regs") {
            printRegisters();
        }
        else if (cmd == "mem") {
            int addr, count;
            ss >> hex >> addr >> dec >> count;
            printMemory(addr, count);
        }
        else if (cmd == "step") {
            stepProgram();
        }
        else if (cmd == "break") {
            int line;
            ss >> line;
            if (line >= 0 && line < instructions.size()) {
                breakpoints.push_back((line-1) * 4);  // Store the address as line * 4 (since each instruction is 4 bytes)
                cout << "Breakpoint set at line " << line << "\n";
            } else {
                cout << "Error: Invalid line number.\n";
            }
        }
        else if (cmd == "del" && ss.peek() == ' ') {
            string subcmd;
            ss >> subcmd;
            if (subcmd == "break") {
                int line;
                ss >> line;
                int pcValue = (line-1) * 4;
                auto it = find(breakpoints.begin(), breakpoints.end(), pcValue);
                if (it != breakpoints.end()) {
                    breakpoints.erase(it);
                    cout << "Breakpoint removed at line " << line << "\n";
                } else {
                    cout << "Error: No breakpoint set at line " << line << ".\n";
                }
            }
        }
        else if (cmd == "exit") {
            cout << "Exited the simulator\n";
            break;
        }
        else {
            cout << "Unknown command: " << cmd << "\n";
        }
        cout << "\n";
    }

    return 0;
}