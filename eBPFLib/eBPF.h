#pragma once

#include <string>
#include <vector>
#include <memory>
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

enum class BpfExecutionType {
    Any,       
    JIT,       
    Interpret, 
    Native     
};

enum class BpfAttachType {
    Unspecified, 
    Xdp,
    Bind,
    CGroupInet4Connect,
    CGroupInet6Connect,
    CGroupInet4RecvAccept,
    CGroupInet6RecvAccept,
    CGroupSockOps,
    Sample,
    XdpTest,
};

struct BpfProgram {
	std::string Name;
	int Id;
	uint32_t LinkCount;
	BpfProgramType Type;
    GUID UuidType;
    uint32_t MapCount;
    uint32_t PinnedPathCount;
    BpfExecutionType ExecutionType;
    std::string FileName;
    std::string Section;
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

    bool IsPerCpu() const;
};

enum class BpfLinkType {
    Unspecified,
    Plain,  
    CGroup, 
    Xdp,
    Max
};

struct BpfLink {
    uint32_t Id;
    uint32_t ProgramId;
    BpfLinkType Type;
    BpfAttachType AttachType;
    GUID AttachTypeUuid;
    GUID ProgramTypeUuid;
    union {
        struct {
            uint32_t Ifindex;
        };
        struct {
            uint64_t CGroupId;
        };
        uint8_t AttachData;
    };
};

struct BpfMapItem {
    uint32_t Index;
    std::unique_ptr<uint8_t[]> Key;
    std::unique_ptr<uint8_t[]> Value;
};

struct BpfStat {
    std::string Key;
    int Value;
};

struct BpfProgramEx {
    std::string FileName;
    std::string Name;
    std::string SectionName;
    GUID Type;
    GUID ExpectedAttachType;
    std::vector<uint8_t> Data;
    uint32_t OffsetInSection;
    std::vector<BpfStat> Stats;
};

class BpfSystem {
public:
	static std::vector<BpfProgram> EnumPrograms();
    static std::unique_ptr<BpfProgram> GetProgramById(uint32_t id);
    static std::vector<BpfMap> EnumMaps();
    static std::vector<BpfLink> EnumLinks();
    static std::vector<BpfMapItem> GetMapData(uint32_t id);

    static std::vector<BpfProgramEx> EnumProgramsInFile(const char* path, std::string* errMsg = nullptr);
    static std::string DisassembleProgram(BpfProgramEx const& p);
};

