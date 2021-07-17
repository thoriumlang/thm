/// See https://github.com/thoriumlang/thm/wiki/Instructions
#[derive(Debug, PartialEq, Clone, Copy)]
pub enum Op {
    Nop = 0, // 0x00
    Halt = 1, // 0x01
    Panic = 2, // 0x02
    MovRW = 3, // 0x03
    MovRR = 4, // 0x04
    AddRR = 5, // 0x05
    AddRW = 6, // 0x06
    SubRR = 7, // 0x07
    SubRW = 8, // 0x08
    MulRR = 9, // 0x09
    MulRW = 10, // 0x0a
    Inc = 15, // 0x0f
    Dec = 16, // 0x10
    Cmp = 24, // 0x18
    Push = 25, // 0x19
    Pop = 26, // 0x1a
    Ja = 27, // 0x1b
    Jreq = 28, // 0x1c
    Jrne = 29, // 0x1d
    Jr = 30, // 0x1e
    Stor = 31, // 0x1f
    Load = 32, // 0x20
    Call = 33, // 0x21
    Ret = 34, // 0x22
    Xbm = 35, // 0x23
}

impl Op {
    pub fn length(&self) -> u8 {
        match self {
            Op::Nop => 1,
            Op::Halt => 1,
            Op::Panic => 1,
            Op::MovRW => 6,
            Op::MovRR => 3,
            Op::AddRR => 3,
            Op::AddRW => 6,
            Op::SubRR => 3,
            Op::SubRW => 6,
            Op::MulRR => 3,
            Op::MulRW => 6,
            Op::Inc => 2,
            Op::Dec => 2,
            Op::Cmp => 3,
            Op::Push => 2,
            Op::Pop => 2,
            Op::Ja => 3,
            Op::Jreq => 5,
            Op::Jrne => 5,
            Op::Jr => 5,
            Op::Stor => 3,
            Op::Load => 3,
            Op::Call => 5,
            Op::Ret => 1,
            Op::Xbm => 2,
        }
    }

    pub fn bytecode(&self) -> u8 {
        *self as u8
    }
}

impl From<u8> for Op {
    fn from(v: u8) -> Self {
        match v {
            0 => Self::Nop,
            1 => Self::Halt,
            2 => Self::Panic,
            3 => Self::MovRW,
            4 => Self::MovRR,
            5 => Self::AddRR,
            6 => Self::AddRW,
            7 => Self::SubRR,
            8 => Self::SubRW,
            9 => Self::MulRR,
            10 => Self::MulRW,
            15 => Self::Inc,
            16 => Self::Dec,
            24 => Self::Cmp,
            25 => Self::Push,
            26 => Self::Pop,
            27 => Self::Ja,
            28 => Self::Jreq,
            29 => Self::Jrne,
            30 => Self::Jr,
            31 => Self::Stor,
            32 => Self::Load,
            33 => Self::Call,
            34 => Self::Ret,
            35 => Self::Xbm,
            _ => Self::Panic,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_nop() {
        assert_eq!(Op::Nop, Op::from(Op::Nop.bytecode()));
    }

    #[test]
    fn test_halt() {
        assert_eq!(Op::Halt, Op::from(Op::Halt.bytecode()));
    }

    #[test]
    fn test_panic() {
        assert_eq!(Op::Panic, Op::from(Op::Panic.bytecode()));
    }

    #[test]
    fn test_movrw() {
        assert_eq!(Op::MovRW, Op::from(Op::MovRW.bytecode()));
    }

    #[test]
    fn test_movrr() {
        assert_eq!(Op::MovRR, Op::from(Op::MovRR.bytecode()));
    }

    #[test]
    fn test_addrr() {
        assert_eq!(Op::AddRR, Op::from(Op::AddRR.bytecode()));
    }

    #[test]
    fn test_addrw() {
        assert_eq!(Op::AddRW, Op::from(Op::AddRW.bytecode()));
    }

    #[test]
    fn test_subrr() {
        assert_eq!(Op::SubRR, Op::from(Op::SubRR.bytecode()));
    }

    #[test]
    fn test_subrw() {
        assert_eq!(Op::SubRW, Op::from(Op::SubRW.bytecode()));
    }

    #[test]
    fn test_mulrr() {
        assert_eq!(Op::MulRR, Op::from(Op::MulRR.bytecode()));
    }

    #[test]
    fn test_mulrw() {
        assert_eq!(Op::MulRW, Op::from(Op::MulRW.bytecode()));
    }

    #[test]
    fn test_inc() {
        assert_eq!(Op::Inc, Op::from(Op::Inc.bytecode()));
    }

    #[test]
    fn test_dec() {
        assert_eq!(Op::Dec, Op::from(Op::Dec.bytecode()));
    }

    #[test]
    fn test_cmp() {
        assert_eq!(Op::Cmp, Op::from(Op::Cmp.bytecode()));
    }

    #[test]
    fn test_push() {
        assert_eq!(Op::Push, Op::from(Op::Push.bytecode()));
    }

    #[test]
    fn test_pop() {
        assert_eq!(Op::Pop, Op::from(Op::Pop.bytecode()));
    }

    #[test]
    fn test_ja() {
        assert_eq!(Op::Ja, Op::from(Op::Ja.bytecode()));
    }

    #[test]
    fn test_jreq() {
        assert_eq!(Op::Jreq, Op::from(Op::Jreq.bytecode()));
    }

    #[test]
    fn test_jrne() {
        assert_eq!(Op::Jrne, Op::from(Op::Jrne.bytecode()));
    }

    #[test]
    fn test_jr() {
        assert_eq!(Op::Jr, Op::from(Op::Jr.bytecode()));
    }

    #[test]
    fn test_stor() {
        assert_eq!(Op::Stor, Op::from(Op::Stor.bytecode()));
    }

    #[test]
    fn test_load() {
        assert_eq!(Op::Load, Op::from(Op::Load.bytecode()));
    }

    #[test]
    fn test_call() {
        assert_eq!(Op::Call, Op::from(Op::Call.bytecode()));
    }

    #[test]
    fn test_ret() {
        assert_eq!(Op::Ret, Op::from(Op::Ret.bytecode()));
    }

    #[test]
    fn test_xbm() {
        assert_eq!(Op::Xbm, Op::from(Op::Xbm.bytecode()));
    }
}
