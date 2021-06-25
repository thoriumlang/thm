use crate::cpu::{CPU, ops};

impl CPU {
    pub(in super::super) fn op_jne(&mut self) -> ops::Result {
        if !self.flags.zero {
            let target = match self.fetch_4bytes() {
                None => return Err("Cannot fetch target"),
                Some(bytes) => bytes,
            } + self.cs;

            // println!("JNE  {:#010x}", target);

            self.pc = target;
        } else {
            self.skip_4bytes();
        }
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use crate::cpu::Op;

    use super::*;

    #[test]
    fn test_jne_zero() {
        let mut cpu = CPU::new();

        let _ = cpu.memory.set_bytes(0, &[
            // JNE #12 (i.e. LOAD 0, #2)
            Op::JNE.bytecode(), 0x00, 0x00, 0x00, 0x0C,
            // LOAD 0, #1
            Op::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x01,
            Op::HALT.bytecode(),
            // LOAD 0, #2
            Op::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x02,
            Op::HALT.bytecode()
        ]);
        cpu.flags.zero = false;
        cpu.cs = 0;
        cpu.pc = 0;
        cpu.run();
        assert_eq!(cpu.registers[0], 2);
    }

    #[test]
    fn test_jne_nonzero() {
        let mut cpu = CPU::new();

        let _ = cpu.memory.set_bytes(0, &[
            // JNE #12 (i.e. LOAD 0, #2)
            Op::JNE.bytecode(), 0x00, 0x00, 0x00, 0x0C,
            // LOAD 0, #1
            Op::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x01,
            Op::HALT.bytecode(),
            // LOAD 0, #2
            Op::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x02,
            Op::HALT.bytecode()
        ]);
        cpu.flags.zero = true;
        cpu.cs = 0;
        cpu.pc = 0;
        cpu.run();
        assert_eq!(cpu.registers[0], 1);
    }
}