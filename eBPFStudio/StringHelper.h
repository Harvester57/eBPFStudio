#pragma once

#include <eBPF.h>

struct StringHelper abstract final{
	static PCWSTR ProgramTypeToString(BpfProgramType type);
	static PCWSTR MapTypeToString(BpfMapType type);
	static std::wstring FormatNumber(void const* p, uint32_t size);
	static std::wstring BufferToHexString(void const* p, uint32_t size);
	static std::string BufferToCharString(void const* p, uint32_t size);

	static PCWSTR ExeutionTypeToString(BpfExecutionType type);
	static PCWSTR LinkTypeToString(BpfLinkType type);
	static PCWSTR AttachTypeToString(BpfAttachType type);
	static PCWSTR ObjectTypeToString(BpfObjectType type);

	static CString GuidToString(GUID const& guid);
	template<typename T>
	static CString VectorToString(std::vector<T> const& values) {
		CString result;
		for (auto& v : values)
			result += std::format(L"{}, ", v).c_str();
		return result.IsEmpty() ? result : result.Left(result.GetLength() - 2);
	}
};

