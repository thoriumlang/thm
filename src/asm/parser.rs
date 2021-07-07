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
    IRI(Op, String, u32),
    IR(Op, String),
    IRR(Op, String, String),
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
        return match op {
            "NOP" => self.parse_nop(&position),
            "HALT" => self.parse_halt(&position),
            "PANIC" => self.parse_panic(&position),
            "MOV" => self.parse_mov(&position),
            "ADD" => self.parse_add(&position),
            "CMP" => self.parse_cmp(&position),
            "J" => self.parse_j(&position),
            "JEQ" => self.parse_jeq(&position),
            "JNE" => self.parse_jne(&position),
            "JA" => self.parse_ja(&position),
            "INC" => self.parse_inc(&position),
            "DEC" => self.parse_dec(&position),
            "PUSH" => self.parse_push(&position),
            "POP" => self.parse_pop(&position),
            "STOR" => self.parse_stor(&position),
            op => Err(format!("Invalid mnemonic '{}' at {}", op, position).to_string())
        };
    }

    fn parse_nop(&mut self, position: &Position)-> Result<Instruction> {
        let ret = Ok(Instruction::I(Op::NOP));
        match self.read_eol() {
            false => return Err(format!("Expected <eol> at {}", position).to_string()),
            true => (),
        }
        ret
    }

    fn parse_halt(&mut self, position: &Position) -> Result<Instruction> {
        let ret = Ok(Instruction::I(Op::HALT));
        match self.read_eol() {
            false => return Err(format!("Expected <eol> at {}", position).to_string()),
            true => (),
        }
        ret
    }

    fn parse_panic(&mut self, position: &Position) -> Result<Instruction> {
        match self.read_eol() {
            false => return Err(format!("Expected <eol> at {}", position).to_string()),
            true => (),
        }
        let ret = Ok(Instruction::I(Op::PANIC));
        ret
    }

    fn parse_mov(&mut self, position: &Position) -> Result<Instruction> {
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
                Token::Identifier(_, r2) => Instruction::IRR(Op::MOV, r1, r2),
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

    fn parse_add(&mut self, position: &Position) -> Result<Instruction> {
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

    fn parse_cmp(&mut self, position: &Position) -> Result<Instruction> {
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

    fn parse_j(&mut self, position: &Position) -> Result<Instruction> {
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

    fn parse_jeq(&mut self, position: &Position) -> Result<Instruction> {
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

    fn parse_jne(&mut self, position: &Position) -> Result<Instruction> {
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

    fn parse_ja(&mut self, position: &Position) -> Result<Instruction> {
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

    fn parse_inc(&mut self, position: &Position) -> Result<Instruction> {
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

    fn parse_dec(&mut self, position: &Position) -> Result<Instruction> {
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

    fn parse_push(&mut self, position: &Position) -> Result<Instruction> {
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

    fn parse_pop(&mut self, position: &Position) -> Result<Instruction> {
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

    fn parse_stor(&mut self, position: &Position) -> Result<Instruction> {
        let reg = match self.read_register() {
            None => return Err(format!("Expected <register> at {}", position).to_string()),
            Some(t) => t,
        };
        match self.read_comma() {
            false => return Err(format!("Expected , at {}", position).to_string()),
            true => (),
        }
        let imm4 = match self.read_register() {
            None => return Err(format!("Expected <register> at {}", position).to_string()),
            Some(a) => a
        };
        match self.read_eol() {
            false => return Err(format!("Expected <eol> at {}", position).to_string()),
            true => (),
        }
        Ok(Instruction::IRR(Op::STOR, reg, imm4))
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

    fn read_register(&mut self) -> Option<String> {
        return match self.lexer.next() {
            Some(Ok(Token::Identifier(_, r))) => Some(r),
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
                                Token::Identifier(position, _) => Some(Err(format!("Expected section, label or op at {}", position).to_string())),
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
    fn test_mov_r_r() {
        let mut lexer = Lexer::from_text("MOV r1, r0\n");
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::IRR(Op::MOV, "r1".to_string(), "r0".to_string()));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_mov_r_imm4() {
        let mut lexer = Lexer::from_text("MOV r1, 42\n");
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::IRI(Op::MOVI, "r1".to_string(), 42));
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

        let expected = Node::Instruction(Instruction::IRR(Op::ADD, "r1".to_string(), "r0".to_string()));
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

        let expected = Node::Instruction(Instruction::IRR(Op::CMP, "r1".to_string(), "r0".to_string()));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_j() {
        let mut lexer = Lexer::from_text("J @address\n");
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
        let mut lexer = Lexer::from_text("JEQ @address\n");
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
        let mut lexer = Lexer::from_text("JNE @address\n");
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
        let mut lexer = Lexer::from_text("JA r0, r1\n");
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::IRR(Op::JA, "r0".to_string(), "r1".to_string()));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_inc() {
        let mut lexer = Lexer::from_text("INC r1\n");
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::IR(Op::INC, "r1".to_string()));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_dec() {
        let mut lexer = Lexer::from_text("DEC r1\n");
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::IR(Op::DEC, "r1".to_string()));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_push() {
        let mut lexer = Lexer::from_text("PUSH r1\n");
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::IR(Op::PUSH, "r1".to_string()));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_pop() {
        let mut lexer = Lexer::from_text("POP r1\n");
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::IR(Op::POP, "r1".to_string()));
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
        let mut lexer = Lexer::from_text("//test\n  :label\nMOV r1, 0\n");
        let r = Parser::from_lexer(&mut lexer).parse();
        assert_eq!(true, r.is_ok(), "Expected Ok(...), got {:?}", r);

        let expected = vec![
            Node::Label("label".to_string()),
            Node::Instruction(Instruction::IRI(Op::MOVI, "r1".to_string(), 0))
        ];
        let actual = r.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }
}