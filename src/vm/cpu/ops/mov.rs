use crate::cpu::{CPU, ops};

impl CPU {
    pub(in super::super) fn op_mov(&mut self) -> ops::Result {
        let r0 = match self.fetch_1byte() {
            None => return Err("Cannot fetch r0"),
            Some(byte) => byte,
        } as usize;
        let r1 = match self.fetch_1byte() {
            None => return Err("Cannot fetch r1"),
            Some(byte) => byte,
        } as usize;

        self.registers[r0] = self.registers[r1];
        self.flags.zero = self.registers[r0] == 0;
        self.flags.negative = self.registers[r0] < 0;

        // println!("MOV  r{}, r{} // z={}; n={}", r0, r1, self.flags.zero, self.flags.negative);

        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use crate::cpu::Op;

    use super::*;

    #[test]
    fn test_mov() {
        let mut cpu = CPU::new();
        cpu.registers[1] = 1;
        let _ = cpu.memory.set_bytes(0, &[
            // MOV r0, r1
            Op::MOV.bytecode(), 0x00, 0x01,
            Op::HALT.bytecode()
        ]);
        cpu.pc = 0;
        cpu.run();
        assert_eq!(cpu.registers[0], 1);
        assert_eq!(cpu.registers[1], 1);
        assert_eq!(cpu.flags.zero, false);
        assert_eq!(cpu.flags.negative, false);
    }

    #[test]
    fn test_mov_zero() {
        let mut cpu = CPU::new();
        cpu.registers[0] = 1;
        let _ = cpu.memory.set_bytes(0, &[
            // MOV r0, r1
            Op::MOV.bytecode(), 0x00, 0x01,
            Op::HALT.bytecode()
        ]);
        cpu.pc = 0;
        cpu.run();
        assert_eq!(cpu.registers[0], 0);
        assert_eq!(cpu.registers[1], 0);
        assert_eq!(cpu.flags.zero, true);
        assert_eq!(cpu.flags.negative, false);
    }
}