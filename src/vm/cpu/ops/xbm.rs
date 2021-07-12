use std::time::Instant;

use crate::cpu::{CPU, ops};
use crate::memory::Memory;

impl CPU {
    pub(in super::super) fn op_xbm(&mut self, memory: &Memory) -> ops::Result {
        let i = match self.fetch_1byte(memory) {
            None => return Err("Cannot fetch i"),
            Some(byte) => byte,
        } as usize;

        if self.opts.print_op {
            println!("{:03}\tXBM  {}", self.meta.steps, i);
        }

        let prev = self.meta.bench[i];
        self.meta.bench[i] = Instant::now();

        println!("bench {}: {} ns", i, self.meta.bench[i].duration_since(prev).as_nanos());

        Ok(())
    }
}