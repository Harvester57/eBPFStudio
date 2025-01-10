#pragma once

#include <string>
#include <vector>
#include <guiddef.h>

enum class BpfProgramType {
    Unspecified,
    Xdp,
    Bind,
    CgroupSockAddr,
    SockOps,
    XdpTest = 998,
    Sample = 999,
};

struct BpfProgram {
	std::string Name;
	int Id;
	uint32_t LinkCount;
	BpfProgramType Type;
    GUID UuidType;
    uint32_t MapCount;
    uint32_t PinnedPathCount;
    std::vector<uint32_t> MapIds;
};

class BPF {
public:
	static std::vector<BpfProgram> EnumPrograms(bool includeMapInfo = false);
};

