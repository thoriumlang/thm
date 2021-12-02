use core::fmt;
use std::{fs, io};
use std::fmt::Formatter;
use std::iter::Peekable;
use std::str::FromStr;
use std::vec::IntoIter;
use crate::lexer::AddressKind::{Absolute, Segment};

#[derive(Debug, PartialEq)]
pub struct Position {
    line: u16,
    column: u16,
}

impl Position {
    fn new(line: u16, column: u16) -> Position {
        Position { line, column }
    }
}

impl fmt::Display for Position {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        write!(f, "{}:{}", self.line, self.column)
    }
}

#[derive(Debug, PartialEq)]
pub enum Token {
    Address(Position, String, AddressKind),
    Comma(Position),
    Directive(Position, String),
    Equal(Position),
    Eol(Position),
    Identifier(Position, String),
    Integer(Position, u32),
    Label(Position, String),
    Op(Position, String),
    Section(Position, String),
    Variable(Position, String),
}

#[derive(Debug, PartialEq)]
pub enum AddressKind {
    Absolute,
    Segment,
}

impl Token {
    pub fn position(&self) -> &Position {
        match &self {
            Token::Address(p, _, _) => p,
            Token::Comma(p) => p,
            Token::Directive(p, _) => p,
            Token::Equal(p) => p,
            Token::Eol(p) => p,
            Token::Identifier(p, _) => p,
            Token::Integer(p, _) => p,
            Token::Label(p, _) => p,
            Token::Op(p, _) => p,
            Token::Section(p, _) => p,
            Token::Variable(p, _) => p,
        }
    }
}

pub struct Lexer {
    raw_data: Peekable<IntoIter<char>>,
    position: Position,
}

type Result<T> = std::result::Result<T, String>;

impl Lexer {
    pub fn from_text(text: &str) -> Self {
        Lexer {
            raw_data: text.chars().collect::<Vec<_>>().into_iter().peekable(),
            position: Position {
                line: 1,
                column: 1,
            },
        }
    }

    pub fn from_file(file_path: &str) -> io::Result<Self> {
        Ok(Self::from_text(&fs::read_to_string(file_path)?))
    }

    fn next_char(&mut self) -> Option<char> {
        let char = self.raw_data.next();
        match char {
            Some('\n') => {
                self.position.column = 1;
                self.position.line += 1;
            }
            _ => {
                self.position.column += 1;
            }
        };
        return char;
    }

    fn is_address(c: char) -> bool { c == '@' }

    fn is_absolute_address(c: char) -> bool { c == '&' }

    fn is_directive(c: char) -> bool { c == '#' }

    fn is_identifier(c: char) -> bool {
        c.is_ascii_lowercase()
    }

    fn is_label(c: char) -> bool { c == ':' }

    fn is_number(c: char) -> bool {
        c.is_ascii_digit()
    }

    fn is_op(c: char) -> bool { c.is_ascii_uppercase() }

    fn is_section(c: char) -> bool { c == '.' }

    fn is_variable(c: char) -> bool {
        c == '$'
    }

    /// Parses and return a string in the form `[a-z][A-Za-z0-9_]+` where the first char comes
    /// as parameter (but _may_ be empty).
    fn identifier(&mut self, c: char) -> Result<String> {
        let mut identifier: String = match c {
            '\0' => "".to_string(),
            c => c.to_string()
        };

        loop {
            match self.raw_data.peek() {
                Some(c) if c.is_ascii_lowercase() => {
                    identifier.push(*c);
                    self.next_char();
                }
                Some(c) if !identifier.is_empty() && (c.is_ascii_alphanumeric() || *c == '_') => {
                    identifier.push(*c);
                    self.next_char();
                }
                _ => return Ok(identifier)
            }
        }
    }

    /// Parses a string in the form `[A-Z][A-Za-z0-9_]*` where the first char comes as parameter.
    fn op(&mut self, c: char) -> Result<String> {
        let mut op: String = c.to_string();

        loop {
            match self.raw_data.peek() {
                Some(c) if c.is_ascii_uppercase() || c.is_ascii_digit() || *c == '_' => {
                    op.push(*c);
                    self.next_char();
                }
                _ => return Ok(op)
            }
        }
    }

    /// Parses a string in one of the forms `0b[01]+` or `0x[0-9A-Fa-f]+` or `[0-9_]+` where the
    /// first char comes as parameter.
    fn number(&mut self, c: char) -> Result<u32> {
        if c == '0' {
            match self.raw_data.peek() {
                Some(n) => match n {
                    'x' => self.parse_hex(),
                    'b' => self.parse_bin(),
                    _ => self.parse_dec(c),
                },
                None => self.parse_dec(c),
            }
        } else {
            self.parse_dec(c)
        }
    }

    fn parse_hex(&mut self) -> Result<u32> {
        self.next_char(); // consume the 'x'
        let mut int: String = "".to_string();
        loop {
            match self.raw_data.peek() {
                Some('_') => {
                    self.next_char();
                    continue;
                }
                Some(n) if n.is_ascii_hexdigit() => {
                    int.push(*n);
                    self.next_char();
                }
                Some(_) | None => {
                    if int.is_empty() {
                        return Err(format!("Expected 0x[0-9A-Fa-z]+ at {}", self.position).to_string());
                    }
                    return match u32::from_str_radix(int.as_str(), 16) {
                        Ok(i) => Ok(i),
                        Err(_) => Err(format!("Not a valid binary number at {}", self.position).to_string()),
                    };
                }
            }
        }
    }

    fn parse_bin(&mut self) -> Result<u32> {
        self.next_char(); // consume the 'b'
        let mut int: String = "".to_string();
        loop {
            match self.raw_data.peek() {
                Some('_') => {
                    self.next_char();
                    continue;
                }
                Some('0') => {
                    self.next_char();
                    int.push('0');
                }
                Some('1') => {
                    self.next_char();
                    int.push('1');
                }
                Some(_) | None => {
                    if int.is_empty() {
                        return Err(format!("Expected 0b[0,1]+ at {}", self.position).to_string());
                    }
                    return match u32::from_str_radix(int.as_str(), 2) {
                        Ok(i) => Ok(i),
                        Err(_) => Err(format!("Not a valid binary number at {}", self.position).to_string()),
                    };
                }
            }
        }
    }

    fn parse_dec(&mut self, c: char) -> Result<u32> {
        let mut int: String = c.to_string();

        loop {
            match self.raw_data.peek() {
                Some('_') => {
                    self.next_char();
                    continue;
                }
                Some(n) if n.is_numeric() => {
                    int.push(*n);
                    self.next_char();
                }
                Some(_) | None => {
                    return match u32::from_str(int.as_str()) {
                        Ok(i) => Ok(i),
                        Err(_) => Err(format!("Not a valid number at {}", self.position).to_string()),
                    };
                }
            }
        }
    }
}

impl Iterator for Lexer {
    type Item = Result<Token>;

    fn next(&mut self) -> Option<Self::Item> {
        loop {
            let position = Position::new(self.position.line, self.position.column);
            match self.next_char() { // todo next_char could return char kind as well as raw char
                Some('/') => {
                    match self.next_char() {
                        Some('/') => {
                            loop {
                                match self.next_char() {
                                    None | Some('\n') => return Some(Ok(Token::Eol(position))),
                                    _ => continue,
                                }
                            }
                        }
                        _ => return Some(Err(format!("Unexpected `/` at {}", self.position))),
                    }
                }
                Some(',') => return Some(Ok(Token::Comma(position))),
                Some('\n') => return Some(Ok(Token::Eol(position))),
                Some('=') => return Some(Ok(Token::Equal(position))),
                Some(c) if c.is_whitespace() => continue,
                Some(c) if Self::is_absolute_address(c) => return Some(self.identifier('\0').map(|s| Token::Address(position, s, Absolute))),
                Some(c) if Self::is_address(c) => return Some(self.identifier('\0').map(|s| Token::Address(position, s, Segment))),
                Some(c) if Self::is_directive(c) => return Some(self.identifier('\0').map(|s| Token::Directive(position, s))),
                Some(c) if Self::is_identifier(c) => return Some(self.identifier(c).map(|s| Token::Identifier(position, s))),
                Some(c) if Self::is_number(c) => return Some(self.number(c).map(|n| Token::Integer(position, n))),
                Some(c) if Self::is_label(c) => return Some(self.identifier('\0').map(|s| Token::Label(position, s))),
                Some(c) if Self::is_op(c) => return Some(self.op(c).map(|s| Token::Op(position, s))),
                Some(c) if Self::is_section(c) => return Some(self.identifier('\0').map(|s| Token::Section(position, s))),
                Some(c) if Self::is_variable(c) => return Some(self.identifier(c).map(|s| Token::Variable(position, s))),
                Some(c) => return Some(Err(format!("Unexpected `{}`", c))),
                None => return None,
            }
        }
    }
}


#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_unexpected_char() {
        let r = Lexer::from_text("_").next();
        assert_eq!(true, r.is_some());
        let err = r.unwrap();
        assert_eq!(true, err.is_err());
        assert_eq!("Unexpected `_`", err.err().unwrap());
    }

    #[test]
    fn test_eol() {
        let r = Lexer::from_text(" \n ").next();
        assert_eq!(true, r.is_some(), "Expected Some(...), got {:?}", r);

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(Token::Eol), got {:?}", item);

        let expected = Token::Eol(Position::new(1, 2));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_eol_after_comment() {
        let r = Lexer::from_text(" // comment \n ").next();
        assert_eq!(true, r.is_some(), "Expected Some(...), got {:?}", r);

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(Token::Eol), got {:?}", item);

        let expected = Token::Eol(Position::new(1, 2));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_comment() {
        let r = Lexer::from_text(" // comment ").next();
        assert_eq!(true, r.is_some(), "Expected Some(...), got {:?}", r);

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(Token::Eol), got {:?}", item);

        let expected = Token::Eol(Position::new(1, 2));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_comment_invalid() {
        let r = Lexer::from_text(" / ").next();
        assert_eq!(true, r.is_some(), "Expected Some(...), got {:?}", r);

        let item = r.unwrap();
        assert_eq!(true, item.is_err(), "Expected Err(...), got {:?}", item);

        let expected = "Unexpected `/` at 1:4".to_string();
        let actual = item.err().unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_comma() {
        let r = Lexer::from_text(" , ").next();
        assert_eq!(true, r.is_some(), "Expected Some(...), got {:?}", r);

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(Token::Comma), got {:?}", item);

        let expected = Token::Comma(Position::new(1, 2));
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_directive() {
        let r = Lexer::from_text(" #base 12 ").next();
        assert_eq!(true, r.is_some(), "Expected Some(...), got {:?}", r);

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(Token::Directive), got {:?}", item);

        let expected = Token::Directive(Position::new(1, 2), "base".to_string());
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_section() {
        let r = Lexer::from_text(" .section ").next();
        assert_eq!(true, r.is_some(), "Expected Some(...), got {:?}", r);

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(Token::Section), got {:?}", item);

        let expected = Token::Section(Position::new(1, 2), "section".to_string());
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_label() {
        let r = Lexer::from_text(" :label ").next();
        assert_eq!(true, r.is_some(), "Expected Some(...), got {:?}", r);

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(Token::Label), got {:?}", item);

        let expected = Token::Label(Position::new(1, 2), "label".to_string());
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_address_segment() {
        let r = Lexer::from_text(" @label ").next();
        assert_eq!(true, r.is_some(), "Expected Some(...), got {:?}", r);

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(Token::Address), got {:?}", item);

        let expected = Token::Address(Position::new(1, 2), "label".to_string(), Segment);
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_address_absolute() {
        let r = Lexer::from_text(" &label ").next();
        assert_eq!(true, r.is_some(), "Expected Some(...), got {:?}", r);

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(Token::Address), got {:?}", item);

        let expected = Token::Address(Position::new(1, 2), "label".to_string(), Absolute);
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_identifier() {
        let r = Lexer::from_text(" identifier ").next();
        assert_eq!(true, r.is_some(), "Expected Some(...), got {:?}", r);

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(Token::Identifier), got {:?}", item);

        let expected = Token::Identifier(Position::new(1, 2), "identifier".to_string());
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_op() {
        let r = Lexer::from_text(" LOAD ").next();
        assert_eq!(true, r.is_some(), "Expected Some(...), got {:?}", r);

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(Token::Op), got {:?}", item);

        let expected = Token::Op(Position::new(1, 2), "LOAD".to_string());
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_hex_int() {
        let r = Lexer::from_text(" 0xF_F ").next();
        assert_eq!(true, r.is_some(), "Expected Some(...), got {:?}", r);

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(Token::Integer), got {:?}", item);

        let expected = Token::Integer(Position::new(1, 2), 255);
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_bin_int() {
        let r = Lexer::from_text(" 0b10_1 ").next();
        assert_eq!(true, r.is_some(), "Expected Some(...), got {:?}", r);

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(Token::Integer), got {:?}", item);

        let expected = Token::Integer(Position::new(1, 2), 5);
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_dec_int() {
        let r = Lexer::from_text(" 1_024 ").next();
        assert_eq!(true, r.is_some(), "Expected Some(...), got {:?}", r);

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(Token::Integer), got {:?}", item);

        let expected = Token::Integer(Position::new(1, 2), 1024);
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_dec_0() {
        let r = Lexer::from_text(" 0 ").next();
        assert_eq!(true, r.is_some(), "Expected Some(...), got {:?}", r);

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(Token::Integer), got {:?}", item);

        let expected = Token::Integer(Position::new(1, 2), 0);
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_several_tokens() {
        let mut lexer = Lexer::from_text(".section LOAD r1, @label &label_abs :label2 0xFF 0b0100 1204 //comment \n");
        let tokens = vec![
            Token::Section(Position::new(1, 1), "section".to_string()),
            Token::Op(Position::new(1, 10), "LOAD".to_string()),
            Token::Identifier(Position::new(1, 15), "r1".to_string()),
            Token::Comma(Position::new(1, 17)),
            Token::Address(Position::new(1, 19), "label".to_string(), Segment),
            Token::Address(Position::new(1, 26), "label_abs".to_string(), Absolute),
            Token::Label(Position::new(1, 37), "label2".to_string()),
            Token::Integer(Position::new(1, 45), 255),
            Token::Integer(Position::new(1, 50), 4),
            Token::Integer(Position::new(1, 57), 1204),
            Token::Eol(Position::new(1, 62)),
        ];

        let mut i = 0;
        loop {
            match lexer.next() {
                None => {
                    assert_eq!(tokens.len(), i);
                    return;
                }
                Some(token) => {
                    assert_eq!(true, token.is_ok(), "{:?}", token);
                    let expected = &tokens[i];
                    let actual = token.unwrap();
                    assert_eq!(*expected, actual, "Expected {:?}, got {:?}", expected, actual)
                }
            }
            i += 1;
        }
    }
}
