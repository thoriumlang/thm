use crate::cpu::{CPU, ops};

impl CPU {
    pub(in super::super) fn op_add(&mut self) -> ops::Result {
        let r0 = match self.fetch_1byte() {
            None => return Err("Cannot fetch r0"),
            Some(byte) => byte,
        } as usize;
        let r1 = match self.fetch_1byte() {
            None => return Err("Cannot fetch r1"),
            Some(byte) => byte,
        } as usize;
        self.registers[r0] += self.registers[r1];
        self.flags.zero = self.registers[r0] == 0;
        self.flags.negative = self.registers[r0] < 0;

        // println!("ADD  r{}, r{} // z={}; n={}", r0, r1, self.flags.zero, self.flags.negative);

        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use crate::cpu::Op;

    use super::*;

    #[test]
    fn test_add() {
        let mut cpu = CPU::new();

        let _ = cpu.memory.set_bytes(0, &[
            Op::ADD.bytecode(), 0x00, 0x01,
            Op::HALT.bytecode()
        ]);
        cpu.registers[0] = 1;
        cpu.registers[1] = 2;
        cpu.pc = 0;
        cpu.run();
        assert_eq!(cpu.registers[0], 3);
        assert_eq!(cpu.registers[1], 2);
        assert_eq!(cpu.flags.zero, false);
        assert_eq!(cpu.flags.negative, false);
    }

    #[test]
    fn test_add_zero() {
        let mut cpu = CPU::new();

        let _ = cpu.memory.set_bytes(0, &[
            Op::ADD.bytecode(), 0x00, 0x01,
            Op::HALT.bytecode()
        ]);
        cpu.registers[0] = 0;
        cpu.registers[1] = 0;
        cpu.pc = 0;
        cpu.run();
        assert_eq!(cpu.registers[0], 0);
        assert_eq!(cpu.registers[1], 0);
        assert_eq!(cpu.flags.zero, true);
        assert_eq!(cpu.flags.negative, false);
    }
}