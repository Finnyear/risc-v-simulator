#include <bits/stdc++.h>
using namespace std;

unsigned int reg[32], pc;

unsigned int incode;

unsigned int rs1, rs2, rd, funct3, funct7, opcode;

int imm;

enum format_set {R, I, S, B, U, J};

format_set format;

stringstream sstream;

unsigned int mem[6666666];

bool flag = 0;

int fro[32];
void init(){
	for(int i = 0; i < 32; i++)
		fro[i] = -1 << i;
	
	string str, tmp;
	while(cin >> str){
		// cout << "str = " << str << endl; 
		if(str[0] == '@'){
			sstream << str.substr(1);
			sstream >> hex >> uppercase >> pc;
			sstream.clear();
			// cout << "pc == " << pc << endl;
		}
		else{
			sstream << str;
			sstream >> hex >> uppercase >> mem[pc++];
			sstream.clear();
			// cout << "pc == " << pc << endl;
			// cout << "x == " << hex << uppercase << mem[pc - 1] << endl;
		}
	}
}

void instruction_fetch(){
	incode = 0;
	for(int i = 0; i < 4; i++)
		incode |= (mem[pc + i] << (i * 8));
	// printf("incode == %08x\n", incode);
}
void decode_R(){
	format = R;
	rd = (incode >> 7) & 31;
	funct3 = (incode >> 12) & 7;
	rs1 = (incode >> 15) & 31;
	rs2 = (incode >> 20) & 31;
	funct7 = (incode >> 25);
	// printf("R ::: rd = %d  rs1 = %d  rs2 = %d  funct3 = %d  funct7 = %d\n", rd, rs1, rs2, funct3, funct7);
}
void print_R(){
	// printf("rd = [%d : %d]  rs1 = [%d : %d]  rs2 = [%d : %d]\n", rd, reg[rd], rs1, reg[rs1], rs2, reg[rs2]);
}
void decode_I(){
	format = I;
	rd = (incode >> 7) & 31;
	funct3 = (incode >> 12) & 7;
	rs1 = (incode >> 15) & 31;
	imm = (incode >> 20);
	if(incode >> 31) imm |= fro[11];
	// printf("I ::: rd = %d  rs1 = %d  imm = %d  funct3 = %d\n", rd, rs1, imm, funct3);
}
void print_I(){
	// printf("rd = [%d : %d]  rs1 = [%d : %d]\n", rd, reg[rd], rs1, reg[rs1]);
}

void decode_S(){
	format = S;
	funct3 = (incode >> 12) & 7;
	rs1 = (incode >> 15) & 31;
	rs2 = (incode >> 20) & 31;
	imm = ((incode >> 7) & 31) | (((incode >> 25) & 63) << 5);
	if(incode >> 31) imm |= fro[11];
	// printf("S ::: rs1 = %d  rs2 = %d  imm = %d  funct3 = %d\n", rs1, rs2, imm, funct3);
}
void print_S(){
	// printf("rs1 = [%d : %d]  rs2 = [%d : %d]\n", rs1, reg[rs1], rs2, reg[rs2]);
}
void decode_B(){
	format = B;
	funct3 = (incode >> 12) & 7;
	rs1 = (incode >> 15) & 31;
	rs2 = (incode >> 20) & 31;
	imm = (((incode >> 8) & 15) << 1) | (((incode >> 25) & 63) << 5) | (((incode >> 7) & 1) << 11);
	if(incode >> 31) imm |= fro[12];
	// printf("B ::: rs1 = %d  rs2 = %d  imm = %d  funct3 = %d\n", rs1, rs2, imm, funct3);
}
void print_B(){
	// printf("rs1 = [%d : %d]  rs2 = [%d : %d]\n", rs1, reg[rs1], rs2, reg[rs2]);
}
void decode_U(){
	format = U;
	rd = (incode >> 7) & 31;
	imm = incode & fro[12];
	// printf("U ::: rd = %d  imm = %d\n", rd, imm);
}
void print_U(){
	// printf("rd = [%d : %d]\n", rd, reg[rd]);
}
void decode_J(){
	format = J;
	rd = (incode >> 7) & 31;
	imm = (((incode >> 21) & 1023) << 1) | (((incode >> 20) & 1) << 11) | (((incode >> 12) & 255) << 12);
	if(incode >> 31) imm |= fro[20];
	// printf("J ::: rd = %d  imm = %d\n", rd, imm);
}
void print_J(){
	// printf("rd = [%d : %d]\n", rd, reg[rd]);
}

void instruction_decode(){
	opcode = incode & 127;
	if(opcode == 55) decode_U();
	else if(opcode == 23) decode_U();
	else if(opcode == 111) decode_J();
	else if(opcode == 103) decode_I();
	else if(opcode == 99) decode_B();
	else if(opcode == 3) decode_I();
	else if(opcode == 35) decode_S();
	else if(opcode == 19) decode_I();
	else if(opcode == 51) decode_R();
}
void print(){
	if(opcode == 55) print_U();
	else if(opcode == 23) print_U();
	else if(opcode == 111) print_J();
	else if(opcode == 103) print_I();
	else if(opcode == 99) print_B();
	else if(opcode == 3) print_I();
	else if(opcode == 35) print_S();
	else if(opcode == 19) print_I();
	else if(opcode == 51) print_R();
}
int read_signed(int ad, int len){
	int ans = 0;
	for(int i = 0; i < len / 8; i++)
		ans |= mem[ad + i] << (i * 8);
	if(ans >> (len - 1))
		ans |= fro[len - 1];
	return ans;
}
int read_unsigned(int ad, int len){
	unsigned int ans = 0;
	for(int i = 0; i < len / 8; i++)
		ans |= mem[ad + i] << (i * 8);
	return ans;
}
void execute(){
	// puts("---begin---");
	// printf("pc === %08x\n", pc);
	if(opcode == 55){
		// puts("LUI");
		reg[rd] = imm;
		pc += 4;
	}
	else if(opcode == 23){
		// puts("AUIPC");
		pc += imm;
		reg[rd] = pc;
	}
	else if(opcode == 111){
		// puts("JAL");
		if(rd) reg[rd] = pc + 4;
		pc += imm;
	}
	else if(opcode == 103){
		// puts("JALR");
		if(rd) reg[rd] = pc + 4;
		pc = (imm + reg[rs1]) & fro[1];
	}
	else if(opcode == 99){
		if(funct3 == 0){
			// puts("BEQ");
			if(reg[rs1] == reg[rs2]) pc += imm;
			else pc += 4;
		}
		else if(funct3 == 1){
			// puts("BNE");
			if(reg[rs1] != reg[rs2]) pc += imm;
			else pc += 4;
		}
		else if(funct3 == 4){
			// puts("BLT");
			if(((int)reg[rs1]) < ((int)reg[rs2])) pc += imm;
			else pc += 4;
		}
		else if(funct3 == 5){
			// puts("BGE");
			if(((int)reg[rs1]) >= ((int)reg[rs2])) pc += imm;
			else pc += 4;
		}
		else if(funct3 == 6){
			// puts("BLTU");
			if(reg[rs1] < reg[rs2]) pc += imm;
			else pc += 4;
		}
		else if(funct3 == 7){
			// puts("BGEU");
			if(reg[rs1] >= reg[rs2]) pc += imm;
			else pc += 4;
		}
	}
	else if(opcode == 3){
		pc += 4;
		if(funct3 == 0){
			// puts("LB");
			reg[rd] = read_signed(reg[rs1] + imm, 8);
		}
		else if(funct3 == 1){
			// puts("LH");
			reg[rd] = read_signed(reg[rs1] + imm, 16);
		}
		else if(funct3 == 2){
			// puts("LW");
			reg[rd] = read_unsigned(reg[rs1] + imm, 32);
		}
		else if(funct3 == 4){
			// puts("LBU");
			reg[rd] = read_unsigned(reg[rs1] + imm, 8);
		}
		else if(funct3 == 5){
			// puts("LHU");
			reg[rd] = read_unsigned(reg[rs1] + imm, 16);
		}
	}
	else if(opcode == 35){
		pc += 4;
		if(funct3 == 0){
			// puts("SB");
			mem[reg[rs1] + imm] = reg[rs2] & 0xff;
		}
		else if(funct3 == 1){
			// puts("SH");
			for(int i = 0; i < 2; i++)
				mem[reg[rs1] + imm + i] = (reg[rs2] >> (8 * i))& 0xff;
		}
		else if(funct3 == 2){
			// puts("SW");
			for(int i = 0; i < 4; i++)
				mem[reg[rs1] + imm + i] = (reg[rs2] >> (8 * i))& 0xff;
		}
	}
	else if(opcode == 19){
		pc += 4;
		if(funct3 == 0){
			// puts("ADDI");
			if(rd == 10 && rs1 == 0 && imm == 255){
				flag = 1;
				cout << (reg[10] & 255) << endl;
			}
			else reg[rd] = reg[rs1] + imm;
		}
		else if(funct3 == 2){
			// puts("SLTI");
			if(((int)reg[rs1]) < imm) reg[rd] = 1;
			else reg[rd] = 0;
		}
		else if(funct3 == 3){
			// puts("SLTIU");
			if(reg[rs1] < ((unsigned int)imm)) reg[rd] = 1;
			else reg[rd] = 0;
		}
		else if(funct3 == 4){
			// puts("XORI");
			reg[rd] = reg[rs1] ^ imm;
		}
		else if(funct3 == 6){
			// puts("ORI");
			reg[rd] = reg[rs1] | imm;
		}
		else if(funct3 == 7){
			// puts("ANDI");
			reg[rd] = reg[rs1] & imm;
		}
		else if(funct3 == 1){
			// puts("SLLI");
			reg[rd] = reg[rs1] << (imm & 31);
		}
		else if(funct3 == 5){
			if((imm >> 5) == 0){
				// puts("SRLI");
				reg[rd] = reg[rs1] >> (imm & 31);
			}
			else{
				// puts("SRAI");
				reg[rd] = ((int)reg[rs1]) >> (imm & 31);
			}
		}
	}
	else if(opcode == 51){
		pc += 4;
		if(funct3 == 0){
			if(funct7 == 0){
				// puts("ADD");
				reg[rd] = reg[rs1] + reg[rs2];
			}
			else{
				// puts("SUB");
				reg[rd] = reg[rs1] - reg[rs2];
			}
		}
		else if(funct3 == 1){
			// puts("SLL");
			reg[rd] = reg[rs1] << (reg[rs2] & 31);
		}
		else if(funct3 == 2){
			// puts("SLT");
			if(((int)reg[rs1]) < ((int)reg[rs2])) reg[rd] = 1;
			else reg[rd] = 0;
		}
		else if(funct3 == 3){
			// puts("SLTU");
			if(reg[rs1] < reg[rs2]) reg[rd] = 1;
			else reg[rd] = 0;
		}
		else if(funct3 == 4){
			// puts("XOR");
			reg[rd] = reg[rs1] ^ reg[rs2];
		}
		else if(funct3 == 5){
			if(funct7 == 0){
				// puts("SRL");
				reg[rd] = reg[rs1] >> (reg[rs2] & 31);
			}
			else{
				// puts("SRA");
				reg[rd] = ((int)reg[rs1]) >> (reg[rs2] & 31);
			}
		}
		else if(funct3 == 6){
			// puts("OR");
			reg[rd] = reg[rs1] | reg[rs2];
		}
		else if(funct3 == 7){
			// puts("AND");
			reg[rd] = reg[rs1] & reg[rs2];
		}
	}
	// printf("pc === %08x\n", pc);
	// print();
	// puts("---END---");
}
void memory_access(){}
void write_back(){}
void run(){
	instruction_fetch();
	instruction_decode();
	execute();
	memory_access();
	write_back();
}
void work(){
	pc = 0;
	while(!flag)
		run();
}
int main(){
	// freopen("tak.data", "r", stdin);
	// freopen("1.out", "w", stdout);
	init();
	work();
	return 0;
}