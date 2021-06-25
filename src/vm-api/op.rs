use std::convert::TryFrom;

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

impl TryFrom<&str> for Op {
    type Error = String;

    fn try_from(value: &str) -> Result<Self, Self::Error> {
        match value {
            "NOP" => Ok(Self::NOP),
            "HALT" => Ok(Self::HALT),
            "PANIC" => Ok(Self::PANIC),
            "LOAD" => Ok(Self::LOAD),
            "MOV" => Ok(Self::MOV),
            "ADD" => Ok(Self::ADD),
            "CMP" => Ok(Self::CMP),
            "INC" => Ok(Self::INC),
            "DEC" => Ok(Self::DEC),
            "PUSH" => Ok(Self::PUSH),
            "POP" => Ok(Self::POP),
            "JA" => Ok(Self::JA),
            "JEQ" => Ok(Self::JEQ),
            "JNE" => Ok(Self::JNE),
            "J" => Ok(Self::J),
            _ => Err(format!("Invalid op: {}", value).to_string()),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_nop() {
        assert_eq!(Op::NOP, Op::from(Op::NOP.bytecode()));
        assert_eq!(Op::NOP, Op::try_from("NOP").unwrap());
    }

    #[test]
    fn test_halt() {
        assert_eq!(Op::HALT, Op::from(Op::HALT.bytecode()));
        assert_eq!(Op::HALT, Op::try_from("HALT").unwrap());
    }

    #[test]
    fn test_panic() {
        assert_eq!(Op::PANIC, Op::from(Op::PANIC.bytecode()));
        assert_eq!(Op::PANIC, Op::try_from("PANIC").unwrap());
    }

    #[test]
    fn test_load() {
        assert_eq!(Op::LOAD, Op::from(Op::LOAD.bytecode()));
        assert_eq!(Op::LOAD, Op::try_from("LOAD").unwrap());
    }

    #[test]
    fn test_mov() {
        assert_eq!(Op::MOV, Op::from(Op::MOV.bytecode()));
        assert_eq!(Op::MOV, Op::try_from("MOV").unwrap());
    }

    #[test]
    fn test_add() {
        assert_eq!(Op::ADD, Op::from(Op::ADD.bytecode()));
        assert_eq!(Op::ADD, Op::try_from("ADD").unwrap());
    }

    #[test]
    fn test_cmp() {
        assert_eq!(Op::CMP, Op::from(Op::CMP.bytecode()));
        assert_eq!(Op::CMP, Op::try_from("CMP").unwrap());
    }

    #[test]
    fn test_inc() {
        assert_eq!(Op::INC, Op::from(Op::INC.bytecode()));
        assert_eq!(Op::INC, Op::try_from("INC").unwrap());
    }

    #[test]
    fn test_dec() {
        assert_eq!(Op::DEC, Op::from(Op::DEC.bytecode()));
        assert_eq!(Op::DEC, Op::try_from("DEC").unwrap());
    }

    #[test]
    fn test_push() {
        assert_eq!(Op::PUSH, Op::from(Op::PUSH.bytecode()));
        assert_eq!(Op::PUSH, Op::try_from("PUSH").unwrap());
    }

    #[test]
    fn test_pop() {
        assert_eq!(Op::POP, Op::from(Op::POP.bytecode()));
        assert_eq!(Op::POP, Op::try_from("POP").unwrap());
    }

    #[test]
    fn test_ja() {
        assert_eq!(Op::JA, Op::from(Op::JA.bytecode()));
        assert_eq!(Op::JA, Op::try_from("JA").unwrap());
    }

    #[test]
    fn test_jeq() {
        assert_eq!(Op::JEQ, Op::from(Op::JEQ.bytecode()));
        assert_eq!(Op::JEQ, Op::try_from("JEQ").unwrap());
    }

    #[test]
    fn test_jne() {
        assert_eq!(Op::JNE, Op::from(Op::JNE.bytecode()));
        assert_eq!(Op::JNE, Op::try_from("JNE").unwrap());
    }

    #[test]
    fn test_j() {
        assert_eq!(Op::J, Op::from(Op::J.bytecode()));
        assert_eq!(Op::J, Op::try_from("J").unwrap());
    }
}
