use std::fs::OpenOptions;
use std::io::Write;

use clap::{App, Arg, ArgMatches, crate_authors, crate_version};

use vmlib::REG_COUNT;

use crate::address_resolver::AddressResolver;
use crate::checker::{Checker, VmConfig};
use crate::emitter::Emitter;
use crate::lexer::Lexer;
use crate::parser::Parser;

mod lexer;
mod parser;
mod address_resolver;
mod checker;
mod emitter;

fn main() {
    let matches = parse_opts();
    let input = matches.value_of("input").unwrap();
    let output = matches.value_of("output").unwrap();

    println!("input: {}\noutput: {}", input, output);

    let mut lexer = Lexer::from_file(input).unwrap();
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

    let vm_config = VmConfig {
        register_count: REG_COUNT as u8,
    };
    let checker = Checker::new(vm_config);
    match checker.check(&nodes) {
        None => (),
        Some(errors) => {
            errors.iter().for_each(|error| println!("Semantic error: {}", error));
            return;
        }
    }

    let emitter = Emitter::new(&nodes, &addresses);
    let code = emitter.emit();

    let mut file = OpenOptions::new()
        .create(true)
        .write(true)
        .open(output)
        .unwrap();

    file.write_all(code.as_slice()).unwrap();
    println!("Wrote {} bytes to {}", code.len(), output);
}

fn parse_opts<'a>() -> ArgMatches<'a> {
    App::new("Thorium Assembler")
        .version(crate_version!())
        .author(crate_authors!())
        .about("The Thorium VM")
        .arg(
            Arg::with_name("input")
                .help("Input file")
                .required(true)
                .index(1),
        )
        .arg(
            Arg::with_name("output")
                .help("Output file")
                .required(true)
                .index(2)
        )
        .get_matches()
}