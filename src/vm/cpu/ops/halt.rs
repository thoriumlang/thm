use crate::cpu::{CPU, ops};

impl CPU {
    pub(in super::super) fn op_halt(&mut self) -> ops::Result {
        self.running = false;
        Ok(())
    }
}
