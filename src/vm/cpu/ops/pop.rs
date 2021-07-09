use crate::cpu::{CPU, ops};

impl CPU {
    pub(in super::super) fn op_pop(&mut self) -> ops::Result {
        // we map sp = 78, sp-1 = 56, sp-2 = 34 sp-3 = 12 like to:
        // r = 0x12345678
        let r = match self.fetch_1byte() {
            None => return Err("Cannot fetch r"),
            Some(byte) => byte
        } as usize;

        let mut bytes: u32 = 0;
        for i in 0..4 {
            bytes = bytes << 8;
            match self.memory.get(self.sp + i) {
                None => return Err("Cannot fetch 4 bytes"),
                Some(byte) => bytes |= byte as u32,
            }
        }
        self.sp += 4;
        self.registers[r] = bytes as i32;
        self.flags.zero = self.registers[r] == 0;
        self.flags.negative = self.registers[r] < 0;

        if self.opts.print_op {
            println!("{:03}\tPOP  r{}", self.meta.steps, r);
        }

        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use crate::cpu::Op;

    use super::*;

    #[test]
    fn test_pop() {
        let mut cpu = CPU::new();

        // pretend we pushed something before
        let _ = cpu.memory.set(0, 0x01);
        let _ = cpu.memory.set(1, 0x02);
        let _ = cpu.memory.set(2, 0x03);
        let _ = cpu.memory.set(3, 0x04);
        cpu.sp = 0;
        cpu.flags.zero = true;
        cpu.flags.negative = true;
        let _ = cpu.memory.set_bytes(4, &[
            Op::Pop.bytecode(), 0x00,
        ]);
        cpu.pc = 4;
        cpu.start();
        cpu.step();
        assert_eq!(cpu.registers[0], 0x01020304, "reg {} != {}", cpu.registers[0], 0x01020304);
        assert_eq!(false, cpu.flags.zero, "zero flag not set");
        assert_eq!(false, cpu.flags.negative, "negative flag set");
        assert_eq!(4, cpu.sp, "sp {} != 4", cpu.sp);
    }
}