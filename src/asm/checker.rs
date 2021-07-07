use std::collections::HashMap;

use crate::parser::{Instruction, Node};

pub struct VmConfig {
    pub register_count: u8,
}

pub struct Checker {
    vm_config: VmConfig,
    registers: HashMap<String, u8>,
}

impl Checker {
    pub fn new(vm_config: VmConfig) -> Checker {
        let mut registers = HashMap::new();
        registers.insert("cs".to_string(), 255);
        registers.insert("cp".to_string(), 254);
        registers.insert("sp".to_string(), 253);
        for r in 0..vm_config.register_count {
            registers.insert(format!("r{}", r).to_string(), r);
        }
        Checker {
            vm_config,
            registers,
        }
    }

    pub fn check(&self, nodes: &Vec<Node>) -> Option<Vec<String>> {
        let errors: Vec<String> = nodes.iter()
            .flat_map(|node| match node {
                Node::Instruction(Instruction::IR(_, r)) => self.check_register_is_valid(vec![r]),
                Node::Instruction(Instruction::IRR(_, r1, r2)) => self.check_register_is_valid(vec![r1, r2]),
                Node::Instruction(Instruction::IRI(_, r, _)) => self.check_register_is_valid(vec![r]),
                _ => vec![],
            }).collect();

        if errors.is_empty() {
            None
        } else {
            Some(errors)
        }
    }

    fn check_register_is_valid(&self, registers: Vec<&String>) -> Vec<String> {
        registers.iter()
            .filter(|r| !self.registers.contains_key(**r))
            .map(|r| format!("{} is not a valid register", r))
            .collect()
    }
}

#[cfg(test)]
mod tests {
    use vmlib::op::Op;

    use crate::parser::{Instruction, Node};

    use super::*;

    const VM_CONFIG: VmConfig = VmConfig {
        register_count: 32,
    };

    #[test]
    fn test_register_invalid_r() {
        let nodes = vec![Node::Instruction(Instruction::IR(Op::INC, "r32".to_string()))];

        let checker = Checker::new(VM_CONFIG);
        let result = checker.check(&nodes);

        assert_eq!(true, result.is_some());
        assert_eq!(1, result.unwrap().len());
    }

    #[test]
    fn test_register_invalid_rr() {
        let nodes = vec![Node::Instruction(Instruction::IRR(Op::MOV, "r32".to_string(), "r33".to_string()))];

        let checker = Checker::new(VM_CONFIG);
        let result = checker.check(&nodes);

        assert_eq!(true, result.is_some());
        assert_eq!(2, result.unwrap().len());
    }

    #[test]
    fn test_register_invalid_ri() {
        let nodes = vec![Node::Instruction(Instruction::IRI(Op::MOV, "r32".to_string(), 42))];

        let checker = Checker::new(VM_CONFIG);
        let result = checker.check(&nodes);

        assert_eq!(true, result.is_some());
        assert_eq!(1, result.unwrap().len());
    }
}