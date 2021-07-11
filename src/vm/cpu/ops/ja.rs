use crate::cpu::{CPU, ops};
use crate::memory::Memory;

impl CPU {
    pub(in super::super) fn op_ja(&mut self, memory: &mut Memory) -> ops::Result {
        let r0 = match self.fetch_1byte(memory) {
            None => return Err("Cannot fetch r0"),
            Some(byte) => byte,
        } as usize;
        let r1 = match self.fetch_1byte(memory) {
            None => return Err("Cannot fetch r1"),
            Some(byte) => byte,
        } as usize;

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
            Op::MovRI.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x01,
            Op::Halt.bytecode(),
            Op::MovRI.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x02,
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