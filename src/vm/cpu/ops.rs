mod add;
mod cmp;
mod dec;
mod halt;
mod inc;
mod jr;
mod ja;
mod jreq;
mod jrne;
mod mov;
mod movi;
mod nop;
mod panic;
mod pop;
mod push;

type Result = std::result::Result<(), &'static str>;