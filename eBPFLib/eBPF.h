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

enum class BpfMapType {
    Unspecified = 0,
    Hash = 1,
    Array = 2, 
    ProgramsArray = 3,
    PerCpuHash = 4,
    PerCpuArray = 5,
    HashOfMaps = 6,
    ArrayOfMaps = 7,
    LruHash = 8,
    LpmTrie = 9,
    Queue = 10,     
    LruPerCpuHash = 11,
    Stack = 12,
    RingBuffer = 13
};

struct BpfMap {
    std::string Name;
    uint32_t Id;
    BpfMapType Type;
    uint32_t KeySize;
    uint32_t ValueSize;
    uint32_t MaxEntries;
    uint32_t Flags;

    // Windows-specific fields.
    uint32_t InnerMapId;
    uint32_t PinnedPathCount; 
};

class BPF {
public:
	static std::vector<BpfProgram> EnumPrograms();
    static std::vector<BpfMap> EnumMaps();
};

