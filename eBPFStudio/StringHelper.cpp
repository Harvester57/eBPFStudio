#include "pch.h"
#include "StringHelper.h"

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
