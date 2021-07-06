use std::sync::{Arc, RwLock};
use std::thread;

use serde::{Deserialize, Serialize};
use warp;
use warp::Filter;

use crate::cpu::CPU;

pub struct RestApi {}

#[derive(Deserialize, Serialize)]
struct ErrorResponse {
    code: u8,
    message: String,
}

const ERROR_INVALID_ADDRESS: u8 = 0;

#[derive(Deserialize, Serialize)]
struct GetMemoryRequest {
    from: u32,
    to: u32,
}

#[derive(Deserialize, Serialize)]
struct GetMemoryResponse {
    bytes: Vec<u8>,
}

impl RestApi {
    pub fn new(cpu: Arc<RwLock<CPU>>) {
        thread::spawn(|| {
            tokio::runtime::Builder::new_multi_thread()
                .enable_all()
                .build()
                .unwrap()
                .block_on(async move {
                    let cpu_filter = warp::any().map(move || cpu.clone());

                    let get_register = warp::get()
                        .and(warp::path("cpu"))
                        .and(warp::path("registers"))
                        .and(warp::path::param::<u8>())
                        .and(cpu_filter.clone())
                        .and_then(get_register);

                    let get_memory = warp::get()
                        .and(warp::path("mem"))
                        .and(warp::path("range"))
                        .and(warp::query::<GetMemoryRequest>())
                        .and(cpu_filter.clone())
                        .and_then(get_memory);

                    let execute_step = warp::post()
                        .and(warp::path("vm"))
                        .and(warp::path("cmd"))
                        .and(warp::path("step"))
                        .and(cpu_filter.clone())
                        .and_then(execute_step);

                    let routes = get_register
                        .or(get_memory)
                        .or(execute_step);

                    warp::serve(routes)
                        .run(([127, 0, 0, 1], 8080))
                        .await;
                });
        });

        async fn get_register(r: u8, cpu: Arc<RwLock<CPU>>) -> Result<impl warp::Reply, warp::Rejection> {
            Ok(warp::reply::with_status(
                warp::reply::json(&cpu.read().unwrap().read_register(r)),
                warp::http::StatusCode::OK,
            ))
        }

        async fn get_memory(query: GetMemoryRequest, cpu: Arc<RwLock<CPU>>) -> Result<impl warp::Reply, warp::Rejection> {
            match cpu.read().unwrap().read_memory(query.from, query.to - query.from) {
                None => Ok(warp::reply::with_status(
                    warp::reply::json(&ErrorResponse {
                        code: ERROR_INVALID_ADDRESS,
                        message: format!("{}..{} is not a valid address range", query.from, query.to),
                    }), warp::http::StatusCode::BAD_REQUEST)),
                Some(v) => Ok(warp::reply::with_status(
                    warp::reply::json(&GetMemoryResponse {
                        bytes: v,
                    }),
                    warp::http::StatusCode::OK,
                ))
            }
        }

        async fn execute_step(cpu: Arc<RwLock<CPU>>) -> Result<impl warp::Reply, warp::Rejection> {
            Ok(warp::reply::with_status(
                warp::reply::json(&cpu.write().unwrap().step()), warp::http::StatusCode::OK
            ))
        }
    }
}
