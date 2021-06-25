use crate::cpu::{CPU, ops};

impl CPU {
    pub(in super::super) fn op_load(&mut self) -> ops::Result {
        let r = match self.fetch_1byte() {
            None => return Err("Cannot fetch r"),
            Some(byte) => byte,
        } as usize;
        let immediate = match self.fetch_4bytes() {
            None => return Err("Cannot fetch 4 bytes"),
            Some(bytes) => bytes
        } as i32;

        self.registers[r] = immediate;
        self.flags.zero = self.registers[r] == 0;
        self.flags.negative = self.registers[r] < 0;

        // println!("LOAD r{}, {:#010x} // z={}; n={}", r, immediate, self.flags.zero, self.flags.negative);

        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use crate::cpu::Op;

    use super::*;

    #[test]
    fn test_load() {
        let mut cpu = CPU::new();
        let _ = cpu.memory.set_bytes(0, &[
            // LOAD 0, #16909320
            Op::LOAD.bytecode(), 0x00, 0x01, 0x02, 0x04, 0x08,
            Op::HALT.bytecode()
        ]);
        cpu.pc = 0;
        cpu.run();
        assert_eq!(cpu.registers[0], 16909320);
        assert_eq!(cpu.flags.zero, false);
        assert_eq!(cpu.flags.negative, false);
    }

    #[test]
    fn test_load_zero() {
        let mut cpu = CPU::new();
        let _ = cpu.memory.set_bytes(0, &[
            // LOAD 0, #0
            Op::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x00,
            Op::HALT.bytecode()
        ]);
        cpu.pc = 0;
        cpu.run();
        assert_eq!(cpu.registers[0], 0);
        assert_eq!(cpu.flags.zero, true);
        assert_eq!(cpu.flags.negative, false);
    }
}