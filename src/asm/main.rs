use crate::lexer::Lexer;
use crate::parser::Parser;

mod lexer;
mod parser;

fn main() {
    let mut lexer = Lexer::from_file("examples/fibonacci.a").unwrap();
    let mut parser = Parser::from_lexer(&mut lexer);

    println!("{:?}", parser.parse());
}
