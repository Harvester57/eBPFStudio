#include "pch.h"
#include "eBPF.h"
#include <bpf\bpf.h>
#include <io.h>

extern "C" int __cdecl _free_osfhnd(int const fh);

bool LocalClose(int fd) {
#ifdef _DEBUG
    return 0 == _close(fd);
#else
    // BUG: this fails in release build (_close crashes)
    return _free_osfhnd(fd);
#endif
}

std::vector<BpfProgram> BPF::EnumPrograms() {
    uint32_t id = 0;
    std::vector<BpfProgram> programs;
    programs.reserve(8);

    for (;;) {
        auto err = bpf_prog_get_next_id(id, &id);
        if (err)
            break;

        auto fd = bpf_prog_get_fd_by_id(id);
        if (fd < 0)
            break;

        bpf_prog_info info{};
        uint32_t size = sizeof(info);
        err = bpf_obj_get_info_by_fd(fd, &info, &size);

        ebpf_execution_type_t type;
        const char* filename, * section;
        err = ebpf_program_query_info(fd, &type, &filename, &section);
        LocalClose(fd);

        if (err)
            break;

        BpfProgram p;
        p.Id = info.id;
        p.Name = info.name;
        p.LinkCount = info.link_count;
        p.Type = (BpfProgramType)info.type;
        p.UuidType = info.type_uuid;
        p.MapCount = info.nr_map_ids;
        p.PinnedPathCount = info.pinned_path_count;
        if (err == 0) {
            p.ExecutionType = (BpfExecutionType)type;
            p.Section = section;
            p.FileName = filename;
        }
        ebpf_free_string(filename);
        ebpf_free_string(section);

        programs.push_back(std::move(p));
    }
    return programs;
}

std::vector<BpfMap> BPF::EnumMaps() {
    uint32_t id = 0;
    std::vector<BpfMap> maps;
    maps.reserve(8);
    for (;;) {
        auto err = bpf_map_get_next_id(id, &id);
        if (err)
            break;

        auto fd = bpf_map_get_fd_by_id(id);
        if (fd < 0)
            break;

        bpf_map_info info{};
        uint32_t size = sizeof(info);
        err = bpf_obj_get_info_by_fd(fd, &info, &size);
        LocalClose(fd);

        if (err)
            break;

        BpfMap map;
        map.Name = info.name;
        map.Id = info.id;
        map.InnerMapId = info.inner_map_id;
        map.Flags = info.map_flags;
        map.Type = (BpfMapType)info.type;
        map.KeySize = info.key_size;
        map.ValueSize = info.value_size;
        map.MaxEntries = info.max_entries;
        map.PinnedPathCount = info.pinned_path_count;

        maps.push_back(std::move(map));
    }
    return maps;
}

std::vector<BpfLink> BPF::EnumLinks() {
    uint32_t id = 0;
    std::vector<BpfLink> links;
    links.reserve(8);

    for (;;) {
        auto err = bpf_link_get_next_id(id, &id);
        if (err) break;

        auto fd = bpf_link_get_fd_by_id(id);
        if (fd < 0) break;

        bpf_link_info info{};
        uint32_t size = sizeof(info);
        err = bpf_obj_get_info_by_fd(fd, &info, &size);
        LocalClose(fd);
        if (err) break;

        BpfLink link;
        link.Id = info.id;
        link.Type = (BpfLinkType)info.type;
        link.CGroupId = info.cgroup.cgroup_id;
        link.AttachType = (BpfAttachType)info.attach_type;
        link.ProgramId = info.prog_id;
        link.ProgramTypeUuid = info.program_type_uuid;
        link.AttachTypeUuid = info.attach_type_uuid;
        
        links.push_back(std::move(link));
    }
    return links;
}

std::vector<BpfMapItem> BPF::GetMapData(uint32_t id) {
    auto fd = bpf_map_get_fd_by_id(id);
    if (fd < 0)
        return {};
    
    bpf_map_info info{};
    uint32_t size = sizeof(info);
    auto err = bpf_obj_get_info_by_fd(fd, &info, &size);
    if (err)
        return {};

    void const* keystart = nullptr;
    std::vector<BpfMapItem> data;
    data.reserve(info.max_entries);
    auto key = std::vector<uint8_t>(info.key_size);
    auto value = std::vector<uint8_t>(info.value_size);

    uint32_t index = 0;
    for (;;) {
        err = bpf_map_get_next_key(fd, keystart, key.data());
        if (err)
            break;

        err = bpf_map_lookup_elem(fd, key.data(), value.data());
        if (err)
            break;

        keystart = key.data();
        BpfMapItem item{ index++, std::make_unique<uint8_t[]>(info.key_size), std::make_unique<uint8_t[]>(info.value_size) };
        memcpy(item.Key.get(), key.data(), key.size());
        memcpy(item.Value.get(), value.data(), value.size());
        data.push_back(std::move(item));
    }

    LocalClose(fd);
    return data;
}

bool BpfMap::IsPerCpu() const {
    return Type == BpfMapType::LruPerCpuHash || Type == BpfMapType::PerCpuArray || Type == BpfMapType::PerCpuHash;
}
