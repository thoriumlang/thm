mod add;
mod cmp;
mod dec;
mod halt;
mod inc;
mod j;
mod ja;
mod jeq;
mod jne;
mod load;
mod mov;
mod nop;
mod panic;
mod pop;
mod push;

type Result = std::result::Result<(), &'static str>;