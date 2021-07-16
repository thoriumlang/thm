use std::iter::Peekable;

use vmlib::op::Op;

use crate::lexer::{Lexer, Position, Token};
use std::collections::HashMap;

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
    IB(Op, u8),
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
            &Instruction::IB(op, _) => op,
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
    variables: HashMap<String, Token>,
}

type Result<T> = std::result::Result<T, String>;

impl<'t> Parser<'t> {
    pub fn from_lexer(lexer: &'t mut Lexer) -> Self {
        Parser {
            lexer: lexer.peekable(),
            variables: HashMap::new(),
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

    fn parse_variable(&mut self, name: String, position: &Position) -> Result<()> {
        match self.lexer.next() {
            Some(Ok(Token::Equal(_))) => {}
            _ => return Err(format!("Expected '=' at {}", position).into()),
        }

        let token = self.lexer.next();
        match token {
            Some(Ok(Token::Integer(_, _)))
            | Some(Ok(Token::Address(_, _))) => self.variables.insert(name, token.unwrap().unwrap()),
            _ =>  return Err(format!("Expected <integer> or <address> at {}", position).into()),
        };

        Ok(())
    }

    fn parse_instruction(&mut self, op: &str, position: Position) -> Result<Instruction> {
        return match op {
            "NOP" => self.parse_nop(&position),
            "HALT" => self.parse_halt(&position),
            "PANIC" => self.parse_panic(&position),
            "MOV" => self.parse_mov(&position),
            "ADD" => self.parse_add(&position),
            "SUB" => self.parse_sub(&position),
            "MUL" => self.parse_mul(&position),
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
            "LOAD" => self.parse_load(&position),
            "CALL" => self.parse_call(&position),
            "RET" => self.parse_ret(&position),
            "XBM" => self.parse_xbm(&position),
            op => Err(format!("Invalid mnemonic '{}' at {}", op, position).into())
        };
    }

    fn parse_nop(&mut self, position: &Position) -> Result<Instruction> {
        if self.read_eol() {
            return Ok(Instruction::I(Op::Nop));
        }
        Err(format!("Expected <eol> at {}", position).into())
    }

    fn parse_halt(&mut self, position: &Position) -> Result<Instruction> {
        if self.read_eol() {
            return Ok(Instruction::I(Op::Halt));
        }
        Err(format!("Expected <eol> at {}", position).into())
    }

    fn parse_panic(&mut self, position: &Position) -> Result<Instruction> {
        if self.read_eol() {
            return Ok(Instruction::I(Op::Panic));
        }
        Err(format!("Expected <eol> at {}", position).into())
    }

    fn parse_mov(&mut self, position: &Position) -> Result<Instruction> {
        let r1 = match self.read_register() {
            None => return Err(format!("Expected <r> at {}", position).into()),
            Some(str) => str,
        };

        if !self.read_comma() {
            return Err(format!("Expected ',' at {}", position).into());
        }

        let instruction = self.read_next()
            .and_then(|token| match token {
                Token::Identifier(_, r2) => Some(Ok(Instruction::IRR(Op::MovRR, r1, r2))),
                Token::Integer(_, w) => Some(Ok(Instruction::IRI(Op::MovRI, r1, w))),
                Token::Variable(_, name) => match self.variables.get(&name) {
                    Some(Token::Integer(_, w)) => Some(Ok(Instruction::IRI(Op::MovRI, r1, *w))),
                    _ => None
                },
                _ => None,
            })
            .unwrap_or(Err(format!("Expected <variable>, <w> or <r> at {}", position).into()));

        if self.read_eol() {
            return instruction;
        }
        Err(format!("Expected <eol> at {}", position).into())
    }

    fn parse_add(&mut self, position: &Position) -> Result<Instruction> {
        let r1 = match self.read_register() {
            None => return Err(format!("Expected <r> at {}", position).into()),
            Some(str) => str,
        };

        if !self.read_comma() {
            return Err(format!("Expected ',' at {}", position).into());
        }

        let instruction = self.read_next()
            .and_then(|token| match token {
                Token::Identifier(_, r2) => Some(Ok(Instruction::IRR(Op::AddRR, r1, r2))),
                Token::Integer(_, w) => Some(Ok(Instruction::IRI(Op::AddRI, r1, w))),
                _ => None,
            })
            .unwrap_or(Err(format!("Expected <w> or <r> at {}", position).into()));

        if self.read_eol() {
            return instruction;
        }
        Err(format!("Expected <eol> at {}", position).into())
    }

    fn parse_sub(&mut self, position: &Position) -> Result<Instruction> {
        let r1 = match self.read_register() {
            None => return Err(format!("Expected <r> at {}", position).into()),
            Some(str) => str,
        };

        if !self.read_comma() {
            return Err(format!("Expected ',' at {}", position).into());
        }

        let instruction = self.read_next()
            .and_then(|token| match token {
                Token::Identifier(_, r2) => Some(Ok(Instruction::IRR(Op::SubRR, r1, r2))),
                Token::Integer(_, w) => Some(Ok(Instruction::IRI(Op::SubRI, r1, w))),
                _ => None,
            })
            .unwrap_or(Err(format!("Expected <w> or <r> at {}", position).into()));

        if self.read_eol() {
            return instruction;
        }
        Err(format!("Expected <eol> at {}", position).into())
    }

    fn parse_mul(&mut self, position: &Position) -> Result<Instruction> {
        let r1 = match self.read_register() {
            None => return Err(format!("Expected <r> at {}", position).to_string()),
            Some(str) => str,
        };

        if !self.read_comma() {
            return Err(format!("Expected ',' at {}", position).to_string());
        }

        let instruction = self.read_next()
            .and_then(|token| match token {
                Token::Identifier(_, r2) => Some(Ok(Instruction::IRR(Op::MulRR, r1, r2))),
                Token::Integer(_, w) => Some(Ok(Instruction::IRI(Op::MulRI, r1, w))),
                _ => None,
            })
            .unwrap_or(Err(format!("Expected <w> or <r> at {}", position).into()));

        if self.read_eol() {
            return instruction;
        }
        Err(format!("Expected <eol> at {}", position).to_string())
    }

    fn parse_cmp(&mut self, position: &Position) -> Result<Instruction> {
        let r1 = match self.read_register() {
            None => return Err(format!("Expected <r> at {}", position).into()),
            Some(str) => str,
        };

        if !self.read_comma() {
            return Err(format!("Expected ',' at {}", position).into());
        }

        let r2 = match self.read_register() {
            None => return Err(format!("Expected <r> at {}", position).into()),
            Some(str) => str
        };

        if self.read_eol() {
            return Ok(Instruction::IRR(Op::Cmp, r1, r2));
        }
        Err(format!("Expected <eol> at {}", position).into())
    }

    fn parse_j(&mut self, position: &Position) -> Result<Instruction> {
        let instruction = self.read_next()
            .and_then(|token| match token {
                Token::Address(_, addr) => Some(Ok(Instruction::IA(Op::Jr, addr))),
                Token::Integer(_, w) => Some(Ok(Instruction::II(Op::Jr, w))),
                _ => None,
            })
            .unwrap_or(Err(format!("Expected <w> or <addr> at {}", position).into()));

        if self.read_eol() {
            return instruction;
        }
        Err(format!("Expected <eol> at {}", position).into())
    }

    fn parse_jeq(&mut self, position: &Position) -> Result<Instruction> {
        let instruction = self.read_address()
            .map(|addr| Ok(Instruction::IA(Op::Jreq, addr)))
            .unwrap_or(Err(format!("Expected <addr> at {}", position).into()));

        if self.read_eol() {
            return instruction;
        }
        Err(format!("Expected <eol> at {}", position).into())
    }

    fn parse_jne(&mut self, position: &Position) -> Result<Instruction> {
        let instruction = self.read_address()
            .map(|addr| Ok(Instruction::IA(Op::Jrne, addr)))
            .unwrap_or(Err(format!("Expected <addr> at {}", position).into()));

        if self.read_eol() {
            return instruction;
        }
        Err(format!("Expected <eol> at {}", position).into())
    }

    fn parse_ja(&mut self, position: &Position) -> Result<Instruction> {
        let r1 = match self.read_register() {
            None => return Err(format!("Expected <r> at {}", position).into()),
            Some(str) => str,
        };

        if !self.read_comma() {
            return Err(format!("Expected ',' at {}", position).into());
        }

        let r2 = match self.read_register() {
            None => return Err(format!("Expected <r> at {}", position).into()),
            Some(str) => str
        };

        if self.read_eol() {
            return Ok(Instruction::IRR(Op::Ja, r1, r2));
        }
        Err(format!("Expected <eol> at {}", position).into())
    }

    fn parse_inc(&mut self, position: &Position) -> Result<Instruction> {
        let r = match self.read_register() {
            None => return Err(format!("Expected <r> at {}", position).into()),
            Some(str) => str,
        };
        if self.read_eol() {
            return Ok(Instruction::IR(Op::Inc, r));
        }
        Err(format!("Expected <eol> at {}", position).into())
    }

    fn parse_dec(&mut self, position: &Position) -> Result<Instruction> {
        let r = match self.read_register() {
            None => return Err(format!("Expected <r> at {}", position).into()),
            Some(str) => str,
        };

        if self.read_eol() {
            return Ok(Instruction::IR(Op::Dec, r));
        }
        Err(format!("Expected <eol> at {}", position).into())
    }

    fn parse_push(&mut self, position: &Position) -> Result<Instruction> {
        let r = match self.read_register() {
            None => return Err(format!("Expected <r> at {}", position).into()),
            Some(str) => str,
        };

        if self.read_eol() {
            return Ok(Instruction::IR(Op::Push, r));
        }
        Err(format!("Expected <eol> at {}", position).into())
    }

    fn parse_pop(&mut self, position: &Position) -> Result<Instruction> {
        let r = match self.read_register() {
            None => return Err(format!("Expected <r> at {}", position).into()),
            Some(str) => str,
        };

        if self.read_eol() {
            return Ok(Instruction::IR(Op::Pop, r));
        }
        Err(format!("Expected <eol> at {}", position).into())
    }

    fn parse_stor(&mut self, position: &Position) -> Result<Instruction> {
        let r0 = match self.read_register() {
            None => return Err(format!("Expected <r> at {}", position).into()),
            Some(str) => str,
        };

        if !self.read_comma() {
            return Err(format!("Expected ',' at {}", position).into());
        }

        let r1 = match self.read_register() {
            None => return Err(format!("Expected <r> at {}", position).into()),
            Some(str) => str
        };

        if self.read_eol() {
            return Ok(Instruction::IRR(Op::Stor, r0, r1));
        }
        Err(format!("Expected <eol> at {}", position).into())
    }

    fn parse_load(&mut self, position: &Position) -> Result<Instruction> {
        let r0 = match self.read_register() {
            None => return Err(format!("Expected <r> at {}", position).into()),
            Some(str) => str,
        };

        if !self.read_comma() {
            return Err(format!("Expected ',' at {}", position).into());
        }

        let r1 = match self.read_register() {
            None => return Err(format!("Expected <r> at {}", position).into()),
            Some(str) => str
        };

        if self.read_eol() {
            return Ok(Instruction::IRR(Op::Load, r0, r1));
        }
        Err(format!("Expected <eol> at {}", position).into())
    }

    fn parse_call(&mut self, position: &Position) -> Result<Instruction> {
        let addr = match self.read_address() {
            None => return Err(format!("Expected <addr> at {}", position).into()),
            Some(str) => str,
        };

        if self.read_eol() {
            return Ok(Instruction::IA(Op::Call, addr));
        }
        Err(format!("Expected <eol> at {}", position).into())
    }

    fn parse_ret(&mut self, position: &Position) -> Result<Instruction> {
        if self.read_eol() {
            return Ok(Instruction::I(Op::Ret));
        }
        Err(format!("Expected <eol> at {}", position).into())
    }

    fn parse_xbm(&mut self, position: &Position) -> Result<Instruction> {
        let i = match self.read_imm1() {
            None => return Err(format!("Expected <b> at {}", position).into()),
            Some(i) => i,
        };

        if self.read_eol() {
            return Ok(Instruction::IB(Op::Xbm, i));
        }
        Err(format!("Expected <eol> at {}", position).into())
    }

    fn read_comma(&mut self) -> bool {
        match self.lexer.next() {
            Some(Ok(Token::Comma(_))) => true,
            _ => false,
        }
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
        match self.lexer.next() {
            Some(Ok(Token::Identifier(_, r))) => Some(r),
            _ => None,
        }
    }

    fn read_address(&mut self) -> Option<String> {
        match self.lexer.next() {
            Some(Ok(t)) => match t {
                Token::Address(_, s) => Some(s),
                _ => None
            },
            _ => None,
        }
    }

    fn read_imm1(&mut self) -> Option<u8> {
        match self.lexer.next() {
            Some(Ok(Token::Integer(_, v))) => match v {
                0..=255 => Some(v as u8),
                _ => None,
            }
            _ => None,
        }
    }
}

impl<'t> Iterator for Parser<'t> {
    type Item = Result<Node>;

    fn next(&mut self) -> Option<Self::Item> {
        loop {
            return match self.lexer.next() {
                None => None,
                Some(Err(err)) => Some(Err(err)),
                Some(Ok(token)) => match token {
                    Token::Eol(_) => continue,
                    Token::Section(_, _) => None,
                    Token::Label(position, label) => match self.lexer.next() {
                        Some(Ok(Token::Eol(_))) => Some(Ok(Node::Label(label))),
                        _ => Some(Err(format!("Expected <eol> at {}", position).into())),
                    },
                    Token::Variable(position, name) => {
                        match self.parse_variable(name, &position) {
                            Ok(_) => continue,
                            Err(err) => Some(Err(err))
                        }
                    },
                    Token::Op(position, op) => Some(self.parse_instruction(op.as_str(), position).map(|i| Node::Instruction(i))),
                    Token::Equal(position)
                    | Token::Address(position, _)
                    | Token::Identifier(position, _)
                    | Token::Integer(position, _)
                    | Token::Comma(position) => Some(Err(format!("Expected section, label or op at {}", position).into())),
                },
            };
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_parse_variable() {
        let mut lexer = Lexer::from_text("$v = 1\n");
        let mut parser = Parser::from_lexer(&mut lexer);

        let r = parser.next();
        assert_eq!(true, r.is_none());

        assert_eq!(true, parser.variables.contains_key("$v"));
        match  parser.variables.get("$v").unwrap() {
            Token::Integer(_, v) => assert_eq!(1, *v),
            _ => assert_eq!(true, false, "map did not contain Token::Integer(_, 1)"),
        }
    }

    #[test]
    fn test_use_variable() {
        let mut lexer = Lexer::from_text("$v = 42\nMOV r1, $v\n");
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::IRI(Op::MovRI, "r1".into(), 42));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_nop() {
        let mut lexer = Lexer::from_text("NOP\n");
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::I(Op::Nop));
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

        let expected = Node::Instruction(Instruction::I(Op::Halt));
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

        let expected = Node::Instruction(Instruction::I(Op::Panic));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_mov_rr() {
        let mut lexer = Lexer::from_text("MOV r1, r0\n");
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::IRR(Op::MovRR, "r1".to_string(), "r0".to_string()));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_mov_ri() {
        let mut lexer = Lexer::from_text("MOV r1, 42\n");
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::IRI(Op::MovRI, "r1".to_string(), 42));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_add_rr() {
        let mut lexer = Lexer::from_text("ADD r1, r0\n");
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::IRR(Op::AddRR, "r1".to_string(), "r0".to_string()));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_add_ri() {
        let mut lexer = Lexer::from_text("ADD r1, 9\n");
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::IRI(Op::AddRI, "r1".to_string(), 9));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_sub_rr() {
        let mut lexer = Lexer::from_text("SUB r1, r0\n");
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::IRR(Op::SubRR, "r1".to_string(), "r0".to_string()));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_sub_ri() {
        let mut lexer = Lexer::from_text("SUB r1, 9\n");
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::IRI(Op::SubRI, "r1".to_string(), 9));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_mul_rr() {
        let mut lexer = Lexer::from_text("MUL r1, r0\n");
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::IRR(Op::MulRR, "r1".to_string(), "r0".to_string()));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_mul_ri() {
        let mut lexer = Lexer::from_text("MUL r1, 9\n");
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::IRI(Op::MulRI, "r1".to_string(), 9));
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

        let expected = Node::Instruction(Instruction::IRR(Op::Cmp, "r1".to_string(), "r0".to_string()));
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

        let expected = Node::Instruction(Instruction::IA(Op::Jr, "address".to_string()));
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

        let expected = Node::Instruction(Instruction::IA(Op::Jreq, "address".to_string()));
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

        let expected = Node::Instruction(Instruction::IA(Op::Jrne, "address".to_string()));
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

        let expected = Node::Instruction(Instruction::IRR(Op::Ja, "r0".to_string(), "r1".to_string()));
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

        let expected = Node::Instruction(Instruction::IR(Op::Inc, "r1".to_string()));
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

        let expected = Node::Instruction(Instruction::IR(Op::Dec, "r1".to_string()));
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

        let expected = Node::Instruction(Instruction::IR(Op::Push, "r1".to_string()));
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

        let expected = Node::Instruction(Instruction::IR(Op::Pop, "r1".to_string()));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_load() {
        let mut lexer = Lexer::from_text("LOAD r0, r1\n");
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::IRR(Op::Load, "r0".to_string(), "r1".to_string()));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_stor() {
        let mut lexer = Lexer::from_text("STOR r0, r1\n");
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::IRR(Op::Stor, "r0".to_string(), "r1".to_string()));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_call() {
        let mut lexer = Lexer::from_text("CALL @addr\n");
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::IA(Op::Call, "addr".to_string()));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_ret() {
        let mut lexer = Lexer::from_text("RET\n");
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::I(Op::Ret));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_xbm() {
        let mut lexer = Lexer::from_text("XBM 1\n");
        let r = Parser::from_lexer(&mut lexer).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::IB(Op::Xbm, 1));
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
            Node::Instruction(Instruction::IRI(Op::MovRI, "r1".to_string(), 0))
        ];
        let actual = r.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }
}