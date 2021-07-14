use crate::cpu::{CPU, ops};
use crate::memory::Memory;
use std::convert::TryInto;

impl CPU {
    pub(in super::super) fn op_ret(&mut self, memory: &mut Memory) -> ops::Result {
        if self.opts.print_op {
            println!("{:03}\tRET", self.meta.steps);
        }

        let bytes = memory.get_bytes(self.sp, 4)
            .ok_or("load: cannot get memory")?
            .as_slice().try_into().expect("load: did not read 4 bytes");

        self.sp += 4;
        self.pc = u32::from_be_bytes(bytes);

        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use crate::cpu::Op;

    use super::*;
    use super::super::super::vmlib::MIN_RAM_SIZE;

    #[test]
    fn test_ret() {
        let mut memory = Memory::new(MIN_RAM_SIZE as u32, vec![]);
        let _ = memory.set_bytes(4, &[
            Op::Call.bytecode(), 0x00, 0x00, 0x00, 0x06,
            Op::Halt.bytecode(),
            Op::MovRI.bytecode(), 0x00, 0x00, 0x00, 0x00, 0xff,
            Op::Ret.bytecode(),
            Op::Panic.bytecode(),
        ]);

        let mut cpu = CPU::new();

        cpu.sp = 4;
        cpu.cs = cpu.sp;
        cpu.pc = cpu.cs;
        cpu.start();

        while cpu.step(&mut memory) {}

        assert_eq!(cpu.registers[0], 0x000000ff, "r0 {} != 0x000000ff", cpu.registers[0]);
        assert_eq!(4, cpu.sp, "sp {} != 4", cpu.sp);
    }
}