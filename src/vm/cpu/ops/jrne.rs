use crate::cpu::{CPU, ops};
use crate::memory::Memory;

impl CPU {
    pub(in super::super) fn op_jrne(&mut self, memory: &mut Memory) -> ops::Result {
        let target = self.fetch_word(memory)
            .ok_or("jrne: cannot fetch target")? + self.cs;

        if !self.flags.zero {
            self.pc = target;
            if self.opts.print_op {
                println!("{:03}\tJRNE {:#010x}", self.meta.steps, target);
            }
        } else {
            if self.opts.print_op {
                println!("{:03}\tNOP  (JRNE {:#010x})", self.meta.steps, target);
            }
        }

        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use crate::cpu::Op;

    use super::*;
    use super::super::super::vmlib::MIN_RAM_SIZE;

    #[test]
    fn test_jrne_zero() {
        let mut memory = Memory::new(MIN_RAM_SIZE as u32, vec![]);
        let _ = memory.set_bytes(0, &[
            Op::Jrne.bytecode(), 0x00, 0x00, 0x00, 0x0C,
            Op::MovRI.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x01,
            Op::Halt.bytecode(),
            Op::MovRI.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x02,
            Op::Halt.bytecode()
        ]);

        let mut cpu = CPU::new();
        cpu.flags.zero = false;
        cpu.cs = 0;
        cpu.pc = 0;
        cpu.start();

        while cpu.step(&mut memory) {}

        assert_eq!(cpu.registers[0], 2);
    }

    #[test]
    fn test_jrne_nonzero() {
        let mut memory = Memory::new(MIN_RAM_SIZE as u32, vec![]);
        let _ = memory.set_bytes(0, &[
            Op::Jrne.bytecode(), 0x00, 0x00, 0x00, 0x0C,
            Op::MovRI.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x01,
            Op::Halt.bytecode(),
            Op::MovRI.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x02,
            Op::Halt.bytecode()
        ]);

        let mut cpu = CPU::new();
        cpu.flags.zero = true;
        cpu.cs = 0;
        cpu.pc = 0;
        cpu.start();

        while cpu.step(&mut memory) {}

        assert_eq!(cpu.registers[0], 1);
    }
}