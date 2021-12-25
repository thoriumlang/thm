use std::collections::HashMap;
use std::ops::Add;
use peek_nth::{IteratorExt, PeekableNth};

use crate::op::Op;
use crate::lexer::{AddressKind as LexerAddressKind, Lexer, Position, Token};
use crate::parser::AddressKind::{Absolute, Segment};

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
    Word(String, i32),
}

#[derive(Debug, PartialEq, Clone)]
pub enum AddressKind {
    Absolute,
    Segment,
}

#[derive(Debug, PartialEq, Clone)]
pub enum Instruction {
    I(Op),
    IA(Op, String, AddressKind),
    IB(Op, u8),
    IR(Op, String),
    IRA(Op, String, String, AddressKind),
    IRW(Op, String, u32),
    IRR(Op, String, String),
    IRRR(Op, String, String, String),
    IRRW(Op, String, String, u32),
    IW(Op, u32),
}

impl Instruction {
    pub fn op(&self) -> Op {
        return match self {
            &Instruction::I(op) => op,
            &Instruction::IA(op, _, _) => op,
            &Instruction::IB(op, _) => op,
            &Instruction::IR(op, _) => op,
            &Instruction::IRA(op, _, _, _) => op,
            &Instruction::IRW(op, _, _) => op,
            &Instruction::IRR(op, _, _) => op,
            &Instruction::IRRR(op, _, _, _) => op,
            &Instruction::IRRW(op, _, _, _) => op,
            &Instruction::IW(op, _) => op,
        };
    }
}

pub struct Parser<'t> {
    lexer: PeekableNth<&'t mut Lexer>,
    symbols: &'t mut HashMap<String, Token>,
    nodes: &'t mut Vec<Node>,
}

type Result<T> = std::result::Result<T, String>;

impl<'t> Parser<'t> {
    pub fn from_lexer(lexer: &'t mut Lexer, nodes: &'t mut Vec<Node>, symbols: &'t mut HashMap<String, Token>) -> Self {
        Parser {
            lexer: lexer.peekable_nth(),
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
            // Some(Ok(Token::Address(_, _, _))) => self.symbols.insert(name, token.unwrap().unwrap()),
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
            "word" => {
                let identifier = match self.read_next() {
                    Some(Token::Identifier(_, str)) => str,
                    _ => return Err(format!("Expected <string> for directive '#{}' at {}", name, position).into()),
                };
                let value = match self.read_next() {
                    Some(Token::Integer(_, val)) => val,
                    _ => return Err(format!("Expected <value> for directive '#{}' at {}", name, position).into()),
                };

                if self.read_eol() {
                    return Ok(Directive::Word(identifier, value as i32));
                }

                Err(format!("Expected <eol> at {}", position).into())
            }
            _ => Err(format!("Unknown directive '#{}' at {}", name, position).into()),
        }
    }

    fn parse_instruction(&mut self, op: &str, position: &Position) -> Result<Instruction> {
        return match op {
            "ADD" => self.parse_op(position, &[
                (Op::AddRR, Self::op_rr as fn(&mut Self, Op, &Position) -> Result<Instruction>),
                (Op::AddRW, Self::op_rw as fn(&mut Self, Op, &Position) -> Result<Instruction>),
            ]),
            "AND" => self.parse_op(position, &[
                (Op::AndRR, Self::op_rr as fn(&mut Self, Op, &Position) -> Result<Instruction>),
                (Op::AndRW, Self::op_rw as fn(&mut Self, Op, &Position) -> Result<Instruction>),
            ]),
            "CALL" => self.parse_op(position, &[
                (Op::CallS, Self::op_as as fn(&mut Self, Op, &Position) -> Result<Instruction>),
                (Op::CallA, Self::op_aa as fn(&mut Self, Op, &Position) -> Result<Instruction>),
                (Op::CallR, Self::op_r as fn(&mut Self, Op, &Position) -> Result<Instruction>),
            ]),
            "CMP" => self.parse_op(position, &[
                (Op::CmpRR, Self::op_rr as fn(&mut Self, Op, &Position) -> Result<Instruction>),
                (Op::CmpRW, Self::op_rw as fn(&mut Self, Op, &Position) -> Result<Instruction>),
            ]),
            "DEC" => self.parse_op(position, &[
                (Op::DecR, Self::op_r as fn(&mut Self, Op, &Position) -> Result<Instruction>),
            ]),
            "HALT" => self.parse_op(position, &[
                (Op::Halt, Self::op_void as fn(&mut Self, Op, &Position) -> Result<Instruction>),
            ]),
            "INC" => self.parse_op(position, &[
                (Op::IncR, Self::op_r as fn(&mut Self, Op, &Position) -> Result<Instruction>),
            ]),
            "IND" => self.parse_op(position, &[
                (Op::Ind, Self::op_void as fn(&mut Self, Op, &Position) -> Result<Instruction>),
            ]),
            "INE" => self.parse_op(position, &[
                (Op::Ine, Self::op_void as fn(&mut Self, Op, &Position) -> Result<Instruction>),
            ]),
            "INT" => self.parse_op(position, &[
                (Op::IntB, Self::op_b as fn(&mut Self, Op, &Position) -> Result<Instruction>),
            ]),
            "J" => self.parse_op(position, &[
                (Op::JA, Self::op_aa as fn(&mut Self, Op, &Position) -> Result<Instruction>),
                (Op::JS, Self::op_as as fn(&mut Self, Op, &Position) -> Result<Instruction>),
            ]),
            "JEQ" => self.parse_op(position, &[
                (Op::JeqA, Self::op_aa as fn(&mut Self, Op, &Position) -> Result<Instruction>),
                (Op::JeqS, Self::op_as as fn(&mut Self, Op, &Position) -> Result<Instruction>),
            ]),
            "JNE" => self.parse_op(position, &[
                (Op::JneA, Self::op_aa as fn(&mut Self, Op, &Position) -> Result<Instruction>),
                (Op::JneS, Self::op_as as fn(&mut Self, Op, &Position) -> Result<Instruction>),
            ]),
            "LOAD" => self.parse_op(position, &[
                (Op::LoadRR, Self::op_rr as fn(&mut Self, Op, &Position) -> Result<Instruction>),
                (Op::LoadRW, Self::op_rw as fn(&mut Self, Op, &Position) -> Result<Instruction>),
                (Op::LoadRW, Self::op_raa as fn(&mut Self, Op, &Position) -> Result<Instruction>),
                (Op::LoadRRW, Self::op_ro as fn(&mut Self, Op, &Position) -> Result<Instruction>),
            ]),
            "MI" => self.parse_op(position, &[
                (Op::MiB, Self::op_b as fn(&mut Self, Op, &Position) -> Result<Instruction>),
            ]),
            "MOV" => self.parse_op(position, &[
                (Op::MovRR, Self::op_rr as fn(&mut Self, Op, &Position) -> Result<Instruction>),
                (Op::MovRW, Self::op_rw as fn(&mut Self, Op, &Position) -> Result<Instruction>),
                (Op::MovRW, Self::op_raa as fn(&mut Self, Op, &Position) -> Result<Instruction>),
                (Op::MovRW, Self::op_ras as fn(&mut Self, Op, &Position) -> Result<Instruction>),
            ]),
            "MUL" => self.parse_op(position, &[
                (Op::MulRR, Self::op_rr as fn(&mut Self, Op, &Position) -> Result<Instruction>),
                (Op::MulRW, Self::op_rw as fn(&mut Self, Op, &Position) -> Result<Instruction>),
            ]),
            "NOP" => self.parse_op(position, &[
                (Op::Nop, Self::op_void as fn(&mut Self, Op, &Position) -> Result<Instruction>),
            ]),
            "OR" => self.parse_op(position, &[
                (Op::OrRR, Self::op_rr as fn(&mut Self, Op, &Position) -> Result<Instruction>),
                (Op::OrRW, Self::op_rw as fn(&mut Self, Op, &Position) -> Result<Instruction>),
            ]),
            "PANIC" => self.parse_op(position, &[
                (Op::Panic, Self::op_void as fn(&mut Self, Op, &Position) -> Result<Instruction>),
            ]),
            "POP" => self.parse_op(position, &[
                (Op::PopR, Self::op_r as fn(&mut Self, Op, &Position) -> Result<Instruction>),
                (Op::PopRR, Self::op_rr as fn(&mut Self, Op, &Position) -> Result<Instruction>),
                (Op::PopRRR, Self::op_rrr as fn(&mut Self, Op, &Position) -> Result<Instruction>),
            ]),
            "POPA" => self.parse_op(position, &[
                (Op::Popa, Self::op_void as fn(&mut Self, Op, &Position) -> Result<Instruction>),
            ]),
            "PUSH" => self.parse_op(position, &[
                (Op::PushR, Self::op_r as fn(&mut Self, Op, &Position) -> Result<Instruction>),
                (Op::PushRR, Self::op_rr as fn(&mut Self, Op, &Position) -> Result<Instruction>),
                (Op::PushRRR, Self::op_rrr as fn(&mut Self, Op, &Position) -> Result<Instruction>),
                (Op::PushW, Self::op_w as fn(&mut Self, Op, &Position) -> Result<Instruction>),
            ]),
            "PUSHA" => self.parse_op(position, &[
                (Op::Pusha, Self::op_void as fn(&mut Self, Op, &Position) -> Result<Instruction>),
            ]),
            "RET" => self.parse_op(position, &[
                (Op::Ret, Self::op_void as fn(&mut Self, Op, &Position) -> Result<Instruction>),
            ]),
            "STOR" => self.parse_op(position, &[
                (Op::StorRR, Self::op_rr as fn(&mut Self, Op, &Position) -> Result<Instruction>),
                (Op::StorRW, Self::op_wr as fn(&mut Self, Op, &Position) -> Result<Instruction>),
                (Op::StorRW, Self::op_aar as fn(&mut Self, Op, &Position) -> Result<Instruction>),
            ]),
            "SUB" => self.parse_op(position, &[
                (Op::SubRR, Self::op_rr as fn(&mut Self, Op, &Position) -> Result<Instruction>),
                (Op::SubRW, Self::op_rw as fn(&mut Self, Op, &Position) -> Result<Instruction>),
            ]),
            "UMI" => self.parse_op(position, &[
                (Op::UmiB, Self::op_b as fn(&mut Self, Op, &Position) -> Result<Instruction>),
            ]),
            "IRET" => self.parse_op(position, &[
                (Op::Iret, Self::op_void as fn(&mut Self, Op, &Position) -> Result<Instruction>),
            ]),
            "WFI" => self.parse_op(position, &[
                (Op::Wfi, Self::op_void as fn(&mut Self, Op, &Position) -> Result<Instruction>),
            ]),
            "XBM" => self.parse_op(position, &[
                (Op::Xbm, Self::op_b as fn(&mut Self, Op, &Position) -> Result<Instruction>),
            ]),
            "XBRK" => self.parse_op(position, &[
                (Op::Xbrk, Self::op_void as fn(&mut Self, Op, &Position) -> Result<Instruction>),
            ]),
            "XDBG" => self.parse_op(position, &[
                (Op::Xdbg, Self::op_void as fn(&mut Self, Op, &Position) -> Result<Instruction>),
            ]),
            "XPSE" => self.parse_op(position, &[
                (Op::Xpse, Self::op_void as fn(&mut Self, Op, &Position) -> Result<Instruction>),
            ]),
            "XPSD" => self.parse_op(position, &[
                (Op::Xpsd, Self::op_void as fn(&mut Self, Op, &Position) -> Result<Instruction>),
            ]),
            "XOR" => self.parse_op(position, &[
                (Op::XorRR, Self::op_rr as fn(&mut Self, Op, &Position) -> Result<Instruction>),
                (Op::XorRW, Self::op_rw as fn(&mut Self, Op, &Position) -> Result<Instruction>),
            ]),
            op => Err(format!("Invalid mnemonic '{}' at {}", op, position).into())
        };
    }

    fn parse_op<F>(&mut self, position: &Position, fs: &[(Op, F)]) -> Result<Instruction>
        where F: Fn(&mut Self, Op, &Position) -> Result<Instruction>
    {
        let results: Vec<Result<Instruction>> = fs.iter()
            .map(|op| op.1(self, op.0, position))
            .collect();

        let success: Vec<&Instruction> = results.iter()
            .filter(|e| e.is_ok())
            .map(|e| e.as_ref().unwrap())
            .collect();

        fn merge_errors(results: Vec<Result<Instruction>>) -> String {
            let mut str = String::new();
            for result in results {
                str = str.add(" * ");
                str = str.add(result.err().unwrap().as_str());
                str = str.add("\n");
            }
            return str;
        }

        match success.len() {
            0 => Err(format!("Expected one of the following alternatives:\n{}",
                             merge_errors(results)
            ).into()),
            1 => Ok((**success.get(0).unwrap()).clone()),
            _ => Err(format!("No unique alternative at {}", position).into()),
        }
    }

    fn op_aa(&mut self, op: Op, position: &Position) -> Result<Instruction> {
        match self.parse_aa(position) {
            Ok(a1) => Ok(Instruction::IA(op, a1, Absolute)),
            Err(e) => Err(e),
        }
    }

    fn op_as(&mut self, op: Op, position: &Position) -> Result<Instruction> {
        match self.parse_as(position) {
            Ok(a1) => Ok(Instruction::IA(op, a1, Segment)),
            Err(e) => Err(e),
        }
    }

    fn op_aar(&mut self, op: Op, position: &Position) -> Result<Instruction> {
        match self.parse_aar(position) {
            Ok((a1, r1)) => Ok(Instruction::IRA(op, r1, a1, Absolute)),
            Err(e) => Err(e),
        }
    }

    fn op_b(&mut self, op: Op, position: &Position) -> Result<Instruction> {
        let w_result = match self.parse_w(position) {
            Ok(w1) => match w1 {
                0..=255 => Ok(w1 as u8),
                _ => Err(format!("<b> at {}", position).into()),
            },
            Err(e) => Err(e),
        };
        if let Ok(b1) = w_result {
            return Ok(Instruction::IB(op, b1));
        }

        Err(format!("Expected {}", w_result.err().unwrap()))
    }

    fn op_r(&mut self, op: Op, position: &Position) -> Result<Instruction> {
        match self.parse_r(position) {
            Ok(r1) => Ok(Instruction::IR(op, r1)),
            Err(e) => Err(e),
        }
    }

    fn op_raa(&mut self, op: Op, position: &Position) -> Result<Instruction> {
        match self.parse_raa(position) {
            Ok((r1, a1)) => Ok(Instruction::IRA(op, r1, a1, Absolute)),
            Err(e) => Err(e)
        }
    }

    fn op_ras(&mut self, op: Op, position: &Position) -> Result<Instruction> {
        match self.parse_ras(position) {
            Ok((r1, a1)) => Ok(Instruction::IRA(op, r1, a1, Segment)),
            Err(e) => Err(e)
        }
    }

    fn op_ro(&mut self, op: Op, position: &Position) -> Result<Instruction> {
        match self.parse_ro(position) {
            Ok((r1, r2, o1)) => Ok(Instruction::IRRW(op, r1, r2, o1)),
            Err(e) => Err(e),
        }
    }

    fn op_rr(&mut self, op: Op, position: &Position) -> Result<Instruction> {
        match self.parse_rr(position) {
            Ok((r1, r2)) => Ok(Instruction::IRR(op, r1, r2)),
            Err(e) => Err(e),
        }
    }

    fn op_rw(&mut self, op: Op, position: &Position) -> Result<Instruction> {
        match self.parse_rw(position) {
            Ok((r1, w1)) => Ok(Instruction::IRW(op, r1, w1)),
            Err(e) => Err(e)
        }
    }

    fn op_rrr(&mut self, op: Op, position: &Position) -> Result<Instruction> {
        match self.parse_rrr(position) {
            Ok((r1, r2, r3)) => Ok(Instruction::IRRR(op, r1, r2, r3)),
            Err(e) => Err(e),
        }
    }

    fn op_void(&mut self, op: Op, position: &Position) -> Result<Instruction> {
        if self.read_eol() {
            return Ok(Instruction::I(op));
        }
        Err(format!("Expected <eol> at {}", position).into())
    }

    fn op_w(&mut self, op: Op, position: &Position) -> Result<Instruction> {
        match self.parse_w(position) {
            Ok(w1) => Ok(Instruction::IW(op, w1)),
            Err(e) => Err(e),
        }
    }

    fn op_wr(&mut self, op: Op, position: &Position) -> Result<Instruction> {
        match self.parse_wr(position) {
            Ok((w1, r1)) => Ok(Instruction::IRW(op, r1, w1)),
            Err(e) => Err(e),
        }
    }

    // --- parse

    /// parses `<&-addr> <eol>`
    fn parse_aa(&mut self, position: &Position) -> Result<String> {
        if !self.peek_abs_address(0) {
            return Err(format!("<&-addr> at {}", position).into());
        }
        if !self.peek_eol(1) {
            return Err(format!("<eol> at {}", position).into());
        }

        let a1 = self.read_address().unwrap();
        self.read_eol();
        return Ok(a1.0);
    }

    /// parses `<@-addr> <eol>`
    fn parse_as(&mut self, position: &Position) -> Result<String> {
        if !self.peek_seg_address(0) {
            return Err(format!("<@-addr> at {}", position).into());
        }
        if !self.peek_eol(1) {
            return Err(format!("<eol> at {}", position).into());
        }

        let a1 = self.read_address().unwrap();
        self.read_eol();
        return Ok(a1.0);
    }

    /// parses `<&-addr> ',' <r> <eol>`
    fn parse_aar(&mut self, position: &Position) -> Result<(String, String)> {
        if !self.peek_abs_address(0) {
            return Err(format!("<&-addr> at {}", position).into());
        }
        if !self.peek_comma(1) {
            return Err(format!("',' at {}", position).into());
        }
        if !self.peek_register(2) {
            return Err(format!("<r> at {}", position).into());
        }
        if !self.peek_eol(3) {
            return Err(format!("<eol> at {}", position).into());
        }

        let a1 = self.read_address().unwrap();
        self.read_comma();
        let r1 = self.read_register().unwrap();
        self.read_eol();
        return Ok((a1.0, r1));
    }

    /// parses `<r> <eol>`
    fn parse_r(&mut self, position: &Position) -> Result<String> {
        if !self.peek_register(0) {
            return Err(format!("<r> at {}", position).into());
        }
        if !self.peek_eol(1) {
            return Err(format!("<eol> at {}", position).into());
        }

        let r1 = self.read_register().unwrap();
        self.read_eol();
        return Ok(r1);
    }

    /// parses `<r> ',' <&-addr> <eol>`
    fn parse_raa(&mut self, position: &Position) -> Result<(String, String)> {
        if !self.peek_register(0) {
            return Err(format!("<r> ',' <&-addr> <eol> at {}", position).into());
        }
        if !self.peek_comma(1) {
            return Err(format!("',' <&-addr> <eol> at {}", position).into());
        }
        if !self.peek_abs_address(2) {
            return Err(format!("<&-addr> <eol> at {}", position).into());
        }
        if !self.peek_eol(3) {
            return Err(format!("<eol> at {}", position).into());
        }

        let r1 = self.read_register().unwrap();
        self.read_comma();
        let a1 = self.read_address().unwrap();
        self.read_eol();
        return Ok((r1, a1.0));
    }

    /// parses `<r> ',' <@-addr> <eol>`
    fn parse_ras(&mut self, position: &Position) -> Result<(String, String)> {
        if !self.peek_register(0) {
            return Err(format!("<r> ',' <@-addr> <eol> at {}", position).into());
        }
        if !self.peek_comma(1) {
            return Err(format!("',' <@-addr> <eol> at {}", position).into());
        }
        if !self.peek_seg_address(2) {
            return Err(format!("<@-addr> <eol> at {}", position).into());
        }
        if !self.peek_eol(3) {
            return Err(format!("<eol> at {}", position).into());
        }

        let r1 = self.read_register().unwrap();
        self.read_comma();
        let a1 = self.read_address().unwrap();
        self.read_eol();
        return Ok((r1, a1.0));
    }

    /// parses `<r> ',' '[' <r> '+' <w> ']' <eol>`
    fn parse_ro(&mut self, position: &Position) -> Result<(String, String, u32)> {
        if !self.peek_register(0) {
            return Err(format!("<r> ',' '[' <r> '+' ( <w> | <var> ) ']' <eol> at {}", position).into());
        }
        if !self.peek_comma(1) {
            return Err(format!("',' '[' <r> '+' ( <w> | <var> ) ']' <eol> at {}", position).into());
        }
        if !self.peek_lbracket(2) {
            return Err(format!("'[' <r> '+' ( <w> | <var> ) ']' <eol> at {}", position).into());
        }
        if !self.peek_register(3) {
            return Err(format!("<r> '+' ( <w> | <var> ) ']' <eol> at {}", position).into());
        }
        match self.peek(4) {
            Some(Token::Plus(_)) => (),
            _ => return Err(format!("'+' ( <w> | <var> ) ']' <eol> at {}", position).into()),
        };
        if !self.peek_word(5) && !self.peek_variable(3) {
            return Err(format!("( <w> | <var> ) ']' <eol> at {}", position).into());
        }
        if !self.peek_rbracket(6) {
            return Err(format!("']' <eol> at {}", position).into());
        }
        if !self.peek_eol(7) {
            return Err(format!("<eol> at {}", position).into());
        }

        let r1 = self.read_register().unwrap();
        self.read_comma();
        self.read_lbracket();
        let r2 = self.read_register().unwrap();
        self.read_next(); // +
        let o1 = match self.read_next().unwrap() {
            Token::Integer(_, w) => w,
            Token::Variable(_, name) => match self.symbols.get(&name) {
                Some(Token::Integer(_, w)) => *w,
                _ => return Err(format!("Unknown variable '{}' at {}", name, position).into()),
            },
            _ => return Err(format!("Unexpected token at {}", position).into())
        };
        self.read_rbracket();
        self.read_eol();
        return Ok((r1, r2, o1));
    }

    /// parses `<r> ',' <r>`
    fn parse_rr(&mut self, position: &Position) -> Result<(String, String)> {
        if !self.peek_register(0) {
            return Err(format!("<r> ',' <r> <eol> at {}", position).into());
        }
        if !self.peek_comma(1) {
            return Err(format!("',' <r> <eol> at {}", position).into());
        }
        if !self.peek_register(2) {
            return Err(format!("<r> <eol> at {}", position).into());
        }
        if !self.peek_eol(3) {
            return Err(format!("<eol> at {}", position).into());
        }

        let r1 = self.read_register().unwrap();
        self.read_comma();
        let r2 = self.read_register().unwrap();
        self.read_eol();
        return Ok((r1, r2));
    }

    /// parses `<r> ',' ( <w> | <var> ) <eol>`
    fn parse_rw(&mut self, position: &Position) -> Result<(String, u32)> {
        if !self.peek_register(0) {
            return Err(format!("<r> ',' ( <w> | <var> ) <eol> at {}", position).into());
        }
        if !self.peek_comma(1) {
            return Err(format!("',' ( <w> | <var> ) <eol> at {}", position).into());
        }
        if !self.peek_word(2) && !self.peek_variable(2) {
            return Err(format!("( <w> | <var> ) <eol> at {}", position).into());
        }
        if !self.peek_eol(3) {
            return Err(format!("<eol> at {}", position).into());
        }

        let r1 = self.read_register().unwrap();
        self.read_comma();
        let w1 = match self.read_next().unwrap() {
            Token::Integer(_, w) => w,
            Token::Variable(_, name) => match self.symbols.get(&name) {
                Some(Token::Integer(_, w)) => *w,
                _ => return Err(format!("Unknown variable '{}' at {}", name, position).into()),
            },
            _ => return Err(format!("Unexpected token at {}", position).into())
        };
        self.read_eol();
        return Ok((r1, w1));
    }

    /// parses `<r> ',' <r> ',' <r> <eol>`
    fn parse_rrr(&mut self, position: &Position) -> Result<(String, String, String)> {
        if !self.peek_register(0) {
            return Err(format!("<r> at {}", position).into());
        }
        if !self.peek_comma(1) {
            return Err(format!("',' at {}", position).into());
        }
        if !self.peek_register(2) {
            return Err(format!("<r> at {}", position).into());
        }
        if !self.peek_comma(3) {
            return Err(format!("',' at {}", position).into());
        }
        if !self.peek_register(4) {
            return Err(format!("<r> at {}", position).into());
        }
        if !self.peek_eol(5) {
            return Err(format!("<eol> at {}", position).into());
        }

        let r1 = self.read_register().unwrap();
        self.read_comma();
        let r2 = self.read_register().unwrap();
        self.read_comma();
        let r3 = self.read_register().unwrap();
        self.read_eol();
        return Ok((r1, r2, r3));
    }

    /// parses `( <w> | <var> ) <eol>`
    fn parse_w(&mut self, position: &Position) -> Result<u32> {
        if !self.peek_word(0) && !self.peek_variable(0) {
            return Err(format!("<w> or <var> at {}", position).into());
        }
        if !self.peek_eol(1) {
            return Err(format!("<eol> at {}", position).into());
        }

        let w1 = match self.read_next().unwrap() {
            Token::Integer(_, w) => w,
            Token::Variable(_, name) => match self.symbols.get(&name) {
                Some(Token::Integer(_, w)) => *w,
                _ => return Err(format!("Unknown variable '{}' at {}", name, position).into()),
            },
            _ => return Err(format!("Unexpected token at {}", position).into())
        };
        self.read_eol();
        return Ok(w1);
    }

    /// parses `( <w> | <var> ) ',' <r> <eol>`
    fn parse_wr(&mut self, position: &Position) -> Result<(u32, String)> {
        if !self.peek_word(0) && !self.peek_variable(0) {
            return Err(format!("<w> or <var> at {}", position).into());
        }
        if !self.peek_comma(1) {
            return Err(format!("',' at {}", position).into());
        }
        if !self.peek_register(2) {
            return Err(format!("<r> at {}", position).into());
        }
        if !self.peek_eol(3) {
            return Err(format!("<eol> at {}", position).into());
        }

        let w1 = match self.read_next().unwrap() {
            Token::Integer(_, w) => w,
            Token::Variable(_, name) => match self.symbols.get(&name) {
                Some(Token::Integer(_, w)) => *w,
                _ => return Err(format!("Unknown variable '{}' at {}", name, position).into()),
            },
            _ => return Err(format!("Unexpected token at {}", position).into())
        };
        self.read_comma();
        let r1 = self.read_register().unwrap();
        self.read_eol();
        return Ok((w1, r1));
    }

    // --- peek

    fn peek(&mut self, n: usize) -> Option<&Token> {
        match self.lexer.peek_nth(n) {
            Some(Ok(token)) => Some(token),
            _ => None,
        }
    }

    fn peek_comma(&mut self, n: usize) -> bool {
        match self.lexer.peek_nth(n) {
            Some(Ok(Token::Comma(_))) => true,
            _ => false,
        }
    }

    fn peek_lbracket(&mut self, n: usize) -> bool {
        match self.lexer.peek_nth(n) {
            Some(Ok(Token::LBracket(_))) => true,
            _ => false,
        }
    }

    fn peek_rbracket(&mut self, n: usize) -> bool {
        match self.lexer.peek_nth(n) {
            Some(Ok(Token::RBracket(_))) => true,
            _ => false,
        }
    }

    fn peek_variable(&mut self, n: usize) -> bool {
        match self.lexer.peek_nth(n) {
            Some(Ok(Token::Variable(_, _))) => true,
            _ => false,
        }
    }

    fn peek_register(&mut self, n: usize) -> bool {
        match self.lexer.peek_nth(n) {
            Some(Ok(Token::Identifier(_, _))) => true,
            _ => false,
        }
    }

    fn peek_word(&mut self, n: usize) -> bool {
        match self.lexer.peek_nth(n) {
            Some(Ok(Token::Integer(_, _))) => true,
            _ => false,
        }
    }

    fn peek_abs_address(&mut self, n: usize) -> bool {
        match self.lexer.peek_nth(n) {
            Some(Ok(Token::Address(_, _, LexerAddressKind::Absolute))) => true,
            _ => false,
        }
    }

    fn peek_seg_address(&mut self, n: usize) -> bool {
        match self.lexer.peek_nth(n) {
            Some(Ok(Token::Address(_, _, LexerAddressKind::Segment))) => true,
            _ => false,
        }
    }

    fn peek_eol(&mut self, n: usize) -> bool {
        match self.lexer.peek_nth(n) {
            Some(Ok(Token::Eol(_))) => true,
            _ => false
        }
    }

    // --- read

    fn read_next(&mut self) -> Option<Token> {
        match self.lexer.next() {
            Some(Ok(t)) => Some(t),
            _ => None
        }
    }

    fn read_comma(&mut self) -> bool {
        match self.lexer.next() {
            Some(Ok(Token::Comma(_))) => true,
            _ => false,
        }
    }

    fn read_lbracket(&mut self) -> bool {
        match self.lexer.next() {
            Some(Ok(Token::LBracket(_))) => true,
            _ => false,
        }
    }

    fn read_rbracket(&mut self) -> bool {
        match self.lexer.next() {
            Some(Ok(Token::RBracket(_))) => true,
            _ => false,
        }
    }

    fn read_register(&mut self) -> Option<String> {
        match self.lexer.next() {
            Some(Ok(Token::Identifier(_, r))) => Some(r),
            _ => None,
        }
    }

    fn read_address(&mut self) -> Option<(String, AddressKind)> {
        match self.lexer.next() {
            Some(Ok(Token::Address(_, a, LexerAddressKind::Absolute))) => Some((a, Absolute)),
            Some(Ok(Token::Address(_, a, LexerAddressKind::Segment))) => Some((a, Segment)),
            _ => None,
        }
    }

    fn read_eol(&mut self) -> bool {
        match self.read_next() {
            Some(Token::Eol(_)) => true,
            _ => false
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
                let (input, op, a0, k) = $value;

                let mut lexer = Lexer::from_text(input);
                let mut nodes = vec![];
                let mut symbols = HashMap::new();
                let r = Parser::from_lexer(&mut lexer, &mut nodes, &mut symbols).next();
                assert_eq!(true, r.is_some());

                let item = r.unwrap();
                assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

                let expected = Node::Instruction(Instruction::IA(op, a0.into(), k));
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

    macro_rules! op_w_test {
        ($($name:ident: $value:expr,)*) => {
        $(
            #[test]
            fn $name() {
                let (input, op, w) = $value;

                let mut lexer = Lexer::from_text(input);
                let mut nodes = vec![];
                let mut symbols = HashMap::new();
                let r = Parser::from_lexer(&mut lexer, &mut nodes, &mut symbols).next();
                assert_eq!(true, r.is_some());

                let item = r.unwrap();
                assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

                let expected = Node::Instruction(Instruction::IW(op, w));
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

    macro_rules! op_rrr_test {
        ($($name:ident: $value:expr,)*) => {
        $(
            #[test]
            fn $name() {
                let (input, op, r0, r1, r2) = $value;

                let mut lexer = Lexer::from_text(input);
                let mut nodes = vec![];
                let mut symbols = HashMap::new();
                let r = Parser::from_lexer(&mut lexer, &mut nodes, &mut symbols).next();
                assert_eq!(true, r.is_some());

                let item = r.unwrap();
                assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

                let expected = Node::Instruction(Instruction::IRRR(op, r0.into(), r1.into(), r2.into()));
                let actual = item.unwrap();
                assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
            }
        )*
        }
    }

    macro_rules! op_rrw_test {
        ($($name:ident: $value:expr,)*) => {
        $(
            #[test]
            fn $name() {
                let (input, op, r0, r1, w0) = $value;

                let mut lexer = Lexer::from_text(input);
                let mut nodes = vec![];
                let mut symbols = HashMap::new();
                let r = Parser::from_lexer(&mut lexer, &mut nodes, &mut symbols).next();
                assert_eq!(true, r.is_some());

                let item = r.unwrap();
                assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

                let expected = Node::Instruction(Instruction::IRRW(op, r0.into(), r1.into(), w0));
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

    macro_rules! op_ra_test {
        ($($name:ident: $value:expr,)*) => {
        $(
            #[test]
            fn $name() {
                let (input, op, r0, a, k) = $value;

                let mut lexer = Lexer::from_text(input);
                let mut nodes = vec![];
                let mut symbols = HashMap::new();
                let r = Parser::from_lexer(&mut lexer, &mut nodes, &mut symbols).next();
                assert_eq!(true, r.is_some());

                let item = r.unwrap();
                assert_eq!(true, item.is_ok(), "Expected Ok(...), got {:?}", item);

                let expected = Node::Instruction(Instruction::IRA(op, r0.into(), a.into(), k));
                let actual = item.unwrap();
                assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
            }
        )*
        }
    }

    op_void_test! {
        ind:    ("IND\n", Op::Ind),
        ine:    ("INE\n", Op::Ine),
        nop:    ("NOP\n", Op::Nop),
        halt:   ("HALT\n", Op::Halt),
        panic:  ("PANIC\n", Op::Panic),
        popa:   ("POPA\n", Op::Popa),
        pusha:  ("PUSHA\n", Op::Pusha),
        iret:   ("IRET\n", Op::Iret),
        ret:    ("RET\n", Op::Ret),
        wfi:    ("WFI\n", Op::Wfi),
        xbrk:   ("XBRK\n", Op::Xbrk),
        xdbg:   ("XDBG\n", Op::Xdbg),
        xpse:   ("XPSE\n", Op::Xpse),
        xpsd:   ("XPSD\n", Op::Xpsd),
    }

    op_a_test! {
        j_s:      ("J @address\n",    Op::JS,   "address", Segment),
        j_a:      ("J &address\n",    Op::JA,   "address", Absolute),
        jeq_s:    ("JEQ @address\n",  Op::JeqS, "address", Segment),
        jeq_a:    ("JEQ &address\n",  Op::JeqA, "address", Absolute),
        jne_s:    ("JNE @address\n",  Op::JneS, "address", Segment),
        jne_a:    ("JNE &address\n",  Op::JneA, "address", Absolute),
        calls_a:  ("CALL @address\n", Op::CallS, "address", Segment),
        calla_a:  ("CALL &address\n", Op::CallA, "address", Absolute),
    }

    op_b_test! {
        int:    ("INT 12\n", Op::IntB, 12),
        mi:     ("MI 12\n", Op::MiB, 12),
        umi:    ("UMI 12\n", Op::UmiB, 12),
        xbm:    ("XBM 42\n", Op::Xbm, 42),
    }

    op_r_test! {
        inc:     ("INC  r0\n", Op::IncR, "r0"),
        dec:     ("DEC  r0\n", Op::DecR, "r0"),
        push:    ("PUSH r0\n", Op::PushR, "r0"),
        pop:     ("POP  r0\n", Op::PopR, "r0"),
        calla_r: ("CALL r0\n", Op::CallR, "r0"),
    }

    op_w_test! {
        push_w:  ("PUSH  12\n", Op::PushW, 12),
    }

    op_rr_test! {
        mov_rr:  ("MOV  r1, r0\n", Op::MovRR,  "r1", "r0"),
        add_rr:  ("ADD  r1, r0\n", Op::AddRR,  "r1", "r0"),
        and_rr:  ("AND  r1, r0\n", Op::AndRR,  "r1", "r0"),
        or_rr:   ("OR   r1, r0\n", Op::OrRR,   "r1", "r0"),
        sub_rr:  ("SUB  r1, r0\n", Op::SubRR,  "r1", "r0"),
        mul_rr:  ("MUL  r1, r0\n", Op::MulRR,  "r1", "r0"),
        cmp_rr:  ("CMP  r1, r0\n", Op::CmpRR,  "r1", "r0"),
        load_rr: ("LOAD r1, r0\n", Op::LoadRR, "r1", "r0"),
        pop_rr:  ("POP  r2, r3\n", Op::PopRR,  "r2", "r3"),
        push_rr: ("PUSH r2, r3\n", Op::PushRR, "r2", "r3"),
        stor_rr: ("STOR r1, r0\n", Op::StorRR, "r1", "r0"),
        xor_rr:  ("XOR  r1, r0\n", Op::XorRR,  "r1", "r0"),
    }

    op_rrw_test! {
        load_rrw: ("LOAD r1, [r0 + 12]\n", Op::LoadRRW,   "r1", "r0", 12),
    }

    op_rrr_test! {
        pop_rrr:  ("POP  r2, r3, r4\n", Op::PopRRR,  "r2", "r3", "r4"),
        push_rrr: ("PUSH r2, r3, r4\n", Op::PushRRR, "r2", "r3", "r4"),
    }

    op_rw_test! {
        cmp_rw: ("CMP  r1, 42\n", Op::CmpRW, "r1", 42),
        mov_rw: ("MOV  r1, 42\n", Op::MovRW, "r1", 42),
        add_rw: ("ADD  r1, 42\n", Op::AddRW, "r1", 42),
        and_rw: ("AND  r1, 42\n", Op::AndRW, "r1", 42),
        or_rw:  ("OR   r1, 42\n", Op::OrRW, "r1", 42),
        sub_rw: ("SUB  r1, 42\n", Op::SubRW, "r1", 42),
        mul_rw: ("MUL  r1, 42\n", Op::MulRW, "r1", 42),
        xor_rw: ("XOR  r1, 42\n", Op::XorRW, "r1", 42),
    }

    op_ra_test! {
        mov_ra_s: ("MOV  r1, @addr\n", Op::MovRW, "r1", "addr", Segment),
        mov_ra_a: ("MOV  r1, &addr\n", Op::MovRW, "r1", "addr", Absolute),
        load_rw:  ("LOAD r1, &addr\n", Op::LoadRW, "r1", "addr", Absolute),
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
    fn test_parse_directive_word() {
        let mut lexer = Lexer::from_text("#word var 42\n");
        let mut nodes = vec![];
        let mut symbols = HashMap::new();
        let r = Parser::from_lexer(&mut lexer, &mut nodes, &mut symbols).parse();

        assert_eq!(true, r.is_ok(), "Expected Ok(...), got {:?}", r);

        let expected = vec![
            Node::Directive(Directive::Word("var".into(), 42)),
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