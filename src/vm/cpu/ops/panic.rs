use crate::cpu::{CPU, ops};

impl CPU {
    pub(in super::super) fn op_panic(&mut self, message: &str) -> ops::Result {
        let _ = self.op_halt();
        panic!("Panic at {:#010x}: {}", self.pc, message);
    }
}