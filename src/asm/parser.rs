use std::convert::TryFrom;
use std::iter::Peekable;

use vmlib::op::Op;

use crate::lexer::{Lexer, Position, Token};

#[derive(Debug, PartialEq)]
pub enum Node {
    Label(String),
    Instruction(Instruction),
}

#[derive(Debug, PartialEq)]
pub struct Label {
    name: String,
}

#[derive(Debug, PartialEq)]
pub enum Instruction {
    I(Op),
    IRI(Op, u8, u32),
    IRR(Op, u8, u8),
    IA(Op, String),
}

impl Instruction {
    pub fn op(&self) -> Op {
        return match self {
            &Instruction::I(op) => op,
            &Instruction::IRI(op, _, _) => op,
            &Instruction::IRR(op, _, _) => op,
            &Instruction::IA(op, _) => op,
        };
    }
}

pub struct Parser<'t> {
    lexer: Peekable<&'t mut Lexer>,
}

type Result<T> = std::result::Result<T, String>;

impl<'t> Parser<'t> {
    pub fn from_lexer(lexer: &'t mut Lexer) -> Self {
        Parser {
            lexer: lexer.peekable(),
        }
    }

    pub fn parse(&mut self) -> Result<Vec<Node>> {
        let mut res = vec![];
        loop {
            match self.next() {
                None => return Ok(res),
                Some(Err(err)) => return Err(err),
                Some(Ok(n)) => {
                    res.push(n);
                }
            }
        }
    }

    fn parse_instruction(&mut self, op: Op, position: Position) -> Result<Instruction> {
        return match op {
            Op::NOP => {
                let ret = Ok(Instruction::I(op));
                match self.read_eol() {
                    false => return Err(format!("Expected <eol> at {}", position).to_string()),
                    true => (),
                }
                ret
            }
            Op::HALT => {
                let ret = Ok(Instruction::I(op));
                match self.read_eol() {
                    false => return Err(format!("Expected <eol> at {}", position).to_string()),
                    true => (),
                }
                ret
            }
            Op::PANIC => {
                match self.read_eol() {
                    false => return Err(format!("Expected <eol> at {}", position).to_string()),
                    true => (),
                }
                let ret = Ok(Instruction::I(op));
                ret
            }
            Op::LOAD => {
                let reg = match self.read_register() {
                    None => return Err(format!("Expected <register> at {}", position).to_string()),
                    Some(t) => t,
                };
                match self.read_comma() {
                    false => return Err(format!("Expected , at {}", position).to_string()),
                    true => (),
                }
                let value = match self.read_integer() {
                    None => return Err(format!("Expected <integer> at {}", position).to_string()),
                    Some(t) => t
                };
                match self.read_eol() {
                    false => return Err(format!("Expected <eol> at {}", position).to_string()),
                    true => (),
                }

                let ret = Ok(Instruction::IRI(op, reg, value));
                ret
            }
            Op::MOV => {
                let reg1 = match self.read_register() {
                    None => return Err(format!("Expected <register> at {}", position).to_string()),
                    Some(t) => t,
                };
                match self.read_comma() {
                    false => return Err(format!("Expected , at {}", position).to_string()),
                    true => (),
                }
                let reg2 = match self.read_register() {
                    None => return Err(format!("Expected <register> at {}", position).to_string()),
                    Some(t) => t
                };
                match self.read_eol() {
                    false => return Err(format!("Expected <eol> at {}", position).to_string()),
                    true => (),
                }
                let ret = Ok(Instruction::IRR(op, reg1, reg2));
                ret
            }
            Op::ADD => {
                let reg1 = match self.read_register() {
                    None => return Err(format!("Expected <register> at {}", position).to_string()),
                    Some(t) => t,
                };
                match self.read_comma() {
                    false => return Err(format!("Expected , at {}", position).to_string()),
                    true => (),
                }
                let reg2 = match self.read_register() {
                    None => return Err(format!("Expected <register> at {}", position).to_string()),
                    Some(t) => t
                };
                match self.read_eol() {
                    false => return Err(format!("Expected <eol> at {}", position).to_string()),
                    true => (),
                }
                let ret = Ok(Instruction::IRR(op, reg1, reg2));
                ret
            }
            Op::CMP => {
                let reg1 = match self.read_register() {
                    None => return Err(format!("Expected <register> at {}", position).to_string()),
                    Some(t) => t,
                };
                match self.read_comma() {
                    false => return Err(format!("Expected , at {}", position).to_string()),
                    true => (),
                }
                let reg2 = match self.read_register() {
                    None => return Err(format!("Expected <register> at {}", position).to_string()),
                    Some(t) => t
                };
                match self.read_eol() {
                    false => return Err(format!("Expected <eol> at {}", position).to_string()),
                    true => (),
                }
                let ret = Ok(Instruction::IRR(op, reg1, reg2));
                ret
            }
            Op::JMP => {
                let address = match self.read_address() {
                    None => return Err(format!("Expected <address> at {}", position).to_string()),
                    Some(t) => t,
                };
                match self.read_eol() {
                    false => return Err(format!("Expected <eol> at {}", position).to_string()),
                    true => (),
                }
                let ret = Ok(Instruction::IA(op, address));
                ret
            }
            Op::JE => {
                let address = match self.read_address() {
                    None => return Err(format!("Expected <address> at {}", position).to_string()),
                    Some(t) => t,
                };
                match self.read_eol() {
                    false => return Err(format!("Expected <eol> at {}", position).to_string()),
                    true => (),
                }
                let ret = Ok(Instruction::IA(op, address));
                ret
            }
        };
    }

    fn read_comma(&mut self) -> bool {
        return match self.lexer.next() {
            Some(Ok(Token::Comma(_))) => true,
            _ => false,
        };
    }

    fn read_eol(&mut self) -> bool {
        return match self.lexer.next() {
            Some(Ok(Token::Eol(_))) => true,
            _ => false,
        };
    }

    fn read_register(&mut self) -> Option<u8> {
        return match self.lexer.next() {
            Some(Ok(t)) => match t {
                Token::Register(_, r) => Some(r),
                _ => None
            },
            _ => None,
        };
    }

    fn read_integer(&mut self) -> Option<u32> {
        return match self.lexer.next() {
            Some(Ok(t)) => match t {
                Token::Integer(_, v) => Some(v),
                _ => None
            },
            _ => None,
        };
    }

    fn read_address(&mut self) -> Option<String> {
        return match self.lexer.next() {
            Some(Ok(t)) => match t {
                Token::Address(_, s) => Some(s),
                _ => None
            },
            _ => None,
        };
    }
}

impl<'t> Iterator for Parser<'t> {
    type Item = Result<Node>;

    fn next(&mut self) -> Option<Self::Item> {
        loop {
            return match self.lexer.next() {
                None => None,
                Some(t) => {
                    match t {
                        Ok(t) => {
                            return match t {
                                Token::Label(position, label) => {
                                    match self.lexer.next() {
                                        Some(Ok(Token::Eol(_))) => Some(Ok(Node::Label(label))),
                                        _ => Some(Err(format!("Expected <eol> at {}", position).to_string())),
                                    }
                                }
                                Token::Address(position, _) => Some(Err(format!("Expected section, label or op at {}", position).to_string())),
                                Token::Register(position, _) => Some(Err(format!("Expected section, label or op at {}", position).to_string())),
                                Token::Op(position, op) => {
                                    match Op::try_from(op.as_str()) {
                                        Ok(op) => Some(self.parse_instruction(op, position).map(|i| Node::Instruction(i))),
                                        Err(e) => Some(Err(e)),
                                    }
                                }
                                Token::Integer(position, _) => Some(Err(format!("Expected section, label or op at {}", position).to_string())),
                                Token::Section(_, _) => None,
                                Token::Comma(position) => Some(Err(format!("Expected section, label or op at {}", position).to_string())),
                                Token::Eol(_) => continue,
                            };
                        }
                        Err(err) => Some(Err(err)),
                    }
                }
            };
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_nop() {
        let mut lexer = Lexer::from_text("NOP\n");
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::I(Op::NOP));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_halt() {
        let mut lexer = Lexer::from_text("HALT\n");
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::I(Op::HALT));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_panic() {
        let mut lexer = Lexer::from_text("PANIC\n");
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::I(Op::PANIC));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_load() {
        let mut lexer = Lexer::from_text("LOAD r1, x10\n");
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::IRI(Op::LOAD, 1, 16));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_mov() {
        let mut lexer = Lexer::from_text("MOV r1, r0\n");
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::IRR(Op::MOV, 1, 0));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_add() {
        let mut lexer = Lexer::from_text("ADD r1, r0\n");
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::IRR(Op::ADD, 1, 0));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_cmp() {
        let mut lexer = Lexer::from_text("CMP r1, r0\n");
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::IRR(Op::CMP, 1, 0));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_jmp() {
        let mut lexer = Lexer::from_text("JMP @address\n");
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::IA(Op::JMP, "address".to_string()));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_je() {
        let mut lexer = Lexer::from_text("JE @address\n");
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::IA(Op::JE, "address".to_string()));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_label() {
        let mut lexer = Lexer::from_text(" :label\n");
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Label("label".to_string());
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_parse() {
        let mut lexer = Lexer::from_text("//test\n  :label\nLOAD r1, 0\n");
        let r = Parser::from_lexer(&mut lexer).parse();
        assert_eq!(true, r.is_ok());

        let expected = vec![
            Node::Label("label".to_string()),
            Node::Instruction(Instruction::IRI(Op::LOAD, 1, 0))
        ];
        let actual = r.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }
}