#include "stdafx.h"
#include <iostream>
#include "Disassembler.h"
#include "udis86.h"
#include <vector>
#include <memory>

std::vector<std::shared_ptr<Instruction>> Disassembler::disassembly(HANDLE hProcess, HANDLE hThread, DWORD address, unsigned int instructionsAmount)
{
	CONTEXT ctx;
	memset(&ctx, 0, sizeof(ctx));

	ctx.ContextFlags = CONTEXT_CONTROL;

	if (!GetThreadContext(hThread, &ctx)) {
		std::cout << "Cannot get context -> disassembly." << std::endl;
		
		return;
	}

	SIZE_T bytes_read;
	BYTE buffer[4096];

	if (!ReadProcessMemory(hProcess, (LPBYTE)address, buffer, sizeof(buffer), &bytes_read)) {
		std::cout << "Cannot read process memory -> dissasembly." << std::endl;

		return;
	}
	printf("Address : %#010x\n", address);
	std::cout << "Disassembled code: " << std::endl;
	
	std::vector<std::shared_ptr<Instruction>> instructions;

	ud_t ud_obj;
	ud_init(&ud_obj);
	ud_set_input_buffer(&ud_obj, buffer, sizeof(buffer));
	ud_set_mode(&ud_obj, 64);
	ud_set_syntax(&ud_obj, UD_SYN_INTEL);

	int counter = 0;
	unsigned int instructionLenght = 0;
	DWORD instrAddr = address;

	while ((instructionLenght = ud_disassemble(&ud_obj)) != 0) {

	    //printf("\t RAW PRINT : %d 0x%08x: %s\n", instructionLenght, instrAddr, ud_insn_asm(&ud_obj));

		instructions.emplace_back(std::make_shared<Instruction>(instrAddr, ud_insn_asm(&ud_obj)));

		counter++;
		instrAddr += instructionLenght;

		if (counter == instructionsAmount) {
			break;
		}
	}

	/*std::cout << "Dissasembling done!" << std::endl;
	std::cout << "Instructions: " << std::endl;

	for (auto &x : instructions)
	{
		std::cout << "0x" << std::hex << x->address() << " : " << x->code() << std::endl;
	}

	std::cout << "----------------END---------------" << std::endl;*/

	return instructions;
}
