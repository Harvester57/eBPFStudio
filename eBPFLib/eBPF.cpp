#include "pch.h"
#include "eBPF.h"
#include <ebpf_api.h>
#include <bpf/bpf.h>
#include <io.h>
#include <wil\resource.h>

namespace {
	const PCWSTR ServiceNames[] = {
		L"netebpfext", L"ebpfcore", L"ebpfsvc",
	};

	bool LocalClose(int fd) {
		return _close(fd) == 0;
	}

	bool StartService(PCWSTR name) {
		wil::unique_schandle hScm(::OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS));
		if (!hScm)
			return false;

		wil::unique_schandle hService(::OpenService(hScm.get(), name, SERVICE_ALL_ACCESS));
		if (!hService)
			return false;

		return ::StartService(hService.get(), 0, nullptr);
	}

	bool StopService(PCWSTR name) {
		wil::unique_schandle hScm(::OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS));
		if (!hScm)
			return false;

		wil::unique_schandle hService(::OpenService(hScm.get(), name, SERVICE_ALL_ACCESS));
		if (!hService)
			return false;

		SERVICE_STATUS status;
		if (!::ControlService(hService.get(), SERVICE_CONTROL_STOP, &status))
			return false;

		return true;
	}
}


std::vector<BpfProgram> BpfSystem::EnumPrograms() {
	uint32_t id = 0;
	std::vector<BpfProgram> programs;
	programs.reserve(8);

	for (;;) {
		auto err = bpf_prog_get_next_id(id, &id);
		if (err)
			break;

		auto p = GetProgramById(id);
		if (!p)
			break;
		programs.push_back(std::move(*p));
	}
	return programs;
}

std::unique_ptr<BpfProgram> BpfSystem::GetProgramById(uint32_t id) {
	auto fd = bpf_prog_get_fd_by_id(id);
	if (fd < 0)
		return nullptr;

	bpf_prog_info info{};
	uint32_t size = sizeof(info);
	auto p = std::make_unique<BpfProgram>();
	info.nr_map_ids = 32;
	p->MapIds.resize(info.nr_map_ids);
	info.map_ids = (uintptr_t)p->MapIds.data();
	auto err = bpf_obj_get_info_by_fd(fd, &info, &size);
	if (err) {
		LocalClose(fd);
		return nullptr;
	}

	p->MapIds.resize(info.nr_map_ids);
	p->Id = info.id;
	p->Name = info.name;
	p->LinkCount = info.link_count;
	p->Type = (BpfProgramType)info.type;
	p->UuidType = info.type_uuid;
	p->MapCount = info.nr_map_ids;
	p->PinnedPathCount = info.pinned_path_count;

	ebpf_execution_type_t type;
	const char* filename, * section;
	err = ebpf_program_query_info(fd, &type, &filename, &section);
	LocalClose(fd);

	if (err == 0) {
		p->ExecutionType = (BpfExecutionType)type;
		p->Section = section;
		p->FileName = filename;
	}
	ebpf_free_string(filename);
	ebpf_free_string(section);

	return p;
}

std::vector<BpfMap> BpfSystem::EnumMaps() {
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

std::vector<BpfLink> BpfSystem::EnumLinks() {
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

std::vector<BpfMapItem> BpfSystem::GetMapData(uint32_t id) {
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

std::vector<BpfProgramEx> BpfSystem::EnumProgramsInFile(const char* path, std::string* errMsg) {
	ebpf_api_program_info_t* info;
	const char* msg;
	if (ebpf_enumerate_programs(path, true, &info, &msg) < 0) {
		if (errMsg)
			*errMsg = msg;
		return {};
	}

	std::vector<BpfProgramEx> programs;
	auto start = info;
	while (info) {
		BpfProgramEx p;
		p.FileName = path;
		p.Name = info->program_name;
		p.SectionName = info->section_name;
		p.Data.resize(info->raw_data_size);
		memcpy(p.Data.data(), info->raw_data, info->raw_data_size);
		p.Type = info->program_type;
		p.ExpectedAttachType = info->expected_attach_type;
		p.OffsetInSection = (uint32_t)info->offset_in_section;
		auto stats = info->stats;
		while (stats) {
			BpfStat stat{ stats->key, stats->value };
			p.Stats.push_back(std::move(stat));
			stats = stats->next;
		}
		programs.push_back(std::move(p));
		info = info->next;
	}

	ebpf_free_programs(start);

	return programs;
}

std::string BpfSystem::DisassembleProgram(BpfProgramEx const& p) {
	const char* text = nullptr;
	ebpf_api_elf_disassemble_program(p.FileName.c_str(), p.SectionName.c_str(), p.Name.c_str(), &text, &text);
	std::string result(text);
	ebpf_free_string(text);

	return result;
}

const char* BpfSystem::GetProgramTypeName(GUID const& type) {
	return ebpf_get_program_type_name(&type);
}

const char* BpfSystem::GetAttachTypeName(GUID const& type) {
	return ebpf_get_attach_type_name(&type);
}

int BpfSystem::LoadProgramsFromFile(char const* path, const char* pinPath, BpfExecutionType exeType) {
	auto obj = bpf_object__open(path);
	if (!obj)
		return 0;

	int count = 0;
	bpf_program* program = nullptr;
	do {
		if (auto error = ebpf_object_set_execution_type(obj, (ebpf_execution_type_t)exeType); error != EBPF_SUCCESS)
			break;

		program = bpf_object__next_program(obj, program);
		if (!program)
			break;

		if (auto error = bpf_object__load(obj); error < 0) {
			printf("Failed to load program (%d)\n", error);
			size_t log_buffer_size;
			s_LastErrorText = bpf_program__log_buf(program, &log_buffer_size);
			break;
		}

		bpf_link* link;
		if (auto err = ebpf_program_attach(program, nullptr, nullptr, 0, &link); err != ERROR_SUCCESS)
			break;

		auto bs = strrchr(path, '\\');
		auto name = std::string(bs ? bs + 1 : path) + "::" + bpf_program__name(program);

		//auto fd = bpf_program__fd(program);

		//if (bpf_link__pin(link, name.c_str()) < 0) {
		//	fprintf(stderr, "Failed to pin eBPF link: %d\n", errno);
		//	return 1;
		//}

		if (bpf_program__pin(program, pinPath ? pinPath : path) < 0)
			break;
		count++;
	} while (false);
	if (count > 0)
		s_LastErrorText.clear();

	bpf_object__close(obj);
	return count;
}

bool BpfSystem::UnloadProgram(const char* name, const char* pinPath, const char* filePath) {
	auto obj = bpf_object__open(filePath);
	if (!obj)
		return false;

	bpf_program* prog = nullptr;
	do {
		prog = bpf_object__next_program(obj, prog);
		if (prog && strcmp(bpf_program__name(prog), name) == 0)
			break;
	} while (prog);
	if (prog == nullptr)
		return false;

	auto ok = false;
	auto fd = bpf_program__fd(prog);
	if (fd > 0) {
		ok = bpf_prog_detach(fd, bpf_attach_type::BPF_ATTACH_TYPE_UNSPEC);
		LocalClose(fd);
	}
	bpf_object__close(obj);

	return ok;
}

bool BpfSystem::UnloadProgram(uint32_t id) {
	auto fd = bpf_prog_get_fd_by_id(id);
	if (fd < 0)
		return false;

	auto ok = bpf_prog_detach(fd, bpf_attach_type::BPF_ATTACH_TYPE_UNSPEC) == 0;
	LocalClose(fd);
	return ok;
}

bool BpfSystem::DetachLink(uint32_t linkId) {
	uint32_t id = 0;
	bool success = false;
	for(;;) {
		auto err = bpf_link_get_next_id(id, &id);
		if (err)
			return false;

		if (id != linkId)
			continue;

		auto fd = bpf_link_get_fd_by_id(id);
		if (fd < 0)
			return false;

		success = bpf_link_detach(fd) == 0;
		break;
	}

	return success;
}

std::string const& BpfSystem::GetLastErrorText() {
	return s_LastErrorText;
}

bool BpfSystem::StartServices() {
	for (auto& name : ServiceNames)
		StartService(name);
	return true;
}

bool BpfSystem::StopServices() {
	for (auto& name : ServiceNames)
		StopService(name);
	return true;
}

bool BpfSystem::RestartServices() {
	StopServices();
	return StartServices();
}

std::vector<BpfServiceStatus> BpfSystem::GetServicesStatus() {
	wil::unique_schandle hScm(::OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS));
	if (!hScm)
		return {};

	SERVICE_STATUS status;
	std::vector<BpfServiceStatus> services;
	for (auto name : ServiceNames) {
		wil::unique_schandle hService(::OpenService(hScm.get(), name, SERVICE_QUERY_STATUS));
		if (hService) {
			if (::QueryServiceStatus(hService.get(), &status)) {
				BpfServiceStatus ss;
				ss.Name = name;
				ss.Running = status.dwCurrentState = SERVICE_RUNNING;
				services.push_back(std::move(ss));
			}
		}
	}
	return services;
}

bool BpfMap::IsPerCpu() const {
	return Type == BpfMapType::LruPerCpuHash || Type == BpfMapType::PerCpuArray || Type == BpfMapType::PerCpuHash;
}

BpfObject::BpfObject(bpf_object* obj) : m_Object(obj) {
}

BpfObject::operator bpf_object* () const {
	return m_Object;
}

BpfObject::~BpfObject() {
	if (m_Object)
		bpf_object__close(m_Object);
}

