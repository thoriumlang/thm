mod add;
mod cmp;
mod dec;
mod halt;
mod inc;
mod jr;
mod ja;
mod jreq;
mod jrne;
mod mov_rr;
mod mov_ri;
mod nop;
mod panic;
mod pop;
mod push;
mod stor;

type Result = std::result::Result<(), &'static str>;