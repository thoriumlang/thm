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
    II(Op, u32),
    IRI(Op, u8, u32),
    IR(Op, u8),
    IRR(Op, u8, u8),
    IA(Op, String),
}

impl Instruction {
    pub fn op(&self) -> Op {
        return match self {
            &Instruction::I(op) => op,
            &Instruction::II(op, _) => op,
            &Instruction::IRI(op, _, _) => op,
            &Instruction::IR(op, _) => op,
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

    fn parse_instruction(&mut self, op: &str, position: Position) -> Result<Instruction> {
        // todo this function deserves a refactoring ...
        return match op {
            "NOP" => {
                let ret = Ok(Instruction::I(Op::NOP));
                match self.read_eol() {
                    false => return Err(format!("Expected <eol> at {}", position).to_string()),
                    true => (),
                }
                ret
            }
            "HALT" => {
                let ret = Ok(Instruction::I(Op::HALT));
                match self.read_eol() {
                    false => return Err(format!("Expected <eol> at {}", position).to_string()),
                    true => (),
                }
                ret
            }
            "PANIC" => {
                match self.read_eol() {
                    false => return Err(format!("Expected <eol> at {}", position).to_string()),
                    true => (),
                }
                let ret = Ok(Instruction::I(Op::PANIC));
                ret
            }
            "MOV" => {
                let r1 = match self.read_register() {
                    None => return Err(format!("Expected <register> at {}", position).to_string()),
                    Some(t) => t,
                };
                match self.read_comma() {
                    false => return Err(format!("Expected , at {}", position).to_string()),
                    true => (),
                }
                let instruction = match self.read_next() {
                    Some(t) => match t {
                        Token::Register(_, r2) => Instruction::IRR(Op::MOV, r1, r2),
                        Token::Integer(_, imm4) => Instruction::IRI(Op::MOVI, r1, imm4),
                        _ => return Err(format!("Expected <imm> or <r> at {}", position).to_string())
                    }
                    _ => return Err(format!("Expected <imm> or <r> at {}", position).to_string())
                };
                match self.read_eol() {
                    false => return Err(format!("Expected <eol> at {}", position).to_string()),
                    true => (),
                }
                Ok(instruction)
            }
            "ADD" => {
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
                let ret = Ok(Instruction::IRR(Op::ADD, reg1, reg2));
                ret
            }
            "CMP" => {
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
                let ret = Ok(Instruction::IRR(Op::CMP, reg1, reg2));
                ret
            }
            "J" => {
                let instruction = match self.read_next() {
                    Some(t) => match t {
                        Token::Address(_, addr) => Instruction::IA(Op::JR, addr),
                        Token::Integer(_, imm4) => Instruction::II(Op::JR, imm4),
                        _ => return Err(format!("Expected <imm> or <addr> at {}", position).to_string())
                    }
                    _ => return Err(format!("Expected <imm> or <addr> at {}", position).to_string())
                };
                match self.read_eol() {
                    false => return Err(format!("Expected <eol> at {}", position).to_string()),
                    true => (),
                }
                Ok(instruction)
            }
            "JEQ" => {
                let address = match self.read_address() {
                    None => return Err(format!("Expected <addr> at {}", position).to_string()),
                    Some(t) => t,
                };
                match self.read_eol() {
                    false => return Err(format!("Expected <eol> at {}", position).to_string()),
                    true => (),
                }
                let ret = Ok(Instruction::IA(Op::JREQ, address));
                ret
            }
            "JNE" => {
                let address = match self.read_address() {
                    None => return Err(format!("Expected <addr> at {}", position).to_string()),
                    Some(t) => t,
                };
                match self.read_eol() {
                    false => return Err(format!("Expected <eol> at {}", position).to_string()),
                    true => (),
                }
                let ret = Ok(Instruction::IA(Op::JRNE, address));
                ret
            }
            "JA" => {
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
                let ret = Ok(Instruction::IRR(Op::JA, reg1, reg2));
                ret
            }
            "INC" => {
                let reg = match self.read_register() {
                    None => return Err(format!("Expected <register> at {}", position).to_string()),
                    Some(t) => t,
                };
                match self.read_eol() {
                    false => return Err(format!("Expected <eol> at {}", position).to_string()),
                    true => (),
                }
                let ret = Ok(Instruction::IR(Op::INC, reg));
                ret
            }
            "DEC" => {
                let reg = match self.read_register() {
                    None => return Err(format!("Expected <register> at {}", position).to_string()),
                    Some(t) => t,
                };
                match self.read_eol() {
                    false => return Err(format!("Expected <eol> at {}", position).to_string()),
                    true => (),
                }
                let ret = Ok(Instruction::IR(Op::DEC, reg));
                ret
            }
            "PUSH" => {
                let reg = match self.read_register() {
                    None => return Err(format!("Expected <register> at {}", position).to_string()),
                    Some(t) => t,
                };
                match self.read_eol() {
                    false => return Err(format!("Expected <eol> at {}", position).to_string()),
                    true => (),
                }
                let ret = Ok(Instruction::IR(Op::PUSH, reg));
                ret
            }
            "POP" => {
                let reg = match self.read_register() {
                    None => return Err(format!("Expected <register> at {}", position).to_string()),
                    Some(t) => t,
                };
                match self.read_eol() {
                    false => return Err(format!("Expected <eol> at {}", position).to_string()),
                    true => (),
                }
                let ret = Ok(Instruction::IR(Op::POP, reg));
                ret
            }
            op => Err(format!("Invalid mnemonic '{}' at {}", op, position).to_string())
        };
    }

    fn read_comma(&mut self) -> bool {
        return match self.lexer.next() {
            Some(Ok(Token::Comma(_))) => true,
            _ => false,
        };
    }

    fn read_eol(&mut self) -> bool {
        match self.read_next() {
            Some(Token::Eol(_)) => true,
            _ => false
        }
    }

    fn read_next(&mut self) -> Option<Token> {
        match self.lexer.next() {
            Some(Ok(t)) => Some(t),
            _ => None
        }
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
                                    Some(self.parse_instruction(op.as_str(), position).map(|i| Node::Instruction(i)))
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
    use crate::lexer::VmConfig;

    use super::*;

    const VM_CONFIG: VmConfig = VmConfig {
        register_count: 32,
    };

    #[test]
    fn test_nop() {
        let mut lexer = Lexer::from_text("NOP\n", VM_CONFIG);
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
        let mut lexer = Lexer::from_text("HALT\n", VM_CONFIG);
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
        let mut lexer = Lexer::from_text("PANIC\n", VM_CONFIG);
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::I(Op::PANIC));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_mov_r_r() {
        let mut lexer = Lexer::from_text("MOV r1, r0\n", VM_CONFIG);
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::IRR(Op::MOV, 1, 0));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_mov_r_imm4() {
        let mut lexer = Lexer::from_text("MOV r1, 42\n", VM_CONFIG);
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::IRI(Op::MOVI, 1, 42));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_add() {
        let mut lexer = Lexer::from_text("ADD r1, r0\n", VM_CONFIG);
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
        let mut lexer = Lexer::from_text("CMP r1, r0\n", VM_CONFIG);
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::IRR(Op::CMP, 1, 0));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_j() {
        let mut lexer = Lexer::from_text("J @address\n", VM_CONFIG);
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::IA(Op::JR, "address".to_string()));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_jeq() {
        let mut lexer = Lexer::from_text("JEQ @address\n", VM_CONFIG);
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::IA(Op::JREQ, "address".to_string()));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_jne() {
        let mut lexer = Lexer::from_text("JNE @address\n", VM_CONFIG);
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::IA(Op::JRNE, "address".to_string()));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_ja() {
        let mut lexer = Lexer::from_text("JA r0, r1\n", VM_CONFIG);
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::IRR(Op::JA, 0, 1));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_inc() {
        let mut lexer = Lexer::from_text("INC r1\n", VM_CONFIG);
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::IR(Op::INC, 1));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_dec() {
        let mut lexer = Lexer::from_text("DEC r1\n", VM_CONFIG);
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::IR(Op::DEC, 1));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_push() {
        let mut lexer = Lexer::from_text("PUSH r1\n", VM_CONFIG);
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::IR(Op::PUSH, 1));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_pop() {
        let mut lexer = Lexer::from_text("POP r1\n", VM_CONFIG);
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::IR(Op::POP, 1));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_label() {
        let mut lexer = Lexer::from_text(" :label\n", VM_CONFIG);
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
        let mut lexer = Lexer::from_text("//test\n  :label\nMOV r1, 0\n", VM_CONFIG);
        let r = Parser::from_lexer(&mut lexer).parse();
        assert_eq!(true, r.is_ok());

        let expected = vec![
            Node::Label("label".to_string()),
            Node::Instruction(Instruction::IRI(Op::MOVI, 1, 0))
        ];
        let actual = r.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }
}