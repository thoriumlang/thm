use crate::cpu::{CPU, ops};
use crate::memory::Memory;

impl CPU {
    pub(in super::super) fn op_call(&mut self, memory: &mut Memory) -> ops::Result {
        let target = self.fetch_word(memory)
            .ok_or("call: cannot fetch target")? + self.cs;

        if self.opts.print_op {
            println!("{:03}\tCALL {:#010x}", self.meta.steps, target);
        }

        self.sp -= 4;
        if !Self::store_word(memory, self.pc, self.sp) {
            return Err("call: cannot write memory");
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