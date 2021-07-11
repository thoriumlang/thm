use crate::cpu::{CPU, ops};
use crate::memory::Memory;

impl CPU {
    pub(in super::super) fn op_jr(&mut self, memory:&mut Memory) -> ops::Result {
        let target = match self.fetch_4bytes(memory) {
            None => return Err("Cannot fetch target"),
            Some(bytes) => bytes
        } + self.cs;

        if self.opts.print_op {
            println!("{:03}\tJR   {:#010x}", self.meta.steps, target);
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
    fn test_jr() {
        let mut memory = Memory::new(MIN_RAM_SIZE as u32, vec![]);
        let _ = memory.set_bytes(0, &[
            Op::Jr.bytecode(), 0x00, 0x00, 0x00, 0x0c,
            Op::MovRI.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x01,
            Op::Halt.bytecode(),
            Op::MovRI.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x02,
            Op::Halt.bytecode()
        ]);

        let mut cpu = CPU::new();
        cpu.cs = 0;
        cpu.pc = 0;
        cpu.start();

        while cpu.step(&mut memory) {}

        assert_eq!(cpu.registers[0], 2);
    }
}