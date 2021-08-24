use std::str::FromStr;
use std::sync::{Arc, RwLock};
use std::thread;
use std::thread::JoinHandle;

use serde::{Deserialize, Serialize};
use warp;
use warp::Filter;

use vmlib::{INT_API, INTERRUPTS_COUNT, MAX_REGISTER, REG_CS, REG_PC, REG_SP, STACK_MAX_ADDRESS, REST_API_START};

use crate::cpu::CPU;
use crate::interrupts::PIC;
use crate::memory::Memory;

pub struct RestApi {}

#[derive(Deserialize, Serialize)]
struct ErrorResponse {
    code: u8,
    message: String,
}

const ERROR_INVALID_ADDRESS: u8 = 0;
const ERROR_INVALID_REGISTER: u8 = 1;
const ERROR_INVALID_INTERRUPT: u8 = 2;

#[derive(Deserialize, Serialize)]
struct GetMemoryRequest {
    from: u32,
    to: u32,
}

#[derive(Deserialize, Serialize)]
struct GetMemoryResponse {
    bytes: Vec<u8>,
}

#[derive(Deserialize, Serialize)]
struct StepRequest {
    count: Option<u32>,
}

#[derive(Deserialize, Serialize)]
struct Registers {
    pc: u32,
}

#[derive(Deserialize, Serialize)]
struct Meta {
    steps_count: u64,
}

#[derive(Deserialize, Serialize)]
struct StepResponse {
    running: bool,
    registers: Registers,
    meta: Meta,
}

#[derive(Deserialize, Serialize)]
struct InterruptResponse {
    ok: bool,
}

impl RestApi {
    pub fn new(cpu: Arc<RwLock<CPU>>, memory: Arc<Memory>, pic: Arc<PIC>) -> JoinHandle<()> {
        return thread::Builder::new().name("api".into()).spawn(|| {
            tokio::runtime::Builder::new_multi_thread()
                .enable_all()
                .build()
                .unwrap()
                .block_on(async move {
                    let cpu_filter = warp::any().map(move || cpu.clone());
                    let memory_filter = warp::any().map(move || memory.clone());
                    let pic_filter = warp::any().map(move || pic.clone());

                    let get_register = warp::get()
                        .and(warp::path("cpu"))
                        .and(warp::path("registers"))
                        .and(warp::path::param::<String>())
                        .and(cpu_filter.clone())
                        .and(memory_filter.clone())
                        .and(pic_filter.clone())
                        .and_then(get_register);

                    let get_memory = warp::get()
                        .and(warp::path("mem"))
                        .and(warp::path("range"))
                        .and(warp::query::<GetMemoryRequest>())
                        .and(memory_filter.clone())
                        .and_then(get_memory);

                    let execute_step = warp::post()
                        .and(warp::path("vm"))
                        .and(warp::path("cmd"))
                        .and(warp::path("step"))
                        .and(warp::body::json::<StepRequest>())
                        .and(cpu_filter.clone())
                        .and(memory_filter.clone())
                        .and_then(execute_step);

                    let trigger_interrupt = warp::post()
                        .and(warp::path("cpu"))
                        .and(warp::path("interrupts"))
                        .and(warp::path::param::<String>())
                        .and(pic_filter.clone())
                        .and_then(trigger_interrupt);

                    let routes = get_register
                        .or(get_memory)
                        .or(execute_step)
                        .or(trigger_interrupt);

                    println!("Listening on 0.0.0.0:8080");
                    warp::serve(routes)
                        .run(([0, 0, 0, 0], 8080))
                        .await;
                });
        }).unwrap();

        async fn get_register(register: String, cpu: Arc<RwLock<CPU>>, memory: Arc<Memory>, pic: Arc<PIC>) -> Result<impl warp::Reply, warp::Rejection> {
            let r = match register.as_str() {
                "cs" => REG_CS,
                "pc" => REG_PC,
                "sp" => REG_SP,
                str => match usize::from_str(str) {
                    Ok(r) => match r {
                        0..=MAX_REGISTER => r,
                        _ => return Ok(warp::reply::with_status(
                            warp::reply::json(&ErrorResponse {
                                code: ERROR_INVALID_REGISTER,
                                message: format!("{} is not a valid register", register),
                            }), warp::http::StatusCode::BAD_REQUEST)),
                    },
                    Err(_) => return Ok(warp::reply::with_status(
                        warp::reply::json(&ErrorResponse {
                            code: ERROR_INVALID_REGISTER,
                            message: format!("{} is not a valid register", register),
                        }), warp::http::StatusCode::BAD_REQUEST)),
                }
            };

            // todo we could save reg to some api zone in memory, trigger interrupt, wait for
            //   answer from cpu and return the value
            memory.set(REST_API_START as u32, r as u8);
            pic.trigger(INT_API);

            //  todo prevent other calls to override this one

            Ok(warp::reply::with_status(
                warp::reply::json(&cpu.read().unwrap().read_register(r)),
                warp::http::StatusCode::OK,
            ))
        }

        async fn get_memory(query: GetMemoryRequest, memory: Arc<Memory>) -> Result<impl warp::Reply, warp::Rejection> {
            match memory.get_bytes(query.from, query.to - query.from) {
                None => Ok(warp::reply::with_status(
                    warp::reply::json(&ErrorResponse {
                        code: ERROR_INVALID_ADDRESS,
                        message: format!("{}..{} is not a valid address range", query.from, query.to),
                    }), warp::http::StatusCode::BAD_REQUEST)),
                Some(v) => Ok(warp::reply::with_status(
                    warp::reply::json(&GetMemoryResponse {
                        bytes: Vec::from(v),
                    }),
                    warp::http::StatusCode::OK,
                ))
            }
        }

        async fn execute_step(query: StepRequest, cpu: Arc<RwLock<CPU>>, memory: Arc<Memory>) -> Result<impl warp::Reply, warp::Rejection> {
            let mut cpu = cpu.write().unwrap();
            let mut running = true;

            for _ in 0..query.count.unwrap_or(1) {
                running = cpu.step(&memory);
                if !running {
                    break;
                }
            }

            let pc = cpu.read_register(REG_PC) as u32;
            Ok(warp::reply::with_status(
                warp::reply::json(&StepResponse {
                    running,
                    registers: Registers {
                        pc
                    },
                    meta: Meta {
                        steps_count: cpu.get_steps_count(),
                    },
                }), warp::http::StatusCode::OK,
            ))
        }

        async fn trigger_interrupt(int: String, pic: Arc<PIC>) -> Result<impl warp::Reply, warp::Rejection> {
            const MAX_INTERRUPT: u8 = (INTERRUPTS_COUNT - 1) as u8;
            match u8::from_str(int.as_str()) {
                Ok(int) => match int {
                    0..=MAX_INTERRUPT => {
                        pic.trigger(int);
                        return Ok(warp::reply::with_status(warp::reply::json(&InterruptResponse {
                            ok: true
                        }), warp::http::StatusCode::OK));
                    }
                    _ => return Ok(warp::reply::with_status(
                        warp::reply::json(&ErrorResponse {
                            code: ERROR_INVALID_INTERRUPT,
                            message: format!("{} is not a valid interrupt", int),
                        }), warp::http::StatusCode::BAD_REQUEST)),
                },
                Err(_) => return Ok(warp::reply::with_status(
                    warp::reply::json(&ErrorResponse {
                        code: ERROR_INVALID_INTERRUPT,
                        message: format!("{} is not a valid interrupt", int),
                    }), warp::http::StatusCode::BAD_REQUEST))
            }
        }
    }
}
