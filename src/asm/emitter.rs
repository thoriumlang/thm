use std::collections::HashMap;

use crate::constants::{REG_CS, REG_PC, REG_SP};
use crate::parser::{AddressKind, Directive, Instruction, Node};

pub struct Emitter<'t> {
    nodes: &'t Vec<Node>,
    addresses: &'t HashMap<String, u32>,
    registers: HashMap<String, usize>,
}

impl<'t> Emitter<'t> {
    pub fn new(nodes: &'t Vec<Node>, addresses: &'t HashMap<String, u32>) -> Emitter<'t> {
        let mut registers = HashMap::new();
        registers.insert("cp".to_string(), REG_PC);
        registers.insert("sp".to_string(), REG_SP);
        registers.insert("cs".to_string(), REG_CS);
        for r in 0..=31 {
            registers.insert(format!("r{}", r).to_string(), r);
        }
        Emitter {
            nodes,
            addresses,
            registers,
        }
    }

    pub fn emit(&self) -> Vec<u8> {
        let mut bytes = vec![];
        let mut base_address = 0u32;

        for node in self.nodes {
            match node {
                Node::Directive(Directive::Base(addr)) => {
                    base_address = *addr;
                    continue;
                }
                Node::Instruction(instruction) => match instruction {
                    Instruction::I(op) => bytes.append(vec![op.bytecode(), 0, 0, 0].as_mut()),
                    Instruction::IB(op, imm1) => bytes.append(vec![op.bytecode(), *imm1, 0, 0].as_mut()),
                    Instruction::IRW(op, r, value) => {
                        bytes.append(vec![op.bytecode(), *self.decode_register(r) as u8, 0, 0].as_mut());
                        let b = value.to_be_bytes();
                        bytes.extend_from_slice(&b);
                    }
                    Instruction::IR(op, r) => bytes.append(vec![op.bytecode(), *self.decode_register(r) as u8, 0, 0].as_mut()),
                    Instruction::IRR(op, r1, r2) => bytes.append(vec![
                        op.bytecode(), *self.decode_register(r1) as u8, *self.decode_register(r2) as u8, 0,
                    ].as_mut()),
                    Instruction::IA(op, addr, kind) => {
                        bytes.append(vec![op.bytecode(), 0, 0, 0].as_mut());
                        let b = (match kind {
                            AddressKind::Absolute => base_address,
                            AddressKind::Segment => 0,
                        } + self.decode_address(addr)).to_be_bytes();
                        bytes.extend_from_slice(&b);
                    }
                }
                _ => continue,
            }
        }

        bytes
    }

    fn decode_address(&self, address: &String) -> u32 {
        self.addresses.get(address).unwrap().to_owned()
    }

    fn decode_register(&self, r: &String) -> &usize {
        self.registers.get(r).unwrap()
    }
}

#[cfg(test)]
mod tests {
    use crate::op::Op;
    use crate::address_resolver::AddressResolver;
    use crate::parser::AddressKind::Absolute;
    use crate::parser::Instruction;

    use super::*;

    #[test]
    fn emit() {
        let nodes = vec![
            Node::Instruction(Instruction::IA(Op::Jseq, "label2".to_string(), Absolute)),
            Node::Label("label2".to_string()),
        ];
        let addresses = AddressResolver::new(&nodes).resolve().unwrap();

        let bytes = Emitter::new(&nodes, &addresses).emit();

        assert_eq!(8, bytes.len());
    }
}