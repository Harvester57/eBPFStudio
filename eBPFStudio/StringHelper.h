#pragma once

#include <eBPF.h>

struct StringHelper abstract final{
	static PCWSTR ProgramTypeToString(BpfProgramType type);
	static PCWSTR MapTypeToString(BpfMapType type);
	static std::wstring FormatNumber(void const* p, uint32_t size);
	static std::wstring BufferToHexString(void const* p, uint32_t size);
};

