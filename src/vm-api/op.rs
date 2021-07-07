/// See https://github.com/thoriumlang/thm/wiki/Instructions
#[derive(Debug, PartialEq, Clone, Copy)]
pub enum Op {
    NOP = 0, // 0x00
    HALT = 1, // 0x01
    PANIC = 2, // 0x02
    MOVI = 3, // 0x03
    MOV = 4, // 0x04
    ADD = 5, // 0x05
    CMP = 6, // 0x06
    INC = 7, // 0x07
    DEC = 8, // 0x08
    PUSH = 9, // 0x09
    POP = 10, // 0x0a
    JA = 11, // 0x0b
    JREQ = 12, // 0x0c
    JRNE = 13, // 0x0d
    JR = 14, // 0x0e
    STOR = 15, // 0x0f
}

impl Op {
    pub fn length(&self) -> u8 {
        match self {
            Op::NOP => 1,
            Op::HALT => 1,
            Op::PANIC => 1,
            Op::MOVI => 6,
            Op::MOV => 3,
            Op::ADD => 3,
            Op::CMP => 3,
            Op::INC => 2,
            Op::DEC => 2,
            Op::PUSH => 2,
            Op::POP => 2,
            Op::JA => 5,
            Op::JREQ => 5,
            Op::JRNE => 5,
            Op::JR => 5,
            Op::STOR => 6,
        }
    }

    pub fn bytecode(&self) -> u8 {
        *self as u8
    }
}

impl From<u8> for Op {
    fn from(v: u8) -> Self {
        match v {
            0 => Self::NOP,
            1 => Self::HALT,
            2 => Self::PANIC,
            3 => Self::MOVI,
            4 => Self::MOV,
            5 => Self::ADD,
            6 => Self::CMP,
            7 => Self::INC,
            8 => Self::DEC,
            9 => Self::PUSH,
            10 => Self::POP,
            11 => Self::JA,
            12 => Self::JREQ,
            13 => Self::JRNE,
            14 => Self::JR,
            15 => Self::STOR,
            _ => Self::PANIC,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_nop() {
        assert_eq!(Op::NOP, Op::from(Op::NOP.bytecode()));
    }

    #[test]
    fn test_halt() {
        assert_eq!(Op::HALT, Op::from(Op::HALT.bytecode()));
    }

    #[test]
    fn test_panic() {
        assert_eq!(Op::PANIC, Op::from(Op::PANIC.bytecode()));
    }

    #[test]
    fn test_movi() {
        assert_eq!(Op::MOVI, Op::from(Op::MOVI.bytecode()));
    }

    #[test]
    fn test_mov() {
        assert_eq!(Op::MOV, Op::from(Op::MOV.bytecode()));
    }

    #[test]
    fn test_add() {
        assert_eq!(Op::ADD, Op::from(Op::ADD.bytecode()));
    }

    #[test]
    fn test_cmp() {
        assert_eq!(Op::CMP, Op::from(Op::CMP.bytecode()));
    }

    #[test]
    fn test_inc() {
        assert_eq!(Op::INC, Op::from(Op::INC.bytecode()));
    }

    #[test]
    fn test_dec() {
        assert_eq!(Op::DEC, Op::from(Op::DEC.bytecode()));
    }

    #[test]
    fn test_push() {
        assert_eq!(Op::PUSH, Op::from(Op::PUSH.bytecode()));
    }

    #[test]
    fn test_pop() {
        assert_eq!(Op::POP, Op::from(Op::POP.bytecode()));
    }

    #[test]
    fn test_ja() {
        assert_eq!(Op::JA, Op::from(Op::JA.bytecode()));
    }

    #[test]
    fn test_jreq() {
        assert_eq!(Op::JREQ, Op::from(Op::JREQ.bytecode()));
    }

    #[test]
    fn test_jrne() {
        assert_eq!(Op::JRNE, Op::from(Op::JRNE.bytecode()));
    }

    #[test]
    fn test_jr() {
        assert_eq!(Op::JR, Op::from(Op::JR.bytecode()));
    }

    #[test]
    fn test_stor() {
        assert_eq!(Op::STOR, Op::from(Op::STOR.bytecode()));
    }
}
