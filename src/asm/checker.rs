use crate::parser::{Instruction, Node};

pub struct VmConfig {
    pub register_count: u8,
}


pub struct Checker {
    vm_config: VmConfig,
}

impl Checker {
    pub fn new(vm_config: VmConfig) -> Checker {
        Checker {
            vm_config,
        }
    }

    pub fn check(&self, nodes: &Vec<Node>) -> Result<(), Vec<String>> {
        let errors: Vec<String> = nodes.iter()
            .flat_map(|node| match node {
                Node::Instruction(Instruction::IR(_, r)) => self.check_register_is_valid(vec![r]),
                Node::Instruction(Instruction::IRR(_, r1, r2)) => self.check_register_is_valid(vec![r1, r2]),
                Node::Instruction(Instruction::IRI(_, r, _)) => self.check_register_is_valid(vec![r]),
                _ => vec![],
            }).collect();

        if errors.is_empty() {
            Ok(())
        } else {
            Err(errors)
        }
    }

    fn check_register_is_valid(&self, registers: Vec<&u8>) -> Vec<String> {
        registers.iter()
            .filter(|r| ***r >= self.vm_config.register_count)
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
        let nodes = vec![Node::Instruction(Instruction::IR(Op::INC, 32))];

        let checker = Checker::new(VM_CONFIG);
        let result = checker.check(&nodes);

        assert_eq!(true, result.is_err());
        assert_eq!(1, result.err().unwrap().len());
    }

    #[test]
    fn test_register_invalid_rr() {
        let nodes = vec![Node::Instruction(Instruction::IRR(Op::MOV, 32, 33))];

        let checker = Checker::new(VM_CONFIG);
        let result = checker.check(&nodes);

        assert_eq!(true, result.is_err());
        assert_eq!(2, result.err().unwrap().len());
    }
}