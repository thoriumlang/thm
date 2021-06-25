/// See https://github.com/thoriumlang/thm/wiki/Instructions
#[derive(Debug, PartialEq, Clone, Copy)]
pub enum Op {
    NOP,
    HALT,
    PANIC,
    LOAD,
    MOV,
    ADD,
    CMP,
    INC,
    DEC,
    PUSH,
    POP,
    JA,
    JEQ,
    JNE,
    J,
}

impl Op {
    pub fn length(&self) -> u8 {
        return match self {
            Op::NOP => 1,
            Op::HALT => 1,
            Op::PANIC => 1,
            Op::LOAD => 6,
            Op::MOV => 3,
            Op::ADD => 3,
            Op::CMP => 3,
            Op::INC => 2,
            Op::DEC => 2,
            Op::PUSH => 2,
            Op::POP => 2,
            Op::JA => 5,
            Op::JEQ => 5,
            Op::JNE => 5,
            Op::J => 5,
        };
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
            3 => Self::LOAD,
            4 => Self::MOV,
            5 => Self::ADD,
            6 => Self::CMP,
            7 => Self::INC,
            8 => Self::DEC,
            9 => Self::PUSH,
            10 => Self::POP,
            11 => Self::JA,
            12 => Self::JEQ,
            13 => Self::JNE,
            14 => Self::J,
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
    fn test_load() {
        assert_eq!(Op::LOAD, Op::from(Op::LOAD.bytecode()));
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
    fn test_jeq() {
        assert_eq!(Op::JEQ, Op::from(Op::JEQ.bytecode()));
    }

    #[test]
    fn test_jne() {
        assert_eq!(Op::JNE, Op::from(Op::JNE.bytecode()));
    }

    #[test]
    fn test_j() {
        assert_eq!(Op::J, Op::from(Op::J.bytecode()));
    }
}
