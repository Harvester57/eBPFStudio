#include "pch.h"
#include "eBPF.h"
#include <bpf\bpf.h>

std::vector<BpfProgram> BPF::EnumPrograms(bool includeMapInfo) {
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

        programs.push_back(std::move(p));
    }
    return programs;
}
