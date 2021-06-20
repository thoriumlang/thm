use crate::address_resolver::AddressResolver;
use crate::lexer::{Lexer, VmConfig};
use crate::parser::Parser;
use crate::emitter::Emitter;
use std::fs::OpenOptions;
use std::io::Write;
use vmlib::REG_COUNT;

mod lexer;
mod parser;
mod address_resolver;
mod emitter;

fn main() {
    let vm_config = VmConfig {
        register_count: REG_COUNT as u8,
    };
    let mut lexer = Lexer::from_file("examples/fibonacci.a", vm_config).unwrap();
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
    let code = emitter.emit();

    let mut file = OpenOptions::new()
        .create(true)
        .write(true)
        .open("target/fibonacci.bin")
        .unwrap();

    file.write_all(code.as_slice()).unwrap();
}
