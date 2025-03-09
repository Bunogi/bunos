#include <bustd/stringview.hpp>
#include <kernel/elfreader.hpp>
#include <kernel/physicalmalloc.hpp>
#include <kernel/process.hpp>

namespace kernel {

Process::Process(bu::StringView s, pid_t pid) : m_pid(pid) {
  m_regs.reset();
  m_syscall_regs.reset();

  m_memory_mapping.mmu_base = allocate_physical_page();
  m_memory_mapping.update_kernel_mappings();

  // ERROR is that it doesnt work beacause the mappings were wrong. Also have to
  // allocate the kernel stack things

  m_regs.set_entry(kernel::elf::load(*this, s));
}

} // namespace kernel
