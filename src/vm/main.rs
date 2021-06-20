use vm::VM;
use std::fs::OpenOptions;
use std::io::Read;

mod vm;

fn main() {
    let mut file = OpenOptions::new()
        .create(false)
        .read(true)
        .open("target/fibonacci.bin")
        .unwrap();

    let mut bytes: Vec<u8> = vec![];
    file.read_to_end(&mut bytes).unwrap();

    let mut vm = VM::new();
    vm.program = bytes;
    vm.run();

    println!("fibo({}): {}", vm.registers[0], vm.registers[3])
}
