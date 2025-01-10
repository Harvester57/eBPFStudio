#pragma once

#include <eBPF.h>

struct StringHelper abstract final{
	static PCWSTR ProgramTypeToString(BpfProgramType type);
	static PCWSTR MapTypeToString(BpfMapType type);
};

