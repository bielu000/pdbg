#include "stdafx.h"
#include <iostream>
#include "Disassembler.h"
#include "udis86.h"
#include <vector>
#include <memory>
#include "ErrorCodes.h"

bool Disassembler::disassembly(HANDLE hProcess, HANDLE hThread, DWORD address, unsigned int instructionsAmount)
{
	CONTEXT ctx;
	memset(&ctx, 0, sizeof(ctx));

	ctx.ContextFlags = CONTEXT_CONTROL;

	if (!GetThreadContext(hThread, &ctx)) {
		return false;
	}

	SIZE_T bytes_read;
	BYTE buffer[4096];

	if (!ReadProcessMemory(hProcess, (LPBYTE)address, buffer, sizeof(buffer), &bytes_read)) {
		return false;
	}
	/*printf("Address : %#010x\n", address);
	std::cout << "Disassembled code: " << std::endl;*/

	ud_t ud_obj;
	ud_init(&ud_obj);
	ud_set_input_buffer(&ud_obj, buffer, sizeof(buffer));
	ud_set_mode(&ud_obj, 64);
	ud_set_syntax(&ud_obj, UD_SYN_INTEL);

	int counter = 0;
	unsigned int instructionLenght = 0;
	DWORD instrAddr = address;

	_disassembledInstructions.clear();

	while ((instructionLenght = ud_disassemble(&ud_obj)) != 0) {

	    //printf("\t RAW PRINT : %d 0x%08x: %s\n", instructionLenght, instrAddr, ud_insn_asm(&ud_obj));

		_disassembledInstructions.emplace_back(std::make_shared<Instruction>(instrAddr, ud_insn_asm(&ud_obj)));

		counter++;
		instrAddr += instructionLenght;

		if (counter == instructionsAmount) {
			break;
		}
	}


	return true;
}
