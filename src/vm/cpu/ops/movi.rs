use crate::cpu::{CPU, ops};

impl CPU {
    pub(in super::super) fn op_movi(&mut self) -> ops::Result {
        let r = match self.fetch_1byte() {
            None => return Err("Cannot fetch r0"),
            Some(byte) => byte,
        } as usize;
        let imm4 = match self.fetch_4bytes() {
            None => return Err("Cannot fetch imm4"),
            Some(byte) => byte,
        } as usize;

        self.registers[r] = imm4 as i32;
        self.flags.zero = self.registers[r] == 0;
        self.flags.negative = self.registers[r] < 0;

        // println!("MOV  r{}, {} // z={}; n={}", r, imm4, self.flags.zero, self.flags.negative);

        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use crate::cpu::Op;

    use super::*;

    #[test]
    fn test_movi() {
        let mut cpu = CPU::new();
        cpu.registers[1] = 1;
        let _ = cpu.memory.set_bytes(0, &[
            Op::MOVI.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x01,
            Op::HALT.bytecode()
        ]);
        cpu.pc = 0;
        cpu.run();
        assert_eq!(cpu.registers[0], 1);
        assert_eq!(cpu.flags.zero, false);
        assert_eq!(cpu.flags.negative, false);
    }

    #[test]
    fn test_movi_zero() {
        let mut cpu = CPU::new();
        cpu.registers[0] = 1;
        let _ = cpu.memory.set_bytes(0, &[
            Op::MOVI.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x00,
            Op::HALT.bytecode()
        ]);
        cpu.pc = 0;
        cpu.run();
        assert_eq!(cpu.registers[0], 0);
        assert_eq!(cpu.flags.zero, true);
        assert_eq!(cpu.flags.negative, false);
    }
}