use crate::cpu::{CPU, ops};
use crate::memory::Memory;

impl CPU {
    pub(in super::super) fn op_mov_ri(&mut self, memory: &mut Memory) -> ops::Result {
        let r = self.fetch_register(memory, &Self::is_general_purpose_register)
            .ok_or("mov_ri: cannot fetch r")?;

        let w = self.fetch_word(memory)
            .ok_or("mov_ri: cannot fetch w" )? as i32;

        self.registers[r] = w;
        self.update_flags(self.registers[r]);

        if self.opts.print_op {
            println!("{:03}\tMOV  r{}, {:#010x}", self.meta.steps, r, w);
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
    fn test_mov_ri() {
        let mut memory = Memory::new(MIN_RAM_SIZE as u32, vec![]);
        let _ = memory.set_bytes(0, &[
            Op::MovRW.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x01,
        ]);

        let mut cpu = CPU::new();
        cpu.pc = 0;
        cpu.start();

        cpu.step(&mut memory);

        assert_eq!(cpu.registers[0], 1);
        assert_eq!(cpu.flags.zero, false);
        assert_eq!(cpu.flags.negative, false);
    }

    #[test]
    fn test_mov_ri_zero() {
        let mut memory = Memory::new(MIN_RAM_SIZE as u32, vec![]);
        let _ = memory.set_bytes(0, &[
            Op::MovRW.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x00,
        ]);

        let mut cpu = CPU::new();
        cpu.registers[0] = 1;
        cpu.pc = 0;
        cpu.start();

        cpu.step(&mut memory);

        assert_eq!(cpu.registers[0], 0);
        assert_eq!(cpu.flags.zero, true);
        assert_eq!(cpu.flags.negative, false);
    }
}