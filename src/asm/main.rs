use std::collections::HashMap;
use std::fs::OpenOptions;
use std::io::Write;

use clap::{App, Arg, ArgMatches, crate_authors, crate_version};

use vmlib::REG_COUNT;

use crate::address_resolver::AddressResolver;
use crate::checker::{Checker, VmConfig};
use crate::emitter::Emitter;
use crate::lexer::{Lexer, Token};
use crate::parser::Parser;

mod lexer;
mod parser;
mod address_resolver;
mod checker;
mod emitter;

fn main() {
    let matches = parse_opts();
    let input: Vec<_> = matches.values_of("input").unwrap().collect();
    let output = matches.value_of("output").unwrap();

    let mut symbols: HashMap<String, Token> = HashMap::new();
    let mut nodes = vec![];
    for f in input {
        let mut lexer = Lexer::from_file(f).unwrap();
        let mut parser = Parser::from_lexer(&mut lexer, &mut nodes, &mut symbols);
        if let Err(err) = parser.parse() {
            println!("Syntax error: {}", err);
            return;
        }
    }

    let addresses = match AddressResolver::new(&nodes).resolve() {
        Err(err) => {
            println!("Semantic error: {}", err);
            return;
        }
        Ok(addresses) => addresses,
    };

    match Checker::new(VmConfig {
        register_count: REG_COUNT as u8,
    }).check(&nodes) {
        None => (),
        Some(errors) => {
            errors.iter().for_each(|error| println!("Semantic error: {}", error));
            return;
        }
    }

    let code = Emitter::new(&nodes, &addresses).emit();

    OpenOptions::new()
        .create(true)
        .write(true)
        .open(output)
        .unwrap()
        .write_all(code.as_slice())
        .unwrap();

    println!("Wrote {} bytes to {}", code.len(), output);
}

fn parse_opts<'a>() -> ArgMatches<'a> {
    App::new("Thorium Assembler")
        .version(crate_version!())
        .author(crate_authors!())
        .about("The Thorium VM")
        .arg(
            Arg::with_name("input")
                .help("Input files")
                .long("input")
                .short("i")
                .multiple(true)
                .number_of_values(1)
                .required(true)
        )
        .arg(
            Arg::with_name("output")
                .help("Output file")
                .long("output")
                .short("o")
                .multiple(false)
                .number_of_values(1)
                .required(true)
        )
        .get_matches()
}