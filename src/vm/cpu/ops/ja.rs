use crate::cpu::{CPU, ops};
use crate::memory::Memory;

impl CPU {
    pub(in super::super) fn op_ja(&mut self, memory: &mut Memory) -> ops::Result {
        let r0 = self.fetch_register(memory, &Self::is_general_purpose_register)
            .ok_or("ja: cannot fetch r0")?;

        let r1 = self.fetch_register(memory, &Self::is_general_purpose_register)
            .ok_or("ja: cannot fetch r1")?;

        let target = self.registers[r0] as u32 + self.registers[r1] as u32;

        if self.opts.print_op {
            println!("{:03}\tJA   r{}, r{} ({:#010x})", self.meta.steps, r0, r1, target);
        }

        self.pc = target;

        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use crate::cpu::Op;

    use super::*;
    use super::super::super::vmlib::MIN_RAM_SIZE;

    #[test]
    fn test_ja() {
        let mut memory = Memory::new(MIN_RAM_SIZE as u32, vec![]);
        let _ = memory.set_bytes(0, &[
            Op::Ja.bytecode(), 0x00, 0x01,
            Op::MovRW.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x01,
            Op::Halt.bytecode(),
            Op::MovRW.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x02,
            Op::Halt.bytecode()
        ]);

        let mut cpu = CPU::new();
        cpu.registers[0] = 0;
        cpu.registers[1] = 10;
        cpu.cs = 1;
        cpu.pc = 0;
        cpu.start();

        while cpu.step(&mut memory) {}

        assert_eq!(cpu.registers[0], 2);
    }
}