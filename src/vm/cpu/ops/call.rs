use crate::cpu::{CPU, ops};
use crate::memory::Memory;

impl CPU {
    pub(in super::super) fn op_call(&mut self, memory: &mut Memory) -> ops::Result {
        let target = match self.fetch_4bytes(memory) {
            None => return Err("Cannot fetch addr"),
            Some(byte) => byte
        } + self.cs;

        if self.opts.print_op {
            println!("{:03}\tCALL {:#010x}", self.meta.steps, target);
        }

        // todo maybe mutualize code for call/push
        for byte in self.pc.to_le_bytes().iter() {
            self.sp -= 1;
            if !memory.set(self.sp, *byte) {
                return Err("Cannot set");
            }
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
    fn test_call() {
        let mut memory = Memory::new(MIN_RAM_SIZE as u32, vec![]);
        let _ = memory.set_bytes(4, &[
            Op::Call.bytecode(), 0x00, 0x00, 0x00, 0x06,
            Op::Panic.bytecode(),
            Op::MovRI.bytecode(), 0x00, 0x00, 0x00, 0x00, 0xff,
            Op::Halt.bytecode()
        ]);

        let mut cpu = CPU::new();
        cpu.sp = 4;
        cpu.cs = cpu.sp;
        cpu.pc = cpu.cs;
        cpu.start();

        while cpu.step(&mut memory) {}

        assert_eq!(cpu.registers[0], 0x000000ff, "r0 {} != 0x000000ff", cpu.registers[0]);
        assert_eq!(0, cpu.sp, "sp {} != 0", cpu.sp);
        assert_eq!(Some(0x00), memory.get(0), "mem[0]: 1 != {:?}", memory.get(0));
        assert_eq!(Some(0x00), memory.get(1), "mem[1]: 2 != {:?}", memory.get(1));
        assert_eq!(Some(0x00), memory.get(2), "mem[2]: 3 != {:?}", memory.get(2));
        assert_eq!(Some(0x09), memory.get(3), "mem[3]: 4 != {:?}", memory.get(3));
    }
}