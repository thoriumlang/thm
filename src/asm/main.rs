use crate::address_resolver::AddressResolver;
use crate::lexer::Lexer;
use crate::parser::Parser;
use crate::emitter::Emitter;

mod lexer;
mod parser;
mod address_resolver;
mod emitter;

fn main() {
    let mut lexer = Lexer::from_file("examples/fibonacci.a").unwrap();
    let mut parser = Parser::from_lexer(&mut lexer);

    let nodes = parser.parse();
    if nodes.is_err() {
        println!("Syntax error: {}", nodes.err().unwrap());
        return;
    }
    let nodes = nodes.unwrap();

    let resolver = AddressResolver::new(&nodes);
    let addresses = resolver.resolve();

    if addresses.is_err() {
        println!("Semantic error: {}", addresses.err().unwrap());
        return;
    }
    let addresses = addresses.unwrap();

    let emitter = Emitter::new(&nodes, &addresses);
    println!("{:?}", emitter.emit());
}
