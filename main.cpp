#include <bits/stdc++.h>
using namespace std;

unsigned int reg[32], pc;
unsigned int mem[6666666];
int pre[256], pre_tot, pre_right;

int fro[32];
stringstream sstream;
void init(){
	for(int i = 0; i < 32; i++)
		fro[i] = -1 << i;
	string str, tmp;
	while(cin >> str){
		if(str[0] == '@'){
			sstream << str.substr(1);
			sstream >> hex >> uppercase >> pc;
			sstream.clear();
		}
		else{
			sstream << str;
			sstream >> hex >> uppercase >> mem[pc++];
			sstream.clear();
		}
	}
}

enum instruction_set {LUI = 0, AUIPC, JAL, JALR, 
					  BEQ, BNE, BLT, BGE, BLTU, BGEU, 
					  LB, LH, LW, LBU, LHU, SB, SH, SW, 
					  ADDI, SLTI, SLTIU, XORI, ORI, ANDI, SLLI, SRLI, SRAI, 
					  ADD, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND, 
					  ERROR};
string ss[55] = {"LUI", "AUIPC", "JAL", "JALR",
				 "BEQ", "BNE", "BLT", "BGE", "BLTU", "BGEU", 
				 "LB", "LH", "LW", "LBU", "LHU", "SB", "SH", "SW", 
				 "ADDI", "SLTI", "SLTIU", "XORI", "ORI", "ANDI", "SLLI", "SRLI", "SRAI", 
				 "ADD", "SUB", "SLL", "SLT", "SLTU", "XOR", "SRL", "SRA", "OR", "AND", 
				 "ERROR"};

struct pipeline_reg{
	instruction_set ins_type;
	// format_set ins_format;
	unsigned int ins_code, pc, add;
	unsigned int rs1_value, rs2_value, rd_value;
	int imm, rs1, rs2, rd, funct3, funct7, opcode, predictor;
	pipeline_reg(unsigned int _ins_code = 0) : ins_code(_ins_code) {
		ins_type = ERROR;
		pc = add = rs1_value = rs2_value = rd_value = 0;
		imm = rs1 = rs2 = rd = funct3 = funct7 = opcode = predictor = 0;
	}
}IF_ID, ID_EX, EX_MEM, MEM_WB;
bool IF_busy = 1, ID_busy, EX_busy, MEM_busy, WB_busy;
int MEM_times, EX_rd, MEM_rd;
unsigned int EX_rd_value, MEM_rd_value;

void instruction_fetch(){
	if(ID_busy) return ;
	// puts("-----IF-Begin-----");
	IF_ID = pipeline_reg();
	unsigned int ins_code = (mem[pc]) | 
							(mem[pc + 1] << 8) | 
							(mem[pc + 2] << 16) | 
							(mem[pc + 3] << 24) ;
	IF_ID.ins_code = ins_code;
	IF_ID.pc = pc;
	pc += 4;
	ID_busy = 1;
	// puts("-----IF-End-----");
}

void instruction_decode(){
	if(EX_busy || (!ID_busy)) return ;
	// puts("-----ID-Begin-----");
	// printf("ID :: %s  pc = %08x  reg[%d] = %d  reg[%d] = %d  reg[%d] = %d\n", ss[IF_ID.ins_type].c_str(), IF_ID.pc, IF_ID.rs1, IF_ID.rs1_value, IF_ID.rs2, IF_ID.rs2_value, IF_ID.rd, IF_ID.rd_value);

	if(IF_ID.ins_code == 0xFF00513){
		// printf("??? %s  pc = %08x  reg[%d] = %d  reg[%d] = %d  reg[%d] = %d\n", ss[IF_ID.ins_type].c_str(), IF_ID.pc, IF_ID.rs1, IF_ID.rs1_value, IF_ID.rs2, IF_ID.rs2_value, IF_ID.rd, IF_ID.rd_value);
		//puts("??????????");
		return ;
	}
	
	ID_EX = IF_ID;
	int opcode, funct3, funct7;
	opcode = ID_EX.ins_code & 127;
	switch(opcode){
		case 0x37:
			ID_EX.ins_type = LUI;
			ID_EX.rd = (ID_EX.ins_code >> 7) & 31;
			ID_EX.imm = ID_EX.ins_code & fro[12];
			break;
		case 0x17:
			ID_EX.ins_type = AUIPC;
			ID_EX.rd = (ID_EX.ins_code >> 7) & 31;
			ID_EX.imm = ID_EX.ins_code & fro[12];
			break;
		case 0x6F:
			ID_EX.ins_type = JAL;
			ID_EX.rd = (ID_EX.ins_code >> 7) & 31;
			ID_EX.imm = (((ID_EX.ins_code >> 21) & 1023) << 1) | (((ID_EX.ins_code >> 20) & 1) << 11) | (((ID_EX.ins_code >> 12) & 255) << 12);
			if(ID_EX.ins_code >> 31) ID_EX.imm |= fro[20];
			break;
		case 0x67:
			ID_EX.rd = (ID_EX.ins_code >> 7) & 31;
			funct3 = (ID_EX.ins_code >> 12) & 7;
			ID_EX.rs1 = (ID_EX.ins_code >> 15) & 31;
			ID_EX.imm = (ID_EX.ins_code >> 20);
			if(ID_EX.ins_code >> 31) ID_EX.imm |= fro[11];
			switch(funct3){
				case 0: ID_EX.ins_type = JALR; break;
				default : break;
			}
			break;
		case 0x63:
			funct3 = (ID_EX.ins_code >> 12) & 7;
			ID_EX.rs1 = (ID_EX.ins_code >> 15) & 31;
			ID_EX.rs2 = (ID_EX.ins_code >> 20) & 31;
			ID_EX.imm = (((ID_EX.ins_code >> 8) & 15) << 1) | (((ID_EX.ins_code >> 25) & 63) << 5) | (((ID_EX.ins_code >> 7) & 1) << 11);
			if(ID_EX.ins_code >> 31) ID_EX.imm |= fro[12];
			switch(funct3){
				case 0: ID_EX.ins_type = BEQ; break;
				case 1: ID_EX.ins_type = BNE; break;
				case 4: ID_EX.ins_type = BLT; break;
				case 5: ID_EX.ins_type = BGE; break;
				case 6: ID_EX.ins_type = BLTU; break;
				case 7: ID_EX.ins_type = BGEU; break;
				default : break;
			}
			break;
		case 0x3:
			ID_EX.rd = (ID_EX.ins_code >> 7) & 31;
			funct3 = (ID_EX.ins_code >> 12) & 7;
			ID_EX.rs1 = (ID_EX.ins_code >> 15) & 31;
			ID_EX.imm = (ID_EX.ins_code >> 20);
			if(ID_EX.ins_code >> 31) ID_EX.imm |= fro[11];
			switch(funct3){
				case 0: ID_EX.ins_type = LB; break;
				case 1: ID_EX.ins_type = LH; break;
				case 2: ID_EX.ins_type = LW; break;
				case 4: ID_EX.ins_type = LBU; break;
				case 5: ID_EX.ins_type = LHU; break;
				default : break;
			}
			break;
		case 0x23:
			funct3 = (ID_EX.ins_code >> 12) & 7;
			ID_EX.rs1 = (ID_EX.ins_code >> 15) & 31;
			ID_EX.rs2 = (ID_EX.ins_code >> 20) & 31;
			ID_EX.imm = ((ID_EX.ins_code >> 7) & 31) | (((ID_EX.ins_code >> 25) & 63) << 5);
			if(ID_EX.ins_code >> 31) ID_EX.imm |= fro[11];
			switch(funct3){
				case 0: ID_EX.ins_type = SB; break;
				case 1: ID_EX.ins_type = SH; break;
				case 2: ID_EX.ins_type = SW; break;
				default : break;
			}
			break;
		case 0x13:
			ID_EX.rd = (ID_EX.ins_code >> 7) & 31;
			funct3 = (ID_EX.ins_code >> 12) & 7;
			ID_EX.rs1 = (ID_EX.ins_code >> 15) & 31;
			ID_EX.imm = (ID_EX.ins_code >> 20);
			if(ID_EX.ins_code >> 31) ID_EX.imm |= fro[11];
			switch(funct3){
				case 0: ID_EX.ins_type = ADDI; break;
				case 2: ID_EX.ins_type = SLTI; break;
				case 3: ID_EX.ins_type = SLTIU; break;
				case 4: ID_EX.ins_type = XORI; break;
				case 6: ID_EX.ins_type = ORI; break;
				case 7: ID_EX.ins_type = ANDI; break;
				case 1: 
					ID_EX.imm &= 31;
					funct7 = (ID_EX.ins_code >> 25);
					switch(funct7){
						case 0x0: ID_EX.ins_type = SLLI; break;
						default : break;
					}
					break;
				case 5:
					ID_EX.imm &= 31;
					funct7 = (ID_EX.ins_code >> 25);
					switch(funct7){
						case 0x00: ID_EX.ins_type = SRLI; break;
						case 0x20: ID_EX.ins_type = SRAI; break;
						default : break;
					}
					break;
			}
			break;
		case 0x33:	
			ID_EX.rd = (ID_EX.ins_code >> 7) & 31;
			funct3 = (ID_EX.ins_code >> 12) & 7;
			ID_EX.rs1 = (ID_EX.ins_code >> 15) & 31;
			ID_EX.rs2 = (ID_EX.ins_code >> 20) & 31;
			funct7 = (ID_EX.ins_code >> 25);
			switch(funct3){
				case 0:
					switch(funct7){
						case 0x00: ID_EX.ins_type = ADD; break;
						case 0x20: ID_EX.ins_type = SUB; break;
						default : break;
					}
					break;
				case 1:
					switch(funct7){
						case 0: ID_EX.ins_type = SLL; break;
						default : break;
					}
					break;
				case 2:
					switch(funct7){
						case 0: ID_EX.ins_type = SLT; break;
						default : break;
					}
					break;
				case 3:
					switch(funct7){
						case 0: ID_EX.ins_type = SLTU; break;
						default : break;
					}
					break;
				case 4:
					switch(funct7){
						case 0: ID_EX.ins_type = XOR; break;
						default : break;
					}
					break;
				case 5:
					switch(funct7){
						case 0x00: ID_EX.ins_type = SRL; break;
						case 0x20: ID_EX.ins_type = SRA; break;
						default : break;
					}
					break;
				case 6:
					switch(funct7){
						case 0: ID_EX.ins_type = OR; break;
						default : break;
					}
					break;
				case 7:
					switch(funct7){
						case 0: ID_EX.ins_type = AND; break;
						default : break;
					}
					break;
			}
			break;
		default : break;
	}
	// printf("%08x  %d  %d  %d  %d\n", ID_EX.pc, MEM_busy, EX_MEM.rd, ID_EX.rs1, ID_EX.rs2);
	if(MEM_busy && EX_MEM.rd && (EX_MEM.rd == ID_EX.rs1 || EX_MEM.rd == ID_EX.rs2)){
		// printf("pc == %08x\n", ID_EX.pc);
		return ;
	}
	// printf("pc == %08x  reg[%d] = %d  reg[%d] = %d  reg[%d] = %d\n", ID_EX.pc, ID_EX.rs1, ID_EX.rs1_value, 
	// 	ID_EX.rs2, ID_EX.rs2_value, MEM_rd, MEM_rd_value);
	ID_EX.rs1_value = reg[ID_EX.rs1];
	ID_EX.rs2_value = reg[ID_EX.rs2];
	if(ID_EX.rs1 == MEM_rd){
		if(ID_EX.rs1)ID_EX.rs1_value = MEM_rd_value;
		// else printf("1 :: %08x\n", ID_EX.pc);
	}
	if(ID_EX.rs2 == MEM_rd){
		if(ID_EX.rs2)ID_EX.rs2_value = MEM_rd_value;
		// else printf("2 :: %08x\n", ID_EX.pc);
	}
	MEM_rd = 0;
	/*if(ID_EX.rs1 == EX_rd){
		if(ID_EX.rs1)ID_EX.rs1_value = EX_rd_value;
		// else printf("3 :: %08x\n", ID_EX.pc);
	}
	if(ID_EX.rs2 == EX_rd){
		if(ID_EX.rs2)ID_EX.rs2_value = EX_rd_value;
		// else printf("4 :: %08x\n", ID_EX.pc);
	}
	EX_rd = 0;*/
	//ID_rs1_forwarding = ID_rs2_forwarding = 0;
	// printf("reg[%d] = %d  reg[%d] = %d\n", ID_EX.rs1, ID_EX.rs1_value, ID_EX.rs2, ID_EX.rs2_value);
	switch(ID_EX.ins_type){
		case JAL: pc = ID_EX.pc + ID_EX.imm; break;
		case JALR: pc = (ID_EX.imm + ID_EX.rs1_value) & fro[1]; break;
		case BEQ: case BNE: case BLT: case BGE: case BLTU: case BGEU:
			ID_EX.predictor = (pre[ID_EX.pc >> 2 & 0xFF] >> 1) & 1;
			if(ID_EX.predictor) pc = ID_EX.pc + ID_EX.imm;
			pre_tot++;
			break;
		default : break;
	}
	// puts("-----ID-End-----");
	ID_busy = 0;
	EX_busy = 1;
	// printf("ID :: %08x  %08x  pc = %d\n", ID_EX.pc, ID_EX.ins_code, pc);
}
bool cmp(instruction_set ins, unsigned int rs1_value, unsigned int rs2_value){
	switch(ins){
		case BEQ: return rs1_value == rs2_value;
		case BNE: return rs1_value != rs2_value;
		case BLT: return ((int)rs1_value) < ((int)rs2_value);
		case BGE: return ((int)rs1_value) >= ((int)rs2_value);
		case BLTU: return rs1_value < rs2_value;
		case BGEU: return rs1_value >= rs2_value;
		default : break;
	}
	return 0;
}
void execute(){
	if(MEM_busy || (!EX_busy)) return ;
	// puts("-----EX-Begin-----");
	// printf("%s  pc = %08x  reg[%d] = %d  reg[%d] = %d  reg[%d] = %d\n", ss[ID_EX.ins_type].c_str(), ID_EX.pc, ID_EX.rs1, ID_EX.rs1_value, ID_EX.rs2, ID_EX.rs2_value, ID_EX.rd, ID_EX.rd_value);
	EX_MEM = ID_EX;
	switch(EX_MEM.ins_type){
		case LUI: EX_MEM.rd_value = EX_MEM.imm; break;
		case AUIPC: EX_MEM.rd_value = EX_MEM.pc + EX_MEM.imm; break;
		case JAL: case JALR: EX_MEM.rd_value = EX_MEM.pc + 4; break;
		case BEQ: case BNE: case BLT: case BGE: case BLTU: case BGEU:
			if(cmp(EX_MEM.ins_type, EX_MEM.rs1_value, EX_MEM.rs2_value)){
				if(EX_MEM.predictor) pre_right++;
				else{pc = EX_MEM.pc + EX_MEM.imm; ID_busy = 0;}
				pre[(EX_MEM.pc >> 2) & 0xFF] = min(pre[(EX_MEM.pc >> 2) & 0xFF] + 1, 3);
			}
			else{
				if(!EX_MEM.predictor) pre_right++;
				else{pc = EX_MEM.pc + 4; ID_busy = 0;}
				pre[(EX_MEM.pc >> 2) & 0xFF] = max(pre[(EX_MEM.pc >> 2) & 0xFF] - 1, 0);
			}
			break;
		case LB: case LH: case LW: case LBU: case LHU: case SB: case SH: case SW:
			EX_MEM.add = EX_MEM.rs1_value + EX_MEM.imm; break;
		case ADDI: EX_MEM.rd_value = EX_MEM.rs1_value + EX_MEM.imm; break;
		case SLTI: EX_MEM.rd_value = ((int)EX_MEM.rs1_value) < EX_MEM.imm; break;
		case SLTIU: EX_MEM.rd_value = EX_MEM.rs1_value < ((unsigned int) EX_MEM.imm); break;
		case XORI: EX_MEM.rd_value = EX_MEM.rs1_value ^ EX_MEM.imm; break;
		case ORI: EX_MEM.rd_value = EX_MEM.rs1_value | EX_MEM.imm; break;
		case ANDI: EX_MEM.rd_value = EX_MEM.rs1_value & EX_MEM.imm; break;
		case SLLI: EX_MEM.rd_value = EX_MEM.rs1_value << (EX_MEM.imm & 31); break;
		case SRLI: EX_MEM.rd_value = EX_MEM.rs1_value >> (EX_MEM.imm & 31); break;
		case SRAI: EX_MEM.rd_value = ((int)EX_MEM.rs1_value) >> (EX_MEM.imm & 31); break;
		case ADD: EX_MEM.rd_value = EX_MEM.rs1_value + EX_MEM.rs2_value; break;
		case SUB: EX_MEM.rd_value = EX_MEM.rs1_value - EX_MEM.rs2_value; break;
		case SLL: EX_MEM.rd_value = EX_MEM.rs1_value << (EX_MEM.rs2_value & 31); break;
		case SLT: EX_MEM.rd_value = ((int)EX_MEM.rs1_value) < ((int)EX_MEM.rs2_value); break;
		case SLTU: EX_MEM.rd_value = EX_MEM.rs1_value < EX_MEM.rs2_value; break;
		case XOR: EX_MEM.rd_value = EX_MEM.rs1_value ^ EX_MEM.rs2_value; break;
		case SRL: EX_MEM.rd_value = EX_MEM.rs1_value >> (EX_MEM.rs2_value & 31); break;
		case SRA: EX_MEM.rd_value = ((int)EX_MEM.rs1_value) >> (EX_MEM.rs2_value & 31); break;
		case OR: EX_MEM.rd_value = EX_MEM.rs1_value | EX_MEM.rs2_value; break;
		case AND: EX_MEM.rd_value = EX_MEM.rs1_value & EX_MEM.rs2_value; break;
	}
	// printf("%s  pc = %08x  reg[%d] = %d  reg[%d] = %d  reg[%d] = %d\n", ss[ID_EX.ins_type].c_str(), ID_EX.pc, ID_EX.rs1, ID_EX.rs1_value, ID_EX.rs2, ID_EX.rs2_value, ID_EX.rd, ID_EX.rd_value);
	// puts("-----EX-End-----");
	EX_busy = 0;
	MEM_busy = 1;
	MEM_times = 0;
	EX_rd = EX_MEM.rd;
	EX_rd_value = EX_MEM.rd_value;
	// printf("EX :: pc = %08x  reg[%d] = %d\n", EX_MEM.pc, EX_rd, EX_rd_value);
}
void memory_access(){
	if(WB_busy || (!MEM_busy)) return ;
	// puts("-----MEM-Begin-----");
	// printf("MEM_times == %d\n", MEM_times);
	// printf("%s  pc = %08x  reg[%d] = %d  reg[%d] = %d  reg[%d] = %d\n", ss[EX_MEM.ins_type].c_str(), EX_MEM.pc, EX_MEM.rs1, EX_MEM.rs1_value, EX_MEM.rs2, EX_MEM.rs2_value, EX_MEM.rd, EX_MEM.rd_value);
	MEM_WB = EX_MEM;
	switch(MEM_WB.ins_type){
		case LB: case LH: case LW: case LBU: case LHU: case SB: case SH: case SW:
			if(MEM_times < 2) MEM_times++;
			else{
				switch(MEM_WB.ins_type){
					case LB: MEM_WB.rd_value = (char)(mem[MEM_WB.add]); break;
					case LH: MEM_WB.rd_value = (short)(mem[MEM_WB.add] | (mem[MEM_WB.add + 1] << 8)); break;
					case LW: MEM_WB.rd_value = mem[MEM_WB.add] | (mem[MEM_WB.add + 1] << 8) | 
											   (mem[MEM_WB.add + 2] << 16) | (mem[MEM_WB.add + 3] << 24); 
							 break;
					case LBU: MEM_WB.rd_value = mem[MEM_WB.add]; break;
					case LHU: MEM_WB.rd_value = mem[MEM_WB.add] | (mem[MEM_WB.add + 1] << 8); break;
					case SB: mem[MEM_WB.add] = MEM_WB.rs2_value & 0xFF;
							 break;
					case SH: mem[MEM_WB.add] = MEM_WB.rs2_value & 0xFF;
							 mem[MEM_WB.add + 1] = (MEM_WB.rs2_value >> 8) & 0xFF;
							 break;
					case SW: mem[MEM_WB.add] = MEM_WB.rs2_value & 0xFF;
							 mem[MEM_WB.add + 1] = (MEM_WB.rs2_value >> 8) & 0xFF;
							 mem[MEM_WB.add + 2] = (MEM_WB.rs2_value >> 16) & 0xFF;
							 mem[MEM_WB.add + 3] = (MEM_WB.rs2_value >> 24) & 0xFF;
							 break;
					default : break;
				}
				MEM_busy = 0;
				WB_busy = 1;
			}
			break;
		default:
			MEM_busy = 0;
			WB_busy = 1;
			break;
	}
	// printf("%s  pc = %08x  reg[%d] = %d  reg[%d] = %d  reg[%d] = %d\n", ss[MEM_WB.ins_type].c_str(), MEM_WB.pc, MEM_WB.rs1, MEM_WB.rs1_value, MEM_WB.rs2, MEM_WB.rs2_value, MEM_WB.rd, MEM_WB.rd_value);
	/*if(ID_busy){
		printf("%08x, %08x, %d  %d  %d\n", MEM_WB.pc, IF_ID.pc, IF_ID.rs1, IF_ID.rs2, MEM_WB.rd);
		if(IF_ID.rs1 && IF_ID.rs1 == MEM_WB.rd) {IF_ID.rs1_value = MEM_WB.rd_value; ID_rs1_forwarding = 1;}
		if(IF_ID.rs2 && IF_ID.rs2 == MEM_WB.rd) {IF_ID.rs2_value = MEM_WB.rd_value; ID_rs2_forwarding = 1;}
	}*/
	MEM_rd = MEM_WB.rd;
	MEM_rd_value = MEM_WB.rd_value;
	// printf("MEM :: pc = %08x  reg[%d] = %d\n", MEM_WB.pc, MEM_rd, MEM_rd_value);
	if(EX_busy){
		if(ID_EX.rs1 && ID_EX.rs1 == MEM_WB.rd) ID_EX.rs1_value = MEM_WB.rd_value;
		if(ID_EX.rs2 && ID_EX.rs2 == MEM_WB.rd) ID_EX.rs2_value = MEM_WB.rd_value;
	}
	// puts("-----MEM-End-----");
}
void write_back(){
	if(!WB_busy) return ;
	// puts("-----WB-Begin-----");
	// printf("%s  pc = %08x  imm = %d  reg[%d] = %d  reg[%d] = %d  reg[%d] = %d\n", ss[MEM_WB.ins_type].c_str(), MEM_WB.pc, MEM_WB.imm, MEM_WB.rs1, MEM_WB.rs1_value, MEM_WB.rs2, MEM_WB.rs2_value, MEM_WB.rd, MEM_WB.rd_value);
	/*if(ID_busy){
		printf("%08x, %08x, %d  %d  %d\n", MEM_WB.pc, IF_ID.pc, IF_ID.rs1, IF_ID.rs2, MEM_WB.rd);
		if(IF_ID.rs1 && IF_ID.rs1 == MEM_WB.rd) {IF_ID.rs1_value = MEM_WB.rd_value; ID_rs1_forwarding = 1;}
		if(IF_ID.rs2 && IF_ID.rs2 == MEM_WB.rd) {IF_ID.rs2_value = MEM_WB.rd_value; ID_rs2_forwarding = 1;}
	}*/
	// WB_rd = MEM_WB.rd;
	// WB_rd_value = MEM_WB.rd_value;
	if(EX_busy){
		if(ID_EX.rs1 && ID_EX.rs1 == MEM_WB.rd) ID_EX.rs1_value = MEM_WB.rd_value;
		if(ID_EX.rs2 && ID_EX.rs2 == MEM_WB.rd) ID_EX.rs2_value = MEM_WB.rd_value;
	}
	switch(MEM_WB.ins_type){
		case BEQ: case BNE: case BLT: case BGE: case BLTU: case BGEU: case SB: case SH: case SW:
			break;
		default:
			if(MEM_WB.rd) reg[MEM_WB.rd] = MEM_WB.rd_value;
			break;
	}
	// puts("-----WB-End-----");
	WB_busy = 0;
}
void work(){
	pc = 0;
	while(IF_ID.ins_code != 0xFF00513 || WB_busy || MEM_busy || EX_busy || (!ID_busy)){
		// puts("-----Pipe-Begin-----");
		write_back();
		memory_access();
		execute();
		instruction_decode();
		instruction_fetch();
		// puts("-----Pipe-End-----");
	}
	cout << (reg[10] & 255) << endl;
	// printf("%0.8f\n", (double)pre_right/pre_tot);
}
int main(){
	//freopen("b.data", "r", stdin);
	//freopen("2.out", "w", stdout);
	init();
	work();
	return 0;
}