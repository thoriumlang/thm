use std::collections::HashMap;

use crate::parser::{Instruction, Node};

pub struct Emitter<'t> {
    nodes: &'t Vec<Node>,
    addresses: &'t HashMap<String, u32>,
    registers: HashMap<String, u8>,
}

impl<'t> Emitter<'t> {
    pub fn new(nodes: &'t Vec<Node>, addresses: &'t HashMap<String, u32>) -> Emitter<'t> {
        let mut registers = HashMap::new();
        registers.insert("cs".to_string(), 255);
        registers.insert("cp".to_string(), 254);
        registers.insert("sp".to_string(), 253);
        for r in 0..32 {
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

        for node in self.nodes {
            match node {
                Node::Instruction(instruction) => match instruction {
                    Instruction::I(op) => bytes.push(op.bytecode()),
                    Instruction::II(op, imm4) => {
                        bytes.push(op.bytecode());
                        let b = imm4.to_be_bytes();
                        bytes.extend_from_slice(&b);
                    }
                    Instruction::IRI(op, r, value) => {
                        bytes.append(vec![op.bytecode(), *self.decode_register(r)].as_mut());
                        let b = value.to_be_bytes();
                        bytes.extend_from_slice(&b);
                    }
                    Instruction::IR(op, r) => bytes.append(vec![op.bytecode(), *self.decode_register(r)].as_mut()),
                    Instruction::IRR(op, r1, r2) => bytes.append(vec![
                        op.bytecode(), *self.decode_register(r1), *self.decode_register(r2)
                    ].as_mut()),
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

    fn decode_register(&self, r: &String) -> &u8 {
        self.registers.get(r).unwrap()
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
            Node::Instruction(Instruction::IA(Op::Jreq, "label2".to_string())),
            Node::Label("label2".to_string()),
        ];
        let addresses = AddressResolver::new(&nodes).resolve().unwrap();

        let bytes = Emitter::new(&nodes, &addresses).emit();

        assert_eq!(5, bytes.len());
    }
}