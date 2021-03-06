use std::collections::HashMap;

use crate::parser::{Directive, Instruction, Node};

pub struct AddressResolver<'t> {
    nodes: &'t Vec<Node>,
}

type Result<T> = std::result::Result<T, String>;

impl<'t> AddressResolver<'t> {
    pub fn new(nodes: &'t Vec<Node>) -> AddressResolver<'t> {
        AddressResolver { nodes }
    }

    pub fn resolve(&self) -> Result<HashMap<String, u32>> {
        let mut map = HashMap::new();

        let mut position = 0 as u32;
        for node in self.nodes {
            match node {
                Node::Directive(Directive::Word(label, _)) => {
                    if map.contains_key(label) {
                        return Err(format!("Label {} used more than once", label).to_string());
                    }
                    map.insert(label.to_owned(), position);
                    position += 4 as u32; // todo extract to a word_size constant?
                }
                Node::Instruction(i) => {
                    position += i.op().length() as u32;
                }
                Node::Label(label) => {
                    if map.contains_key(label) {
                        return Err(format!("Label {} used more than once", label).to_string());
                    }
                    map.insert(label.to_owned(), position);
                }
                _ => continue,
            }
        }

        for node in self.nodes {
            match node {
                Node::Instruction(Instruction::IA(_, address, _)) => {
                    if !map.contains_key(address) {
                        return Err(format!("Label {} is missing", address).to_string());
                    }
                }
                _ => continue,
            }
        }

        Ok(map)
    }
}

#[cfg(test)]
mod tests {
    use crate::parser::{AddressKind, Instruction};
    use crate::op::Op;
    use super::*;

    #[test]
    fn resolve_success() {
        let nodes = vec![
            Node::Label("label1".to_string()),
            Node::Instruction(Instruction::IA(Op::JeqS, "label2".to_string(), AddressKind::Segment)),
            Node::Label("label2".to_string()),
        ];
        let addresses = AddressResolver::new(&nodes).resolve();

        assert_eq!(true, addresses.is_ok());

        let addresses = addresses.unwrap();
        assert_eq!(true, addresses.get(&"label1".to_string()).is_some());
        assert_eq!(0, *addresses.get(&"label1".to_string()).unwrap());
        assert_eq!(true, addresses.get(&"label2".to_string()).is_some());
        assert_eq!(8, *addresses.get(&"label2".to_string()).unwrap());
    }

    #[test]
    fn resolve_duplicate_label() {
        let nodes = vec![
            Node::Label("label1".to_string()),
            Node::Instruction(Instruction::I(Op::Nop)),
            Node::Label("label1".to_string()),
        ];
        let addresses = AddressResolver::new(&nodes).resolve();

        assert_eq!(true, addresses.is_err());
        let err = addresses.err();
        assert_eq!(true, err.is_some());
        assert_eq!("Label label1 used more than once", err.unwrap());
    }

    #[test]
    fn resolve_missing_label() {
        let nodes = vec![
            Node::Instruction(Instruction::IA(Op::JeqA, "missing".to_string(), AddressKind::Absolute)),
        ];
        let addresses = AddressResolver::new(&nodes).resolve();

        assert_eq!(true, addresses.is_err());
        let err = addresses.err();
        assert_eq!(true, err.is_some());
        assert_eq!("Label missing is missing", err.unwrap());
    }
}
