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

    let n = 5;

    let mut vm = VM::new();
    vm.registers[0] = n;
    vm.program = bytes;
    vm.run();

    println!("fibo({}): {}", n, vm.registers[3])
}
