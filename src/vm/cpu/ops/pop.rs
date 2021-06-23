use crate::cpu::CPU;

impl CPU {
    pub(in super::super) fn op_pop(&mut self) {
        // we map sp = 78, sp-1 = 56, sp-2 = 34 sp-3 = 12 like to:
        // r = 0x12345678
        let r = self.fetch_1byte() as usize;
        let mut bytes: [u8; 4] = [0; 4];
        for i in 0..4 {
            bytes[i] = self.memory.get(self.sp).unwrap(); // FIXME panic
            self.sp += 1;
        }
        self.registers[r] = i32::from_be_bytes(bytes);
        self.flags.zero = self.registers[r] == 0;
        self.flags.negative = self.registers[r] < 0;
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
        cpu.memory.set(0, 0x01);
        cpu.memory.set(1, 0x02);
        cpu.memory.set(2, 0x03);
        cpu.memory.set(3, 0x04);
        cpu.sp = 0;
        cpu.flags.zero = true;
        cpu.flags.negative = true;
        cpu.program = vec![
            // POP r0
            Op::POP.bytecode(), 0x00,
            Op::HALT.bytecode()
        ];
        cpu.run();
        assert_eq!(cpu.registers[0], 0x01020304, "reg {} != {}", cpu.registers[0], 0x01020304);
        assert_eq!(false, cpu.flags.zero, "zero flag not set");
        assert_eq!(false, cpu.flags.negative, "negative flag set");
        assert_eq!(4, cpu.sp, "sp {} != 4", cpu.sp);
    }
}