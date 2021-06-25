use std::collections::HashMap;

use crate::parser::{Instruction, Node};

pub struct Emitter<'t> {
    nodes: &'t Vec<Node>,
    addresses: &'t HashMap<String, u32>,
}

impl<'t> Emitter<'t> {
    pub fn new(nodes: &'t Vec<Node>, addresses: &'t HashMap<String, u32>) -> Emitter<'t> {
        Emitter {
            nodes,
            addresses,
        }
    }

    pub fn emit(&self) -> Vec<u8> {
        let mut bytes = vec![];

        for node in self.nodes {
            match node {
                Node::Instruction(instruction) => match instruction {
                    Instruction::I(op) => bytes.push(op.bytecode()),
                    Instruction::II(op, imm4) => {
                        bytes.push(op.bytecode());
                        let b = imm4.to_be_bytes();
                        bytes.extend_from_slice(&b);
                    },
                    Instruction::IRI(op, r, value) => {
                        bytes.append(vec![op.bytecode(), *r].as_mut());
                        let b = value.to_be_bytes();
                        bytes.extend_from_slice(&b);
                    }
                    Instruction::IR(op, r) => bytes.append(vec![op.bytecode(), *r].as_mut()),
                    Instruction::IRR(op, r1, r2) => bytes.append(vec![op.bytecode(), *r1, *r2].as_mut()),
                    Instruction::IA(op, addr) => {
                        bytes.push(op.bytecode());
                        let b = self.decode_address(addr).to_be_bytes();
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
}

#[cfg(test)]
mod tests {
    use vmlib::op::Op;

    use crate::address_resolver::AddressResolver;
    use crate::parser::Instruction;

    use super::*;

    #[test]
    fn emit() {
        let nodes = vec![
            Node::Instruction(Instruction::IA(Op::JEQ, "label2".to_string())),
            Node::Label("label2".to_string()),
        ];
        let addresses = AddressResolver::new(&nodes).resolve().unwrap();

        let bytes = Emitter::new(&nodes, &addresses).emit();

        assert_eq!(5, bytes.len());
    }
}