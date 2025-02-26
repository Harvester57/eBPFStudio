#include "pch.h"
#include "StringHelper.h"
#include <cassert>
#include <format>

PCWSTR StringHelper::ProgramTypeToString(BpfProgramType type) {
    switch (type) {
        case BpfProgramType::Unspecified: return L"Unspecified";
        case BpfProgramType::Bind: return L"Bind";
        case BpfProgramType::Sample: return L"Sample";
        case BpfProgramType::SockOps: return L"Sock Ops";
        case BpfProgramType::Xdp: return L"XDP";
        case BpfProgramType::XdpTest: return L"XDP Test";
        case BpfProgramType::CgroupSockAddr: return L"CGroup Sock Address";
    }
    return L"(Unknown)";
}

PCWSTR StringHelper::MapTypeToString(BpfMapType type) {
    switch (type) {
        case BpfMapType::Unspecified: return L"Unspecified";
        case BpfMapType::Hash: return L"Hash";
        case BpfMapType::Array: return L"Array";
        case BpfMapType::ProgramsArray: return L"Programs Array";
        case BpfMapType::PerCpuHash: return L"Per-CPU Hash";
        case BpfMapType::PerCpuArray: return L"Per-CPU Array";
        case BpfMapType::HashOfMaps: return L"Hash of Maps";
        case BpfMapType::ArrayOfMaps: return L"Array of Maps";
        case BpfMapType::LruHash: return L"LRU Hash";
        case BpfMapType::LpmTrie: return L"LPM Trie";
        case BpfMapType::Queue: return L"Queue";
        case BpfMapType::LruPerCpuHash: return L"LRU Per CPU Hash";
        case BpfMapType::Stack: return L"Stack";
        case BpfMapType::RingBuffer: return L"Ring Buffer";
    }
    return L"(Unknown)";
}

std::wstring StringHelper::FormatNumber(void const* p, uint32_t size) {
    assert(size <= 8);
    switch (size) {
        case 8: return std::format(L"0x{:X}", *(uint64_t*)p);
        case 4: return std::format(L"0x{:X}", *(uint32_t*)p);
        case 2: return std::format(L"0x{:X}", *(uint16_t*)p);
        case 1: return std::format(L"0x{:X}", *(uint64_t*)p);
    }

    return L"";
}

std::wstring StringHelper::BufferToHexString(void const* p, uint32_t size) {
    std::wstring text;
    size = std::min(64u, size);
    for (uint32_t i = 0; i < size; i++)
        text += std::format(L"{:02X} ", *((uint8_t const*)p + i));
    return text;
}

std::string StringHelper::BufferToCharString(void const* p, uint32_t size) {
    std::string text;
    size = std::min(64u, size);
    for (uint32_t i = 0; i < size; i++) {
        auto ch = *((const char*)p + i);
        text += isprint(ch) ? ch : '.';
    }
    return text;
}

PCWSTR StringHelper::ExeutionTypeToString(BpfExecutionType type) {
    switch (type) {
        case BpfExecutionType::Any: return L"Any";
        case BpfExecutionType::JIT: return L"JIT";
        case BpfExecutionType::Interpret: return L"Interpret";
        case BpfExecutionType::Native: return L"Native";
    }
    return L"(Unknown)";
}

PCWSTR StringHelper::LinkTypeToString(BpfLinkType type) {
    switch (type) {
        case BpfLinkType::Unspecified: return L"Unspecified";
        case BpfLinkType::Plain: return L"Plain";
        case BpfLinkType::Xdp: return L"XDP";
        case BpfLinkType::CGroup: return L"CGroup";
    }
    return L"(Unknown)";
}

PCWSTR StringHelper::AttachTypeToString(BpfAttachType type) {
    switch (type) {
        case BpfAttachType::Xdp: return L"XDP";
        case BpfAttachType::Unspecified: return L"Unspecified";
        case BpfAttachType::CGroupInet4Connect: return L"CGroup Inet4 Connect";
        case BpfAttachType::CGroupInet6Connect: return L"CGroup Inet6 Connect";
        case BpfAttachType::CGroupInet4RecvAccept: return L"CGroup Inet4 Recv Accept";
        case BpfAttachType::CGroupInet6RecvAccept: return L"CGroup Inet6 Recv Accept";
        case BpfAttachType::CGroupSockOps: return L"CGroup Socket Ops";
        case BpfAttachType::XdpTest: return L"XDP Test";
        case BpfAttachType::Bind: return L"Bind";
        case BpfAttachType::Sample: return L"Sample";
    }
    return L"(Unknown)";
}

PCWSTR StringHelper::ObjectTypeToString(BpfObjectType type) {
    switch (type) {
        case BpfObjectType::Program: return L"Program";
        case BpfObjectType::Map: return L"Map";
        case BpfObjectType::Link: return L"Link";
    }
    return L"Unknown";
}

CString StringHelper::GuidToString(GUID const& guid) {
    WCHAR text[64];
    ::StringFromGUID2(guid, text, _countof(text));
    return text;
}
