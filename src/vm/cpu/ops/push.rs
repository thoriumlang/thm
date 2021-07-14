use crate::cpu::{CPU, ops};
use crate::memory::Memory;

impl CPU {
    pub(in super::super) fn op_push(&mut self, memory: &mut Memory) -> ops::Result {
        // we map r = 0x12345678 like to:
        // sp = 78, sp-1 = 56, sp-2 = 34 sp-3 = 12

        let r = self.fetch_register(memory, &Self::is_general_purpose_register)
            .ok_or("push: cannot fetch r")?;

        if self.opts.print_op {
            println!("{:03}\tPUSH r{}", self.meta.steps, r);
        }

        self.sp -= 4;
        let bytes = self.registers[r].to_be_bytes();
        if !memory.set_bytes(self.sp,&bytes) {
            return Err("push: cannot set memory");
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
    fn test_push() {
        let mut memory = Memory::new(MIN_RAM_SIZE as u32, vec![]);
        let _ = memory.set_bytes(5, &[
            Op::Push.bytecode(), 0x00,
        ]);

        let mut cpu = CPU::new();
        cpu.sp = 4;
        cpu.registers[0] = 0x01020304;
        cpu.flags.zero = true;
        cpu.flags.negative = true;
        cpu.pc = 5;
        cpu.start();

        cpu.step(&mut memory);

        assert_eq!(cpu.registers[0], 0x01020304, "r0 {} != 0x01020304", cpu.registers[0]);
        assert_eq!(true, cpu.flags.zero, "zero flag not set");
        assert_eq!(true, cpu.flags.negative, "negative flag not set");
        assert_eq!(0, cpu.sp, "sp {} != 0", cpu.sp);

        assert_eq!(Some(0x01), memory.get(0), "mem[0]: 1 != {:?}", memory.get(0));
        assert_eq!(Some(0x02), memory.get(1), "mem[1]: 2 != {:?}", memory.get(1));
        assert_eq!(Some(0x03), memory.get(2), "mem[2]: 3 != {:?}", memory.get(2));
        assert_eq!(Some(0x04), memory.get(3), "mem[3]: 4 != {:?}", memory.get(3));
    }
}