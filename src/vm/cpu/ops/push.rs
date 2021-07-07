use crate::cpu::{CPU, ops};

impl CPU {
    pub(in super::super) fn op_push(&mut self) -> ops::Result {
        // we map r = 0x12345678 like to:
        // sp = 78, sp-1 = 56, sp-2 = 34 sp-3 = 12

        let r = match self.fetch_1byte() {
            None => return Err("Cannot fetch r"),
            Some(byte) => byte
        } as usize;

        if self.opts.print_op {
            println!("{:03}\tPUSH r{}", self.meta.steps, r);
        }

        for byte in self.registers[r].to_le_bytes().iter() {
            self.sp -= 1;
            if !self.memory.set(self.sp, *byte) {
                return Err("Cannot set");
            }
        }

        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use crate::cpu::Op;

    use super::*;

    #[test]
    fn test_push() {
        let mut cpu = CPU::new();

        cpu.sp = 4;
        cpu.registers[0] = 0x01020304;
        cpu.flags.zero = true;
        cpu.flags.negative = true;
        let _ = cpu.memory.set_bytes(5, &[
            Op::PUSH.bytecode(), 0x00,
        ]);
        cpu.pc = 5;
        cpu.start();
        cpu.step();
        assert_eq!(cpu.registers[0], 0x01020304, "{} != 1", cpu.registers[0]);
        assert_eq!(true, cpu.flags.zero, "zero flag not set");
        assert_eq!(true, cpu.flags.negative, "negative flag not set");
        assert_eq!(0, cpu.sp, "sp {} != 0", cpu.sp);

        assert_eq!(Some(0x01), cpu.memory.get(0), "mem[3]: 1 != {:?}", cpu.memory.get(3));
        assert_eq!(Some(0x02), cpu.memory.get(1), "mem[2]: 2 != {:?}", cpu.memory.get(2));
        assert_eq!(Some(0x03), cpu.memory.get(2), "mem[1]: 3 != {:?}", cpu.memory.get(1));
        assert_eq!(Some(0x04), cpu.memory.get(3), "mem[0]: 4 != {:?}", cpu.memory.get(1));
    }
}