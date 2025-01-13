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
    for (uint32_t i = 0; i < size; i++)
        text += std::format(L"{:02X} ", *((uint8_t const*)p + i));
    return text;
}
