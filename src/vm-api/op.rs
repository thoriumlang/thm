/// See https://github.com/thoriumlang/thm/wiki/Instructions
#[derive(Debug, PartialEq, Clone, Copy)]
pub enum Op {
    Nop = 0, // 0x00
    Halt = 1, // 0x01
    Panic = 2, // 0x02
    MovRI = 3, // 0x03
    MovRR = 4, // 0x04
    Add = 5, // 0x05
    Cmp = 6, // 0x06
    Inc = 7, // 0x07
    Dec = 8, // 0x08
    Push = 9, // 0x09
    Pop = 10, // 0x0a
    Ja = 11, // 0x0b
    Jreq = 12, // 0x0c
    Jrne = 13, // 0x0d
    Jr = 14, // 0x0e
    Stor = 15, // 0x0f
    Load = 16, // 0x10
    Call = 17, // 0x11
    Ret = 18, // 0x12
}

impl Op {
    pub fn length(&self) -> u8 {
        match self {
            Op::Nop => 1,
            Op::Halt => 1,
            Op::Panic => 1,
            Op::MovRI => 6,
            Op::MovRR => 3,
            Op::Add => 3,
            Op::Cmp => 3,
            Op::Inc => 2,
            Op::Dec => 2,
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
            3 => Self::MovRI,
            4 => Self::MovRR,
            5 => Self::Add,
            6 => Self::Cmp,
            7 => Self::Inc,
            8 => Self::Dec,
            9 => Self::Push,
            10 => Self::Pop,
            11 => Self::Ja,
            12 => Self::Jreq,
            13 => Self::Jrne,
            14 => Self::Jr,
            15 => Self::Stor,
            16 => Self::Load,
            17 => Self::Call,
            18 => Self::Ret,
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
    fn test_movri() {
        assert_eq!(Op::MovRI, Op::from(Op::MovRI.bytecode()));
    }

    #[test]
    fn test_movrr() {
        assert_eq!(Op::MovRR, Op::from(Op::MovRR.bytecode()));
    }

    #[test]
    fn test_add() {
        assert_eq!(Op::Add, Op::from(Op::Add.bytecode()));
    }

    #[test]
    fn test_cmp() {
        assert_eq!(Op::Cmp, Op::from(Op::Cmp.bytecode()));
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
}
