use crate::cpu::{CPU, ops};

impl CPU {
    pub(in super::super) fn op_nop(&mut self) -> ops::Result {
        if self.opts.print_op {
            println!("NOP");
        }
        Ok(())
    }
}
