use crate::cpu::{CPU, ops};

impl CPU {
    pub(in super::super) fn op_load(&mut self) -> ops::Result {
        // we map addr = 12, addr+1 = 34, addr+2 = 56, addr+3 = 78 like to:
        // r = 0x12345678

        let r0 = match self.fetch_1byte() {
            None => return Err("Cannot fetch r0"),
            Some(byte) => byte
        } as usize;

        let r1 = match self.fetch_1byte() {
            None => return Err("Cannot fetch r1"),
            Some(byte) => byte
        } as usize;

        let mut bytes: u32 = 0;
        for i in 0..4 {
            bytes = bytes << 8;
            match self.memory.get((self.registers[r1] + i) as u32) {
                None => return Err("Cannot fetch 4 bytes"),
                Some(byte) => bytes |= byte as u32,
            }
        }
        self.registers[r0] = bytes as i32;
        self.flags.zero = self.registers[r0] == 0;
        self.flags.negative = self.registers[r0] < 0;

        if self.opts.print_op {
            println!("{:03}\tLOAD r{}, r{}", self.meta.steps, r0, r1);
        }

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

        let _ = cpu.memory.set_bytes(0, &vec![0x01, 0x02, 0x03, 0x04]);

        cpu.registers[0] = 0x00000000;
        cpu.registers[1] = 0x00000000;
        let _ = cpu.memory.set_bytes(5, &[
            Op::Load.bytecode(), 0x00, 0x01
        ]);
        cpu.pc = 5;
        cpu.start();
        cpu.step();
        assert_eq!(cpu.registers[0], 0x01020304, "{} != 0x01020304", cpu.registers[0]);

        assert_eq!(Some(0x01), cpu.memory.get(0), "mem[0]: 1 != {:?}", cpu.memory.get(0));
        assert_eq!(Some(0x02), cpu.memory.get(1), "mem[1]: 2 != {:?}", cpu.memory.get(1));
        assert_eq!(Some(0x03), cpu.memory.get(2), "mem[2]: 3 != {:?}", cpu.memory.get(2));
        assert_eq!(Some(0x04), cpu.memory.get(3), "mem[3]: 4 != {:?}", cpu.memory.get(3));
    }
}