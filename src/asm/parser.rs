use std::collections::HashMap;
use std::iter::Peekable;

use crate::op::Op;
use crate::lexer::{Lexer, Position, Token};

#[derive(Debug, PartialEq)]
pub struct ParseResult {
    pub nodes: Vec<Node>,
    pub symbols: HashMap<String, Token>,
}

#[derive(Debug, PartialEq)]
pub enum Node {
    Directive(Directive),
    Instruction(Instruction),
    Label(String),
}

#[derive(Debug, PartialEq)]
pub struct Label {
    name: String,
}

#[derive(Debug, PartialEq)]
pub enum Directive {
    Base(u32),
}

#[derive(Debug, PartialEq)]
pub enum Instruction {
    I(Op),
    IA(Op, String),
    IB(Op, u8),
    IR(Op, String),
    IW(Op, u32),
    IRW(Op, String, u32),
    IRR(Op, String, String),
}

impl Instruction {
    pub fn op(&self) -> Op {
        return match self {
            &Instruction::I(op) => op,
            &Instruction::IA(op, _) => op,
            &Instruction::IB(op, _) => op,
            &Instruction::IR(op, _) => op,
            &Instruction::IW(op, _) => op,
            &Instruction::IRW(op, _, _) => op,
            &Instruction::IRR(op, _, _) => op,
        };
    }
}

pub struct Parser<'t> {
    lexer: Peekable<&'t mut Lexer>,
    symbols: &'t mut HashMap<String, Token>,
    nodes: &'t mut Vec<Node>,
}

type Result<T> = std::result::Result<T, String>;

impl<'t> Parser<'t> {
    pub fn from_lexer(lexer: &'t mut Lexer, nodes: &'t mut Vec<Node>, symbols: &'t mut HashMap<String, Token>) -> Self {
        Parser {
            lexer: lexer.peekable(),
            symbols,
            nodes,
        }
    }

    pub fn parse(&mut self) -> Result<()> {
        loop {
            match self.next() {
                None => return Ok(()),
                Some(Err(err)) => return Err(err),
                Some(Ok(n)) => {
                    self.nodes.push(n);
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
            Some(Ok(Token::Integer(_, _))) => self.symbols.insert(name, token.unwrap().unwrap()),
            Some(Ok(Token::Address(_, _))) => self.symbols.insert(name, token.unwrap().unwrap()),
            _ => return Err(format!("Expected <integer> or <addr> at {}", position).into()),
        };

        Ok(())
    }

    fn parse_directive(&mut self, name: String, position: &Position) -> Result<Directive> {
        match name.to_lowercase().as_str() {
            "base" => {
                let directive = self.read_next()
                    .and_then(|token| match token {
                        Token::Integer(_, w) => Some(Ok(Directive::Base(w))),
                        Token::Variable(_, name) => match self.symbols.get(&name) {
                            Some(Token::Integer(_, w)) => Some(Ok(Directive::Base(*w))),
                            _ => None
                        },
                        _ => None,
                    })
                    .unwrap_or(Err(format!("Expected <w> or <variable> for directive '#{}' at {}", name, position).into()));
                if self.read_eol() {
                    return directive;
                }
                Err(format!("Expected <eol> at {}", position).into())
            }
            _ => Err(format!("Unknown directive '#{}' at {}", name, position).into()),
        }
    }

    fn parse_instruction(&mut self, op: &str, position: &Position) -> Result<Instruction> {
        return match op {
            "ADD" => self.op_rr_rw(Op::AddRR, Op::AddRW, position),
            "AND" => self.op_rr_rw(Op::AndRR, Op::AndRW, position),
            "CALL" => self.op_a(Op::Call, position),
            "CMP" => self.op_rr(Op::Cmp, position),
            "DEC" => self.op_r(Op::Dec, position),
            "HALT" => self.op_void(Op::Halt, position),
            "INC" => self.op_r(Op::Inc, position),
            "J" => self.op_a_w(Op::Js, position),
            "JEQ" => self.op_a_w(Op::Jseq, position),
            "JNE" => self.op_a_w(Op::Jsne, position),
            "LOAD" => self.op_rr(Op::Load, position),
            "MOV" => self.op_rr_rw(Op::MovRR, Op::MovRW, position),
            "MUL" => self.op_rr_rw(Op::MulRR, Op::MulRW, position),
            "NOP" => self.op_void(Op::Nop, position),
            "OR" => self.op_rr_rw(Op::OrRR, Op::OrRW, position),
            "PANIC" => self.op_void(Op::Panic, position),
            "POP" => self.op_r(Op::Pop, position),
            "PUSH" => self.op_r(Op::Push, position),
            "RET" => self.op_void(Op::Ret, position),
            "STOR" => self.op_rr(Op::Stor, position),
            "SUB" => self.op_rr_rw(Op::SubRR, Op::SubRW, position),
            "XBM" => self.op_b(Op::Xbm, position),
            op => Err(format!("Invalid mnemonic '{}' at {}", op, position).into())
        };
    }

    fn op_void(&mut self, op: Op, position: &Position) -> Result<Instruction> {
        if self.read_eol() {
            return Ok(Instruction::I(op));
        }
        Err(format!("Expected <eol> at {}", position).into())
    }

    fn op_a(&mut self, op: Op, position: &Position) -> Result<Instruction> {
        let addr = match self.read_address() {
            None => return Err(format!("Expected <addr> at {}", position).into()),
            Some(str) => str,
        };

        if self.read_eol() {
            return Ok(Instruction::IA(op, addr));
        }
        Err(format!("Expected <eol> at {}", position).into())
    }

    fn op_b(&mut self, op: Op, position: &Position) -> Result<Instruction> {
        let b = match self.read_byte() {
            None => return Err(format!("Expected <b> at {}", position).into()),
            Some(i) => i,
        };

        if self.read_eol() {
            return Ok(Instruction::IB(op, b));
        }
        Err(format!("Expected <eol> at {}", position).into())
    }

    fn op_r(&mut self, op: Op, position: &Position) -> Result<Instruction> {
        let r = match self.read_register() {
            None => return Err(format!("Expected <r> at {}", position).into()),
            Some(str) => str,
        };
        if self.read_eol() {
            return Ok(Instruction::IR(op, r));
        }
        Err(format!("Expected <eol> at {}", position).into())
    }

    fn op_rr_rw(&mut self, op_rr: Op, op_ri: Op, position: &Position) -> Result<Instruction> {
        let r1 = match self.read_register() {
            None => return Err(format!("Expected <r> at {}", position).into()),
            Some(str) => str,
        };

        if !self.read_comma() {
            return Err(format!("Expected ',' at {}", position).into());
        }

        let instruction = self.read_next()
            .and_then(|token| match token {
                Token::Identifier(_, r2) => Some(Ok(Instruction::IRR(op_rr, r1, r2))),
                Token::Integer(_, w) => Some(Ok(Instruction::IRW(op_ri, r1, w))),
                Token::Variable(_, name) => match self.symbols.get(&name) {
                    Some(Token::Integer(_, w)) => Some(Ok(Instruction::IRW(op_ri, r1, *w))),
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

    fn op_a_w(&mut self, op: Op, position: &Position) -> Result<Instruction> {
        let instruction = self.read_next()
            .and_then(|token| match token {
                Token::Address(_, addr) => Some(Ok(Instruction::IA(op, addr))),
                Token::Integer(_, w) => Some(Ok(Instruction::IW(op, w))),
                Token::Variable(_, name) => match self.symbols.get(&name) {
                    Some(Token::Address(_, addr)) => Some(Ok(Instruction::IA(op, addr.into()))),
                    Some(Token::Integer(_, w)) => Some(Ok(Instruction::IW(op, *w))),
                    _ => None
                },
                _ => None,
            })
            .unwrap_or(Err(format!("Expected <variable>, <w> or <addr> at {}", position).into()));

        if self.read_eol() {
            return instruction;
        }
        Err(format!("Expected <eol> at {}", position).into())
    }

    fn op_rr(&mut self, op: Op, position: &Position) -> Result<Instruction> {
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
            return Ok(Instruction::IRR(op, r1, r2));
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

    fn read_byte(&mut self) -> Option<u8> {
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
                    Token::Directive(position, name) => Some(self.parse_directive(name, &position).map(|d| { Node::Directive(d) })),
                    Token::Label(position, label) => match self.lexer.next() {
                        Some(Ok(Token::Eol(_))) => Some(Ok(Node::Label(label))),
                        _ => Some(Err(format!("Expected <eol> at {}", position).into())),
                    },
                    Token::Section(_, _) => None,
                    Token::Op(position, op) => Some(self.parse_instruction(op.as_str(), &position).map(|i| Node::Instruction(i))),
                    Token::Variable(position, name) => match self.parse_variable(name, &position) {
                        Ok(_) => continue,
                        Err(err) => Some(Err(err))
                    }
                    other => Some(Err(format!("Expected directive, label, section, label, op or variable at {}", other.position()).into())),
                },
            };
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    macro_rules! op_void_test {
        ($($name:ident: $value:expr,)*) => {
        $(
            #[test]
            fn $name() {
                let (input, op) = $value;

                let mut lexer = Lexer::from_text(input);
                let mut nodes = vec![];
                let mut symbols = HashMap::new();
                let r = Parser::from_lexer(&mut lexer, &mut nodes, &mut symbols).next();
                assert_eq!(true, r.is_some());

                let item = r.unwrap();
                assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

                let expected = Node::Instruction(Instruction::I(op));
                let actual = item.unwrap();
                assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
            }
        )*
        }
    }

    macro_rules! op_a_test {
        ($($name:ident: $value:expr,)*) => {
        $(
            #[test]
            fn $name() {
                let (input, op, a0) = $value;

                let mut lexer = Lexer::from_text(input);
                let mut nodes = vec![];
                let mut symbols = HashMap::new();
                let r = Parser::from_lexer(&mut lexer, &mut nodes, &mut symbols).next();
                assert_eq!(true, r.is_some());

                let item = r.unwrap();
                assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

                let expected = Node::Instruction(Instruction::IA(op, a0.into()));
                let actual = item.unwrap();
                assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
            }
        )*
        }
    }

    macro_rules! op_b_test {
        ($($name:ident: $value:expr,)*) => {
        $(
            #[test]
            fn $name() {
                let (input, op, b0) = $value;

                let mut lexer = Lexer::from_text(input);
                let mut nodes = vec![];
                let mut symbols = HashMap::new();
                let r = Parser::from_lexer(&mut lexer, &mut nodes, &mut symbols).next();
                assert_eq!(true, r.is_some());

                let item = r.unwrap();
                assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

                let expected = Node::Instruction(Instruction::IB(op, b0));
                let actual = item.unwrap();
                assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
            }
        )*
        }
    }

    macro_rules! op_r_test {
        ($($name:ident: $value:expr,)*) => {
        $(
            #[test]
            fn $name() {
                let (input, op, r0) = $value;

                let mut lexer = Lexer::from_text(input);
                let mut nodes = vec![];
                let mut symbols = HashMap::new();
                let r = Parser::from_lexer(&mut lexer, &mut nodes, &mut symbols).next();
                assert_eq!(true, r.is_some());

                let item = r.unwrap();
                assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

                let expected = Node::Instruction(Instruction::IR(op, r0.into()));
                let actual = item.unwrap();
                assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
            }
        )*
        }
    }

    macro_rules! op_rr_test {
        ($($name:ident: $value:expr,)*) => {
        $(
            #[test]
            fn $name() {
                let (input, op, r0, r1) = $value;

                let mut lexer = Lexer::from_text(input);
                let mut nodes = vec![];
                let mut symbols = HashMap::new();
                let r = Parser::from_lexer(&mut lexer, &mut nodes, &mut symbols).next();
                assert_eq!(true, r.is_some());

                let item = r.unwrap();
                assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

                let expected = Node::Instruction(Instruction::IRR(op, r0.into(), r1.into()));
                let actual = item.unwrap();
                assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
            }
        )*
        }
    }

    macro_rules! op_rw_test {
        ($($name:ident: $value:expr,)*) => {
        $(
            #[test]
            fn $name() {
                let (input, op, r0, w0) = $value;

                let mut lexer = Lexer::from_text(input);
                let mut nodes = vec![];
                let mut symbols = HashMap::new();
                let r = Parser::from_lexer(&mut lexer, &mut nodes, &mut symbols).next();
                assert_eq!(true, r.is_some());

                let item = r.unwrap();
                assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

                let expected = Node::Instruction(Instruction::IRW(op, r0.into(), w0));
                let actual = item.unwrap();
                assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
            }
        )*
        }
    }

    op_void_test! {
        nop:    ("NOP\n", Op::Nop),
        halt:   ("HALT\n", Op::Halt),
        panic:  ("PANIC\n", Op::Panic),
        ret:    ("RET\n", Op::Ret),
    }

    op_a_test! {
        j:      ("J @address\n",    Op::Js,   "address"),
        jeq:    ("JEQ @address\n",  Op::Jseq, "address"),
        jne:    ("JNE @address\n",  Op::Jsne, "address"),
        call:   ("CALL @address\n", Op::Call, "address"),
    }

    op_b_test! {
        xbm:    ("XBM 42\n", Op::Xbm, 42),
    }

    op_r_test! {
        inc:    ("INC  r0\n", Op::Inc, "r0"),
        dec:    ("DEC  r0\n", Op::Dec, "r0"),
        push:   ("PUSH r0\n", Op::Push, "r0"),
        pop:    ("POP  r0\n", Op::Pop, "r0"),
    }

    op_rr_test! {
        mov_rr: ("MOV  r1, r0\n", Op::MovRR, "r1", "r0"),
        add_rr: ("ADD  r1, r0\n", Op::AddRR, "r1", "r0"),
        and_rr: ("AND  r1, r0\n", Op::AndRR, "r1", "r0"),
        or_rr:  ("OR   r1, r0\n", Op::OrRR, "r1", "r0"),
        sub_rr: ("SUB  r1, r0\n", Op::SubRR, "r1", "r0"),
        mul_rr: ("MUL  r1, r0\n", Op::MulRR, "r1", "r0"),
        cmp:    ("CMP  r1, r0\n", Op::Cmp,   "r1", "r0"),
        load:   ("LOAD r1, r0\n", Op::Load,  "r1", "r0"),
        stor:   ("STOR r1, r0\n", Op::Stor,  "r1", "r0"),
    }

    op_rw_test! {
        mov_rw: ("MOV  r1, 42\n", Op::MovRW, "r1", 42),
        add_rw: ("ADD  r1, 42\n", Op::AddRW, "r1", 42),
        and_rw: ("AND  r1, 42\n", Op::AndRW, "r1", 42),
        or_rw:  ("OR   r1, 42\n", Op::OrRW, "r1", 42),
        sub_rw: ("SUB  r1, 42\n", Op::SubRW, "r1", 42),
        mul_rw: ("MUL  r1, 42\n", Op::MulRW, "r1", 42),
    }

    #[test]
    fn test_parse_variable() {
        let mut lexer = Lexer::from_text("$v = 1\n");
        let mut nodes = vec![];
        let mut symbols = HashMap::new();
        let r = Parser::from_lexer(&mut lexer, &mut nodes, &mut symbols).next();

        assert_eq!(true, r.is_none());

        assert_eq!(true, symbols.contains_key("$v"));
        match symbols.get("$v").unwrap() {
            Token::Integer(_, v) => assert_eq!(1, *v),
            _ => assert_eq!(true, false, "map did not contain Token::Integer(_, 1)"),
        }
    }

    #[test]
    fn test_use_variable() {
        let mut lexer = Lexer::from_text("$v = 42\nMOV r1, $v\n");
        let mut nodes = vec![];
        let mut symbols = HashMap::new();
        let r = Parser::from_lexer(&mut lexer, &mut nodes, &mut symbols).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Instruction(Instruction::IRW(Op::MovRW, "r1".into(), 42));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_label() {
        let mut lexer = Lexer::from_text(" :label\n");
        let mut nodes = vec![];
        let mut symbols = HashMap::new();
        let r = Parser::from_lexer(&mut lexer, &mut nodes, &mut symbols).next();
        assert_eq!(true, r.is_some());

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

        let expected = Node::Label("label".to_string());
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_parse_directive_base_word() {
        let mut lexer = Lexer::from_text("#base 1\n");
        let mut nodes = vec![];
        let mut symbols = HashMap::new();
        let r = Parser::from_lexer(&mut lexer, &mut nodes, &mut symbols).parse();

        assert_eq!(true, r.is_ok(), "Expected Ok(...), got {:?}", r);
        assert_eq!(true, symbols.is_empty());

        let expected = vec![
            Node::Directive(Directive::Base(1)),
        ];
        assert_eq!(expected, nodes, "Expected {:?}, got {:?}", expected, nodes);
    }

    #[test]
    fn test_parse_directive_base_variable() {
        let mut lexer = Lexer::from_text("$var = 12\n#base $var\n");
        let mut nodes = vec![];
        let mut symbols = HashMap::new();
        let r = Parser::from_lexer(&mut lexer, &mut nodes, &mut symbols).parse();

        assert_eq!(true, r.is_ok(), "Expected Ok(...), got {:?}", r);

        let expected = vec![
            Node::Directive(Directive::Base(12)),
        ];
        assert_eq!(expected, nodes, "Expected {:?}, got {:?}", expected, nodes);
    }

    #[test]
    fn test_parse() {
        let mut lexer = Lexer::from_text("//test\n  :label\nMOV r1, 0\n");
        let mut nodes = vec![];
        let mut symbols = HashMap::new();
        let r = Parser::from_lexer(&mut lexer, &mut nodes, &mut symbols).parse();
        assert_eq!(true, r.is_ok(), "Expected Ok(...), got {:?}", r);

        let expected = vec![
            Node::Label("label".to_string()),
            Node::Instruction(Instruction::IRW(Op::MovRW, "r1".to_string(), 0)),
        ];
        assert_eq!(expected, nodes, "Expected {:?}, got {:?}", expected, nodes);
    }
}