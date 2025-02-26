#pragma once

#include <string>
#include <vector>
#include <memory>
#include <guiddef.h>
#include <functional>

struct bpf_object;

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

struct BpfObject {
    BpfObject(bpf_object* obj);
    BpfObject(BpfObject const&) = delete;
    BpfObject& operator=(BpfObject const&) = delete;
    BpfObject(BpfObject&&) = default;
    BpfObject& operator=(BpfObject&&) = default;

    operator bpf_object* () const;
    ~BpfObject();

private:
    bpf_object* m_Object;
};

struct BpfServiceStatus {
    std::wstring Name;
    bool Running;
};

enum class BpfObjectType {
    Unknown,
    Map,
    Link,
    Program,
};

struct BpfPin {
    uint32_t Id;
    std::string Path;
    BpfObjectType ObjectType;
};

class BpfSystem {
public:
	static std::vector<BpfProgram> EnumPrograms();
    static std::unique_ptr<BpfProgram> GetProgramById(uint32_t id);
    static std::vector<BpfMap> EnumMaps();
    static std::vector<BpfLink> EnumLinks();
    static std::vector<BpfPin> EnumPins();

    static std::vector<BpfMapItem> GetMapData(uint32_t id);

    static std::vector<BpfProgramEx> EnumProgramsInFile(const char* path, std::string* errMsg = nullptr);
    static std::string DisassembleProgram(BpfProgramEx const& p);

    static const char* GetProgramTypeName(GUID const& type);
    static const char* GetAttachTypeName(GUID const& type);

    static bool Unpin(const char* path);
    static bool PinMap(uint32_t id, const char* path);

    static int LoadProgramsFromFile(char const* path, const char* pinPath = nullptr, BpfExecutionType type = BpfExecutionType::Any);
    static bool UnloadProgram(const char* name, const char* pinPath, const char* filePath);
    static bool UnloadProgram(uint32_t id);

    static bool DetachLink(uint32_t id);
    static std::string const& GetLastErrorText();

    static bool StartServices();
    static bool StopServices();
    static bool RestartServices();
    static std::vector<BpfServiceStatus> GetServicesStatus();

private:
    static int OpenLink(uint32_t id);

    inline static std::string s_LastErrorText;
};

