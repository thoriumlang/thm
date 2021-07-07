use crate::cpu::{CPU, ops};

impl CPU {
    pub(in super::super) fn op_stor(&mut self) -> ops::Result {
        // we map r = 0x12345678 like to:
        // addr = 12, addr+1 = 34, addr+2 = 56, addr+3 = 78

        let r0 = match self.fetch_1byte() {
            None => return Err("Cannot fetch r0"),
            Some(byte) => byte
        } as usize;

        let r1 = match self.fetch_1byte() {
            None => return Err("Cannot fetch r1"),
            Some(byte) => byte
        } as usize;

        if self.opts.print_op {
            println!("{:03}\tSTOR r{}, r{}", self.meta.steps, r0, r1);
        }

        let mut address = self.registers[r0] as u32;
        for byte in self.registers[r1].to_be_bytes().iter() {
            if !self.memory.set(address, *byte) {
                return Err("Cannot set");
            }
            address += 1;
        }

        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use crate::cpu::Op;

    use super::*;

    #[test]
    fn test_stor() {
        let mut cpu = CPU::new();

        cpu.registers[0] = 0x01020304;
        cpu.registers[1] = 0x00000000;
        let _ = cpu.memory.set_bytes(5, &[
            Op::STOR.bytecode(), 0x01, 0x00
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