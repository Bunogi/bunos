#include <bustd/assert.hpp>
#include <kernel/interrupts.hpp>
#include <kernel/memory.hpp>
#include <kernel/panic.hpp>
#include <kernel/sched.hpp>
#include <kernel/task/x86.hpp>
#include <kernel/x86/interrupts.hpp>
#include <kernel/x86/memory.hpp>
#include <kernel/x86/pagemapguard.hpp>
#include <kernel/x86/pit.hpp>

extern "C" {
void x86_idle();
void x86_syscall_enter();
}

namespace kernel::task::x86 {

namespace {

auto enter_scheduler_handler(kernel::x86::InterruptFrame *frame) -> bool {
  Registers reg{};
  reg.reset();
  reg.set_entry(reinterpret_cast<void (*)()>(frame->eax));
  reg.insert_into_frame(frame);
  return true;
}

auto systick_handler(kernel::x86::InterruptFrame *frame) -> bool {
  sched::Scheduler::systick_event(frame);
  return true;
}

auto syscall_handler(kernel::x86::InterruptFrame *frame) -> bool {
  sched::Scheduler::syscall_event(frame);
  return true;
}

} // namespace

auto Registers::read_from_frame(const kernel::x86::InterruptFrame *frame)
    -> void {
  m_edi = frame->edi;
  m_esi = frame->esi;
  m_ebp = frame->ebp;

  if (frame->cs != 0x8) {
    kernel::panic_from_interrupt(frame, "Priviledge level change", false);
  } else {
    m_esp = reinterpret_cast<uintptr_t>(&frame->useresp);
  }

  m_ebx = frame->ebx;
  m_edx = frame->edx;
  m_ecx = frame->ecx;
  m_eax = frame->eax;
  m_eflags = frame->eflags;
  m_eip = frame->eip;
}

void Registers::insert_into_frame(kernel::x86::InterruptFrame *frame) {
  frame->edi = m_edi;
  frame->esi = m_esi;
  frame->ebp = m_ebp;
  frame->eip = m_eip;
  frame->ebx = m_ebx;
  frame->edx = m_edx;
  frame->ecx = m_ecx;
  frame->eax = m_eax;
  frame->eflags = m_eflags;
}

auto MemoryMapping::set_mapping() -> void {
  _x86_set_page_directory(mmu_base.get());
}

auto MemoryMapping::update_kernel_mappings() -> void {
  {
    const kernel::x86::PageMapGuard guard(mmu_base);
    memcpy(guard.mapped_address(), kernel::x86::kernel_page_directory,
           sizeof(kernel::x86::kernel_page_directory));
  }
}

auto enter_scheduler(const VirtualAddress &init_entry) -> void {
  interrupts::register_handler(0x80, syscall_handler);
  interrupts::register_handler(0x81, enter_scheduler_handler);
  // TODO: Default is 1KHz which is a little much...
  interrupts::register_handler(kernel::x86::pit::IRQ_NUM, systick_handler);
  kernel::x86::pit::initialize();

  const auto init = init_entry.ptr();
  asm volatile("movl %%eax, %0\n"
               "int $0x81"
               :
               : "r"(init)
               : "eax");
  UNREACHABLE();
}

auto setup_syscall_regs(const kernel::x86::InterruptFrame *const frame,
                        Registers &regs, const VirtualAddress stack_start,
                        const usize stack_size, Process *target_process)
    -> void {
  regs.read_from_frame(frame);
  regs.reset();

  regs.set_entry(x86_syscall_enter);
  constexpr auto stack_align = 16;
  regs.set_stack(
      reinterpret_cast<u8 *>(stack_start.get() + stack_size - stack_align));
  regs.set_syscall_target(target_process);
}
} // namespace kernel::task::x86
