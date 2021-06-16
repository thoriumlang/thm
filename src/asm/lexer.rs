use core::fmt;
use std::{fs, io};
use std::fmt::Formatter;
use std::iter::Peekable;
use std::vec::IntoIter;

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
    Label(Position, String),
    Address(Position, String),
    Register(Position, u8),
    Op(Position, String),
    Integer(Position, u32),
    Section(Position, String),
    Comma(Position),
    Eol(Position),
}

pub struct Lexer {
    raw_data: Peekable<IntoIter<char>>,
    position: Position,
}

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

    fn is_section(c: char) -> bool { c == '.' }

    fn is_label(c: char) -> bool { c == ':' }

    fn is_address(c: char) -> bool { c == '@' }

    fn label(&mut self) -> Result<String> {
        let mut label: String = "".to_string();

        loop {
            match self.raw_data.peek() {
                Some(c) if c.is_ascii_alphabetic() => {
                    label.push(*c);
                    self.next_char();
                }
                Some(c) if !label.is_empty() && (c.is_ascii_alphanumeric() || *c == '_') => {
                    label.push(*c);
                    self.next_char();
                }
                _ => {
                    if label.is_empty() {
                        return Err(format!("Expected [a-zA-Z]([_0-9a-zA-Z])* at {}", self.position).to_string());
                    }
                    return Ok(label);
                }
            }
        }
    }

    fn is_register(c: char) -> bool { c == 'r' }

    fn register(&mut self) -> Result<u8> {
        let mut reg: String = "".to_string();

        loop {
            match self.raw_data.peek() {
                Some(c) if c.is_ascii_digit() => {
                    reg.push(*c);
                    self.next_char();
                }
                _ => {
                    if reg.is_empty() {
                        return Err(format!("Expected 0-9 at {}", self.position).to_string());
                    }
                    return match reg.parse::<u8>() {
                        Ok(u8) => if u8 > 31 {
                            Err(format!("Expected 0..31 at {}", self.position).to_string())
                        } else {
                            Ok(u8)
                        },
                        Err(_) => Err(format!("Expected r0..r31 at {}", self.position).to_string())
                    };
                }
            }
        }
    }

    fn is_op(c: char) -> bool { c.is_ascii_uppercase() }

    fn op(&mut self, c: char) -> Result<String> {
        let mut op: String = c.to_string();

        loop {
            match self.raw_data.peek() {
                Some(c) if c.is_ascii_uppercase() => {
                    op.push(*c);
                    self.next_char();
                }
                Some(c) if !op.is_empty() && (c.is_ascii_digit() || *c == '_') => {
                    op.push(*c);
                    self.next_char();
                }
                _ => return Ok(op)
            }
        }
    }

    fn is_hex_int(c: char) -> bool { c == 'x' }

    fn hex_int(&mut self) -> Result<u32> {
        let mut int: String = "".to_string();

        loop {
            match self.raw_data.peek() {
                Some('_') => {
                    self.next_char();
                    continue;
                }
                Some(c) if c.is_ascii_hexdigit() => {
                    int.push(*c);
                    self.next_char();
                }
                _ => {
                    if int.is_empty() {
                        return Err(format!("Expected xA-Z([A-Z0-9])* at {}", self.position).to_string());
                    }
                    return match u32::from_str_radix(int.as_str(), 16) {
                        Ok(int) => Ok(int),
                        Err(_) => return Err(format!("Not a valid hex number at {}", self.position).to_string()),
                    };
                }
            }
        }
    }

    fn is_bin_int(c: char) -> bool { c == 'b' }

    fn bin_int(&mut self) -> Result<u32> {
        let mut int: String = "".to_string();

        loop {
            match self.raw_data.peek() {
                Some('_') => {
                    self.next_char();
                    continue;
                }
                Some('0') => {
                    int.push('0');
                    self.next_char();
                }
                Some('1') => {
                    int.push('1');
                    self.next_char();
                }
                _ => {
                    if int.is_empty() {
                        return Err(format!("Expected b[0,1]+ at {}", self.position).to_string());
                    }
                    return match u32::from_str_radix(int.as_str(), 2) {
                        Ok(int) => Ok(int),
                        Err(_) => return Err(format!("Not a valid bin number at {}", self.position).to_string()),
                    };
                }
            }
        }
    }

    fn is_dec_int(c: char) -> bool { c.is_ascii_digit() }

    fn dec_int(&mut self, c: char) -> Result<u32> {
        let mut int: String = c.to_string();

        loop {
            match self.raw_data.peek() {
                Some('_') => {
                    self.next_char();
                    continue;
                }
                Some(c) if c.is_ascii_digit() => {
                    int.push(*c);
                    self.next_char();
                }
                _ => return match int.parse::<u32>() {
                    Ok(u32) => Ok(u32),
                    Err(_) => Err(format!("Expected r0..r31 at {}", self.position).to_string()),
                }
            }
        }
    }
}

type Result<T> = std::result::Result<T, String>;

impl Iterator for Lexer {
    type Item = Result<Token>;

    fn next(&mut self) -> Option<Self::Item> {
        loop {
            let start_line = self.position.line;
            let start_column = self.position.column;
            match self.next_char() {
                Some('/') => {
                    match self.next_char() {
                        Some('/') => {
                            loop {
                                match self.next_char() {
                                    None | Some('\n') => return Some(Ok(Token::Eol(Position::new(start_line, start_column)))),
                                    _ => continue,
                                }
                            }
                        }
                        _ => return Some(Err(format!("Unexpected `/` at {}", self.position))),
                    }
                }
                Some('\n') => return Some(Ok(Token::Eol(Position::new(start_line, start_column)))),
                Some(',') => return Some(Ok(Token::Comma(Position::new(start_line, start_column)))),
                Some(c) if c.is_whitespace() => continue,
                Some(c) if Self::is_section(c) => return Some(self.label().map(|s| Token::Section(Position::new(start_line, start_column), s))),
                Some(c) if Self::is_label(c) => return Some(self.label().map(|s| Token::Label(Position::new(start_line, start_column), s))),
                Some(c) if Self::is_address(c) => return Some(self.label().map(|s| Token::Address(Position::new(start_line, start_column), s))),
                Some(c) if Self::is_register(c) => return Some(self.register().map(|s| Token::Register(Position::new(start_line, start_column), s))),
                Some(c) if Self::is_op(c) => return Some(self.op(c).map(|s| Token::Op(Position::new(start_line, start_column), s))),
                Some(c) if Self::is_hex_int(c) => return Some(self.hex_int().map(|s| Token::Integer(Position::new(start_line, start_column), s))),
                Some(c) if Self::is_bin_int(c) => return Some(self.bin_int().map(|s| Token::Integer(Position::new(start_line, start_column), s))),
                Some(c) if Self::is_dec_int(c) => return Some(self.dec_int(c).map(|s| Token::Integer(Position::new(start_line, start_column), s))),
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
    fn test_address() {
        let r = Lexer::from_text(" @label ").next();
        assert_eq!(true, r.is_some(), "Expected Some(...), got {:?}", r);

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(Token::Address), got {:?}", item);

        let expected = Token::Address(Position::new(1, 2), "label".to_string());
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_register() {
        let r = Lexer::from_text(" r12 ").next();
        assert_eq!(true, r.is_some(), "Expected Some(...), got {:?}", r);

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(Token::Register), got {:?}", item);

        let expected = Token::Register(Position::new(1, 2), 12);
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_register_invalid() {
        let r = Lexer::from_text(" r123 ").next();
        assert_eq!(true, r.is_some(), "Expected Some(...), got {:?}", r);

        let item = r.unwrap();
        assert_eq!(true, item.is_err(), "Expected Err(...), got {:?}", item);

        let expected = "Expected 0..31 at 1:6".to_string();
        let actual = item.err().unwrap();
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
        let r = Lexer::from_text(" xF_F ").next();
        assert_eq!(true, r.is_some(), "Expected Some(...), got {:?}", r);

        let item = r.unwrap();
        assert_eq!(true, item.is_ok(), "Expected Ok(Token::Integer), got {:?}", item);

        let expected = Token::Integer(Position::new(1, 2), 255);
        let actual = item.unwrap();
        assert_eq!(expected, actual, "Expected {:?}, got {:?}", expected, actual);
    }

    #[test]
    fn test_bin_int() {
        let r = Lexer::from_text(" b10_1 ").next();
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
    fn test_several_tokens() {
        let mut lexer = Lexer::from_text(".section LOAD r1, @label :label2 xFF b0100 1204 //comment \n");
        let tokens = vec![
            Token::Section(Position::new(1, 1), "section".to_string()),
            Token::Op(Position::new(1, 10), "LOAD".to_string()),
            Token::Register(Position::new(1, 15), 1),
            Token::Comma(Position::new(1, 17)),
            Token::Address(Position::new(1, 19), "label".to_string()),
            Token::Label(Position::new(1, 26), "label2".to_string()),
            Token::Integer(Position::new(1, 34), 255),
            Token::Integer(Position::new(1, 38), 4),
            Token::Integer(Position::new(1, 44), 1204),
            Token::Eol(Position::new(1, 49)),
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
