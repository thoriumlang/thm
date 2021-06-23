use crate::cpu::CPU;

impl CPU {
    pub fn op_panic(&self) {
        panic!("Panic at {}", self.pc);
    }
}