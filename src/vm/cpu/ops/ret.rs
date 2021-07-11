use crate::cpu::{CPU, ops};
use crate::memory::Memory;

impl CPU {
    pub(in super::super) fn op_ret(&mut self, memory: &mut Memory) -> ops::Result {
        if self.opts.print_op {
            println!("{:03}\tRET", self.meta.steps);
        }

        // todo maybe mutualize code for ret/pop
        let mut target: u32 = 0;
        for i in 0..4 {
            target = target << 8;
            match memory.get(self.sp + i) {
                None => return Err("Cannot fetch 4 bytes"),
                Some(byte) => target |= byte as u32,
            }
        }
        self.sp += 4;

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